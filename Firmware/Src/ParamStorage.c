#include "ParamStorage.h"

#include "stm32f10x.h"
#include "stm32f10x_flash.h"
#include <string.h>
#include <stdbool.h>

/* ---------------- Flash 布局与协议 ----------------
 * - 两页轮转模拟 EEPROM：
 *   PAGE0: 0x08007800, PAGE1: 0x08007C00（各 1KB）
 * - 页头 PageHeader：共 8 字节
 *   state(2) : 0xFFFF(擦空), 0xEEEE(RECEIVE), 0xAAAA(VALID)
 *   seq  (2) : 16 位递增序号（回绕比较）
 *   rsv  (4) : 预留（擦后为 0xFFFFFFFF）
 *
 * - 记录格式（变长）：
 *   +0: key  (2)
 *   +2: len  (2)
 *   +4: data (len 字节，向上偶数对齐)
 *   +4+dl: commit(2) == 0xA55A
 *
 * - 断电安全：
 *   只有当 commit 写入成功，该记录才视为有效。
 *   扫描时遇到撕裂/异常，强制把写指针推到页末（避免在非 0xFFFF 单元上重写）。
 */

/* ---------------- 常量与类型 ---------------- */
#define PAGE_SIZE     1024U
#define PAGE0_BASE    0x08007800UL
#define PAGE1_BASE    0x08007C00UL

#define EE_ERASED     0xFFFF
#define EE_RECEIVE    0xEEEE
#define EE_VALID      0xAAAA
#define EE_COMMIT_WORD 0xA55A

typedef struct __attribute__((packed))
{
    uint16_t state;   // 页状态：ERASED/RECEIVE/VALID
    uint16_t seq;     // 16 位序号（回绕比较）
    uint32_t rsv;     // 保留（擦后 0xFFFFFFFF）
} PageHeader;

/* ---------------- 内部状态 ---------------- */
static uint32_t g_active_base = 0; // 当前有效页的起始地址
static uint32_t g_write_ptr  = 0;  // 活动页内下一条记录写入地址（绝对地址）

/* ---------------- 工具函数（Flash/地址/读写） ---------------- */

/** 以半字读取 Flash（volatile），避免被优化合并 */
static inline uint16_t ee_rd16(uint32_t addr)
{
    return *(__IO uint16_t *)addr;
}

/** 页数据区起始与结束 */
static inline uint32_t page_data_start(uint32_t base) { return base + sizeof(PageHeader); }
static inline uint32_t page_end(uint32_t base)        { return base + PAGE_SIZE; }

/** 回绕安全的 16 位序号比较：返回 a 是否“比 b 更新” */
static inline bool seq16_is_newer(uint16_t a, uint16_t b)
{
    if (a == b) return false;
    return (uint16_t)(a - b) < 0x8000U;
}

/** Flash: 清标志 */
static inline void flash_clear_flags(void)
{
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
}

/** Flash: 半字编程，带返回值 */
static inline bool flash_write_half(uint32_t addr, uint16_t v)
{
    return FLASH_ProgramHalfWord(addr, v) == FLASH_COMPLETE;
}

/** Flash: 半字对齐缓冲区编程（len_even 必须为偶数），带返回值 */
static bool flash_write_buf_halfwords(uint32_t addr, const uint8_t *buf, uint32_t len_even)
{
    for (uint32_t off = 0; off < len_even; off += 2)
    {
        uint16_t hw = (uint16_t)buf[off] | ((uint16_t)buf[off + 1] << 8);
        if (!flash_write_half(addr + off, hw)) return false;
    }
    return true;
}

/** Flash: 擦除整页，带返回值（调用前需 Unlock） */
static inline bool flash_erase_page(uint32_t base)
{
    return FLASH_ErasePage(base) == FLASH_COMPLETE;
}

/** 页格式化为 VALID(seq)，并输出数据区写指针（调用前需 Unlock） */
static bool ee_format(uint32_t base, uint16_t seq, uint32_t *out_wp)
{
    if (!flash_erase_page(base)) return false;
    if (!flash_write_half(base + 0, EE_VALID)) return false;
    if (!flash_write_half(base + 2, seq)) return false;
    // rsv 保持擦后态 0xFFFF_FFFF
    if (out_wp) *out_wp = page_data_start(base);
    return true;
}

/** 计算当前活动页剩余空间（字节） */
static inline uint32_t space_left(void)
{
    return page_end(g_active_base) - g_write_ptr;
}

/** 另一页地址 */
static inline uint32_t other_page(uint32_t base)
{
    return (base == PAGE0_BASE) ? PAGE1_BASE : PAGE0_BASE;
}

/* ---------------- 扫描与健壮性 ---------------- */

/**
 * 健壮扫描：从数据区开始遍历有效记录，确定写指针。
 * 规则：
 * - 连续空洞 (key==0xFFFF && len==0xFFFF) 即结束。
 * - 头部撕裂/半写（key==0xFFFF 或 len==0xFFFF）、长度越界、
 *   commit 缺失或不匹配 → 把写指针推到页末（强制下次写迁移）。
 * - 正常记录则跳过到下一条。
 */
static void scan_write_ptr_and_guard(uint32_t base)
{
    uint32_t p   = page_data_start(base);
    uint32_t end = page_end(base);

    for (;;)
    {
        if (p + 4 > end) { break; } // 不够一条头部

        uint16_t k = ee_rd16(p + 0);
        uint16_t l = ee_rd16(p + 2);

        // 干净空洞：结束扫描
        if (k == 0xFFFF && l == 0xFFFF) { break; }

        // 半写头/撕裂头：强制迁移（把写指针放到页尾）
        if (k == 0xFFFF || l == 0xFFFF) { p = end; break; }

        uint32_t dl = (uint32_t)((l + 1U) & ~1U);
        // commit 越界：强制迁移
        if (p + 4 + dl + 2 > end) { p = end; break; }

        uint16_t c = ee_rd16(p + 4 + dl);
        // 撕裂尾：强制迁移
        if (c != EE_COMMIT_WORD) { p = end; break; }

        // 正常记录，跳过
        p += 4 + dl + 2;
    }

    g_write_ptr = p;
}

/**
 * 在给定页内查找“指定 key 的最新一条记录”。
 * 找到则返回 true，并输出该条目的数据地址与原始长度。
 * 若遇到异常/撕裂数据，立即停止扫描并按“未找到”处理。
 */
static bool find_latest_in_page(uint32_t base, uint16_t key,
                                uint32_t *out_data_addr, uint16_t *out_len)
{
    uint32_t p   = page_data_start(base);
    uint32_t end = page_end(base);
    uint32_t latest_data = 0;
    uint16_t latest_len  = 0;

    for (;;)
    {
        if (p + 4 > end) break;

        uint16_t k = ee_rd16(p + 0);
        uint16_t l = ee_rd16(p + 2);

        if (k == 0xFFFF && l == 0xFFFF) break;               // 结束
        if (k == 0xFFFF || l == 0xFFFF) break;                // 头撕裂→停止

        uint32_t dl = (uint32_t)((l + 1U) & ~1U);
        if (p + 4 + dl + 2 > end) break;                      // 越界→停止

        uint16_t c = ee_rd16(p + 4 + dl);
        if (c != EE_COMMIT_WORD) break;                       // 尾撕裂→停止

        if (k == key) { latest_data = p + 4; latest_len = l; }

        p += 4 + dl + 2;
    }

    if (latest_data)
    {
        if (out_data_addr) *out_data_addr = latest_data;
        if (out_len)       *out_len       = latest_len;
        return true;
    }
    return false;
}

/* ---------------- 页迁移（GC） ---------------- */

/**
 * 迁移并追加：把“每个 key 的最新一条”搬到另一页，然后附加本次要写的新记录。
 * 约束：
 * - 由调用者保证：已关中断、Flash 已解锁且已清标志。
 * - 内部全程检查返回值；失败返回 false，尽力保持电源故障可恢复性。
 */
static bool page_transfer_and_append(uint16_t new_key, const uint8_t *new_data, uint16_t new_len)
{
    uint32_t src = g_active_base;
    uint32_t dst = other_page(src);

    uint16_t old_state = ee_rd16(src + 0);
    uint16_t old_seq   = ee_rd16(src + 2);
    uint16_t new_seq   = (uint16_t)(old_seq + 1U);

    flash_clear_flags();
    if (!flash_erase_page(dst)) return false;

    // 先标记接收状态（E->A 只熄位，安全）
    if (!flash_write_half(dst + 0, EE_RECEIVE)) return false;
    if (!flash_write_half(dst + 2, new_seq))    return false;

    uint32_t wptr = page_data_start(dst);

    // 搬运策略：仅搬运你用得到的 key（避免 O(N^2)）
    // 需要时自行维护此列表
    static const uint16_t KEYS[] = {
        /* 示例 */ 0x0100,                // 大数组
        0x0010, 0x0011, 0x0012, 0x0013,    // 各种配置项
    };

    for (unsigned i = 0; i < sizeof(KEYS) / sizeof(KEYS[0]); ++i)
    {
        uint16_t key = KEYS[i];
        if (key == new_key) continue; // 新记录最后写

        uint32_t data_addr; uint16_t len;
        if (!find_latest_in_page(src, key, &data_addr, &len)) continue;

        uint32_t dl = (uint32_t)((len + 1U) & ~1U);

        // 写入一条：key,len,data,commit
        if (!flash_write_half(wptr + 0, key)) return false;
        if (!flash_write_half(wptr + 2, len)) return false;

        if (dl)
        {
            if (!flash_write_buf_halfwords(wptr + 4, (const uint8_t *)data_addr, dl)) return false;
        }

        if (!flash_write_half(wptr + 4 + dl, EE_COMMIT_WORD)) return false;
        wptr += 4 + dl + 2;
    }

    // 追加“本次要写的新记录”
    uint16_t len = new_len;
    uint32_t dl  = (uint32_t)((len + 1U) & ~1U);

    if (!flash_write_half(wptr + 0, new_key)) return false;
    if (!flash_write_half(wptr + 2, len))     return false;

    if (len)
    {
        if (len & 1U)
        {
            // 奇数长度：最后一个字节用 0xFF 补齐成半字
            if (len > 1U)
            {
                if (!flash_write_buf_halfwords(wptr + 4, new_data, len - 1U)) return false;
            }
            uint8_t last_hw[2] = { new_data[len - 1U], 0xFF };
            if (!flash_write_buf_halfwords(wptr + 4 + len - 1U, last_hw, 2U)) return false;
        }
        else
        {
            if (!flash_write_buf_halfwords(wptr + 4, new_data, dl)) return false;
        }
    }
    // 长度 0 也允许（删除语义可自定义）

    if (!flash_write_half(wptr + 4 + dl, EE_COMMIT_WORD)) return false;
    wptr += 4 + dl + 2;

    // 目标页置 VALID
    if (!flash_write_half(dst + 0, EE_VALID)) return false;

    // 擦除源页（失败也不影响新页的数据有效性）
    (void)flash_erase_page(src);

    // 切换活动页
    g_active_base = dst;
    g_write_ptr   = wptr;
    (void)old_state; // 仅为说明，未用
    return true;
}

/* ---------------- 对外接口 ---------------- */

/**
 * 初始化：识别活动页 -> 恢复异常状态（含 RECEIVE/RECEIVE）-> 扫描写指针。
 * 若两页都未初始化，则格式化 PAGE0 为 VALID(seq=1)。
 */
void EE_Init(void)
{
    uint16_t h0_state = ee_rd16(PAGE0_BASE + 0);
    uint16_t h0_seq   = ee_rd16(PAGE0_BASE + 2);
    uint16_t h1_state = ee_rd16(PAGE1_BASE + 0);
    uint16_t h1_seq   = ee_rd16(PAGE1_BASE + 2);

    bool have_valid0 = (h0_state == EE_VALID);
    bool have_valid1 = (h1_state == EE_VALID);

    if (have_valid0 && !have_valid1)
    {
        g_active_base = PAGE0_BASE;
    }
    else if (have_valid1 && !have_valid0)
    {
        g_active_base = PAGE1_BASE;
    }
    else if (have_valid0 && have_valid1)
    {
        // 两页都 VALID：选更新的 seq
        g_active_base = seq16_is_newer(h0_seq, h1_seq) ? PAGE0_BASE : PAGE1_BASE;
    }
    else if (h0_state == EE_RECEIVE && h1_state == EE_RECEIVE)
    {
        // 两页都 RECEIVE：选 seq 新的，并就地置 VALID（E->A 熄位）
        g_active_base = seq16_is_newer(h0_seq, h1_seq) ? PAGE0_BASE : PAGE1_BASE;
        FLASH_Unlock(); flash_clear_flags();
        (void)flash_write_half(g_active_base + 0, EE_VALID);
        FLASH_Lock();
    }
    else if (h0_state == EE_RECEIVE && have_valid1)
    {
        g_active_base = PAGE1_BASE;
    }
    else if (h1_state == EE_RECEIVE && have_valid0)
    {
        g_active_base = PAGE0_BASE;
    }
    else
    {
        // 两页都非 VALID/RECEIVE → 视作未初始化：格式化 PAGE0
        FLASH_Unlock(); flash_clear_flags();
        (void)ee_format(PAGE0_BASE, 1, &g_write_ptr);
        FLASH_Lock();
        g_active_base = PAGE0_BASE;
        return;
    }

    // 扫描活动页写指针（健壮处理撕裂/越界）
    scan_write_ptr_and_guard(g_active_base);
}

/**
 * 读取指定 key 的“最新一条”。
 * 返回值：实际拷贝到 buf 的字节数；若 key 不存在，返回 0。
 * 注意：遇到异常/撕裂记录时停止扫描（较保守）。
 */
uint16_t EE_Read(uint16_t key, void *buf, uint16_t maxlen)
{
    uint32_t p   = page_data_start(g_active_base);
    uint32_t end = page_end(g_active_base);
    uint32_t latest = 0;
    uint16_t latest_len = 0;

    for (;;)
    {
        if (p + 4 > end) break;

        uint16_t k = ee_rd16(p + 0);
        uint16_t l = ee_rd16(p + 2);

        if (k == 0xFFFF && l == 0xFFFF) break;
        if (k == 0xFFFF || l == 0xFFFF) break;

        uint32_t dl = (uint32_t)((l + 1U) & ~1U);
        if (p + 4 + dl + 2 > end) break;

        uint16_t c = ee_rd16(p + 4 + dl);
        if (c != EE_COMMIT_WORD) break;

        if (k == key) { latest = p + 4; latest_len = l; }
        p += 4 + dl + 2;
    }

    if (!latest) return 0;

    if (buf && maxlen)
    {
        uint16_t copy = (latest_len <= maxlen) ? latest_len : maxlen;
        memcpy(buf, (const void *)latest, copy);
        return copy;
    }
    return (latest_len <= maxlen) ? latest_len : maxlen;
}

/**
 * 写入一条记录（追加）。空间不足时触发页迁移并把本条一并写入。
 * 返回值：成功 true / 失败 false。
 * 约束：本条记录不能超过单页数据区容量。
 */
bool EE_Write(uint16_t key, const void *data, uint16_t len)
{
    uint32_t need = 4U + ((len + 1U) & ~1U) + 2U; // header + data_even + commit
    if (need > PAGE_SIZE - sizeof(PageHeader))
        return false; // 单条不能超过 1 页

    __disable_irq();

    if (space_left() < need)
    {
        FLASH_Unlock(); flash_clear_flags();
        bool ok = page_transfer_and_append(key, (const uint8_t *)data, len);
        FLASH_Lock();
        __enable_irq();
        return ok;
    }

    FLASH_Unlock(); flash_clear_flags();

    // 直接追加：key,len
    bool ok = true;
    ok &= flash_write_half(g_write_ptr + 0, key);
    ok &= flash_write_half(g_write_ptr + 2, len);

    uint32_t dl = (uint32_t)((len + 1U) & ~1U);

    if (ok && len)
    {
        if (len & 1U)
        {
            // 奇数长度：先写前面偶数部分，再补最后半字
            if (len > 1U)
            {
                ok &= flash_write_buf_halfwords(g_write_ptr + 4, (const uint8_t *)data, len - 1U);
            }
            uint8_t last_hw[2] = { *(((const uint8_t *)data) + len - 1U), 0xFF };
            ok &= flash_write_buf_halfwords(g_write_ptr + 4 + len - 1U, last_hw, 2U);
        }
        else
        {
            ok &= flash_write_buf_halfwords(g_write_ptr + 4, (const uint8_t *)data, dl);
        }
    }

    if (ok) ok &= flash_write_half(g_write_ptr + 4 + dl, EE_COMMIT_WORD);

    if (ok)
    {
        g_write_ptr += 4 + dl + 2;
    }

    FLASH_Lock();
    __enable_irq();
    return ok;
}
