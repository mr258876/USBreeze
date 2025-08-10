#include "HostCommunication.h"
#include "rl_usb.h"
#include <string.h>

#define PROTO_VER 0x01
#define ADDR_THIS 0x01 // Constant since using USB
#define FLAG_ACK 0x01
#define FLAG_ERR 0x02
#define FLAG_BROADCAST 0x80

enum
{
  CMD_PING = 0x01,
  CMD_GET_INFO = 0x02,
  CMD_SET_ADDR = 0x03,

  CMD_SET_PWM = 0x10,
  CMD_SET_TARGET_RPM = 0x11,
  CMD_SET_MULTI_PWM = 0x12,

  CMD_GET_RPM = 0x20,
  CMD_SUBSCRIBE_RPM = 0x21,
  CMD_RPM_PUSH = 0x22,
};

static uint8_t CDC_RxBuffer[16];
static uint8_t CDC_TxBuffer[256];

osMessageQDef(MsgInBox, 4, uint32_t); // Define message queue
osMessageQId MsgInBox;

static uint8_t rx_frame[PROTO_MAX];
static uint16_t rx_len = 0;

// COBS stuff
static uint8_t code = 0;         // Code of current chunk
static uint8_t rem = 0;          // Remaining bytes of this chunk (distance to next 0x00)
static uint8_t pending_zero = 0; // Whether we need a 0x00 before next byte
static uint8_t in_packet = 0;    // First 0x00 observed, hence we are in a packet
static uint8_t dropping = 0;     // Overflow: drop until next packet segment

static void cobs_finish_if_valid(void);
static void cobs_reset_frame(void);
static void cobs_reset_all(void);
static void cobs_feed_byte(uint8_t b);

static uint16_t crc16_ccitt(const uint8_t *d, uint32_t n);
static uint16_t crc16_upd(uint16_t c, uint8_t b);

static void handle_cmd(const uint8_t *p, uint16_t n);
static void send_ack(uint8_t cmd, uint8_t seq, uint8_t status);

static volatile uint8_t usb_tx_busy = 0;
static void proto_send(const uint8_t *body, uint16_t body_len);
static void usb_send_blockN(const uint8_t* p, uint32_t n);

static void handle_raw_income_data(uint32_t len);

static void handle_cmd(const uint8_t *p, uint16_t n)
{
  // p to VER; Format: VER FLG ADDR CMD SEQ LENlo LENhi PAY... CRC(checked already)
  if (n < 7)
    return;
  uint8_t ver = p[0];                                    // Protol Version
  uint8_t flg = p[1];                                    // Flags
  uint8_t addr = p[2];                                   // Device Addr
  uint8_t cmd = p[3];                                    // Command Code
  uint8_t seq = p[4];                                    // Packet ID
  uint16_t len = (uint16_t)p[5] | ((uint16_t)p[6] << 8); // Payload Length (Little Endian)
  const uint8_t *pay = p + 7;

  // Check Protol Version
  if (ver != PROTO_VER)
    return;
  // Ignore cmd not send to this device
  if (!(addr == ADDR_THIS || addr == 0xFF))
    return;

  switch (cmd)
  {
    // CMD_PING: Check device status
  case CMD_PING:
  {
    // Invalid Data, report & ask for resend
    if (len != 4)
    {
      if (flg & FLAG_ACK)
        send_ack(cmd, seq, 1);
      break;
    }

    // Req: u32 host_ms; Rsp: u32 dev_ms
    uint8_t out[7 + 4];
    out[0] = PROTO_VER;       // Protol Version
    out[1] = 0;               // Flags
    out[2] = ADDR_THIS;       // Device Addr
    out[3] = 0x80 | CMD_PING; // Command Code
    out[4] = seq;             //
    out[5] = 4;
    out[6] = 0;
    uint32_t dev_ms = osKernelSysTick(); // Return system tick
    memcpy(out + 7, &dev_ms, 4);
    proto_send(out, 7 + 4);
  }
  break;

  case CMD_GET_INFO:
  {
    uint8_t fan_cnt = board_get_fan_count();
    uint32_t uid[3];
    board_get_uid(uid);
    uint8_t payload[16 + 6];
    payload[0] = PROTO_VER;
    payload[1] = 1; // fw_major
    payload[2] = 0; // fw_minor
    payload[3] = fan_cnt;
    payload[4] = 8;               // max_fan_channels
    memcpy(payload + 5, uid, 12); // u32*3 = 12B
    payload[17] = 0x01;
    payload[18] = 0x00; // features_bits LSB=1:??RPM_PUSH
    payload[19] = 0xB8;
    payload[20] = 0x0B; // max_rpm=3000?(??),?? 0x0BB8
    uint8_t out_hdr[7] = {PROTO_VER, 0, ADDR_THIS, 0x80 | CMD_GET_INFO, seq, (uint8_t)sizeof(payload), 0};
    proto_send((uint8_t[]){out_hdr[0], out_hdr[1], out_hdr[2], out_hdr[3], out_hdr[4], out_hdr[5], out_hdr[6],
                           payload[0], payload[1], payload[2], payload[3], payload[4],
                           payload[5], payload[6], payload[7], payload[8], payload[9], payload[10], payload[11],
                           payload[12], payload[13], payload[14], payload[15], payload[16], payload[17], payload[18],
                           payload[19], payload[20]},
               7 + (uint16_t)sizeof(payload));
  }
  break;

  case CMD_SET_PWM:
  {
    // Invalid Data, report & ask for resend
    if (len != 3)
    {
      if (flg & FLAG_ACK)
        send_ack(cmd, seq, 1);
      break;
    }

    uint8_t ch = pay[0];
    uint16_t duty = (uint16_t)pay[1] | ((uint16_t)pay[2] << 8);
    uint8_t st = board_set_pwm(ch, duty);
    if (flg & FLAG_ACK)
      send_ack(cmd, seq, st);
  }
  break;

  case CMD_GET_RPM:
  {
    // Invalid Data, report & ask for resend
    if (len != 1)
    {
      if (flg & FLAG_ACK)
        send_ack(cmd, seq, 1);
      break;
    }

    uint8_t ch = pay[0];
    uint8_t n_out = 0;
    uint8_t buf[7 + 5 * 8]; // Buf
    buf[0] = PROTO_VER;
    buf[1] = 0;
    buf[2] = ADDR_THIS;
    buf[3] = 0x80 | CMD_GET_RPM;
    buf[4] = seq;
    uint8_t *pl = buf + 7; // payload
    if (ch == 0xFF)
    {
      for (uint8_t i = 0; i < board_get_fan_count(); i++)
      {
        uint32_t rpm = board_get_rpm(i);
        pl[0] = 0;
        pl[1] = rpm & 0xFF;
        pl[2] = rpm >> 8;
        pl[3] = rpm >> 16;
        pl[4] = rpm >> 24;
        pl += 5;
        n_out++;
      }
    }
    else
    {
      uint32_t rpm = board_get_rpm(ch);
      pl[0] = ch;
      pl[1] = rpm & 0xFF;
      pl[2] = rpm >> 8;
      pl[3] = rpm >> 16;
      pl[4] = rpm >> 24;
      pl += 5;
      n_out = 1;
    }
    buf[5] = (uint8_t)(n_out * 5);
    buf[6] = 0;
    proto_send(buf, 7 + n_out * 5);
  }
  break;
    /*
        case CMD_SUBSCRIBE_RPM: {
          if (len != 3) { if (flg & FLAG_ACK) send_ack(cmd, seq, 1); break; }
          rpm_push_cfg.interval_ms = (uint16_t)pay[0] | ((uint16_t)pay[1]<<8);
          rpm_push_cfg.ch_bitmap   = pay[2];
          rpm_push_cfg.last_tick_ms= osKernelGetTickCount();
          if (flg & FLAG_ACK) send_ack(cmd, seq, 0);
        } break;
    */
  default:
    if (flg & FLAG_ACK)
      send_ack(cmd, seq, 4); // Not Supported
    break;
  }
}

static void send_ack(uint8_t cmd, uint8_t seq, uint8_t status)
{
  uint8_t buf[8] = {PROTO_VER, 0x00, ADDR_THIS, (uint8_t)(cmd | 0x80), seq, 1, 0, status};
  // Format: VER, FLG, ADDR, CMD|0x80, SEQ, LEN(LE=1), PAYLOAD=status
  proto_send(buf, 7 + 1);
}

void proto_send(const uint8_t *body, uint16_t body_len)
{
  const uint32_t CAP = sizeof(CDC_TxBuffer);
  if (CAP < 256)
    return; // safety requirement

  // 1) Send frame delimiter 0x00 (its own small write is fine)
  uint8_t z = 0;
  usb_send_blockN(&z, 1);

  // 2) Local state lives in CDC_TxBuffer
  uint32_t len = 0;      // bytes currently in buffer
  uint32_t code_pos = 0; // index of current block's code placeholder in buffer
  uint8_t code = 1;

  // start first block
  CDC_TxBuffer[len++] = 0; // placeholder

// helper: send first n bytes and compact the unfinished block
#define FLUSH_PREFIX(N)                                   \
  do                                                      \
  {                                                       \
    usb_send_blockN(CDC_TxBuffer, (N));                   \
    memmove(CDC_TxBuffer, CDC_TxBuffer + (N), len - (N)); \
    len -= (N);                                           \
    code_pos -= (N);                                      \
  } while (0)

// helper: flush all completed blocks before current code_pos
#define FLUSH_COMPLETE_BLOCKS() \
  do                            \
  {                             \
    while (code_pos >= CAP)     \
    {                           \
      FLUSH_PREFIX(CAP);        \
    }                           \
    if (code_pos > 0)           \
    {                           \
      FLUSH_PREFIX(code_pos);   \
    }                           \
  } while (0)

  uint16_t crc = 0xFFFF;
  // process body
  for (uint16_t i = 0; i < body_len; i++)
  {
    uint8_t b = body[i];
    crc = crc16_upd(crc, b);
    if (b == 0)
    {
      CDC_TxBuffer[code_pos] = code; // finish block
      code_pos = len;                // start new block
      CDC_TxBuffer[len++] = 0;       // placeholder
      code = 1;
      FLUSH_COMPLETE_BLOCKS();
    }
    else
    {
      if (len + 1 > CAP)
      { // cannot flush mid-block -> CAP must be >=256
        // optional: handle error
        return;
      }
      CDC_TxBuffer[len++] = b;
      if (++code == 0xFF)
      {
        CDC_TxBuffer[code_pos] = code; // finish full block (no implicit zero)
        code_pos = len;
        if (len + 1 > CAP)
        {
          FLUSH_COMPLETE_BLOCKS();
        }
        CDC_TxBuffer[len++] = 0; // new placeholder
        code = 1;
        FLUSH_COMPLETE_BLOCKS();
      }
    }
  }
  // append CRC (LE) through the same path
  uint8_t tail[2] = {(uint8_t)(crc & 0xFF), (uint8_t)(crc >> 8)};
  for (int k = 0; k < 2; k++)
  {
    uint8_t b = tail[k];
    if (b == 0)
    {
      CDC_TxBuffer[code_pos] = code;
      code_pos = len;
      CDC_TxBuffer[len++] = 0;
      code = 1;
      FLUSH_COMPLETE_BLOCKS();
    }
    else
    {
      if (len + 1 > CAP)
      {
        return;
      }
      CDC_TxBuffer[len++] = b;
      if (++code == 0xFF)
      {
        CDC_TxBuffer[code_pos] = code;
        code_pos = len;
        if (len + 1 > CAP)
        {
          FLUSH_COMPLETE_BLOCKS();
        }
        CDC_TxBuffer[len++] = 0;
        code = 1;
        FLUSH_COMPLETE_BLOCKS();
      }
    }
  }

  // finalize last block and flush everything remaining (now all complete)
  CDC_TxBuffer[code_pos] = code;
  while (len)
  {
    uint32_t n = (len > CAP) ? CAP : len;
    FLUSH_PREFIX(n);
  }

#undef FLUSH_PREFIX
#undef FLUSH_COMPLETE_BLOCKS
}

static void usb_send_blockN(const uint8_t *p, uint32_t n)
{
  if (n == 0)
    return;
  while (usb_tx_busy)
  { // optional idle
  }
  usb_tx_busy = 1;
  USBD_CDC_ACM_WriteData(0, (uint8_t *)p, n);
  usb_tx_busy = 0;
}


static uint16_t crc16_upd(uint16_t c, uint8_t b)
{
  c ^= (uint16_t)b << 8;
  for (int i = 0; i < 8; i++)
    c = (c & 0x8000) ? (uint16_t)((c << 1) ^ 0x1021) : (uint16_t)(c << 1);
  return c;
}

void Host_Communication_Thread(void const *dummy_args)
{
  osEvent evt;

  cobs_reset_all();

  MsgInBox = osMessageCreate(osMessageQ(MsgInBox), NULL); // create msg queue

  while (1)
  {
    evt = osMessageGet(MsgInBox, osWaitForever);
    if (evt.status == osEventMessage)
    {
      uint32_t len = evt.value.v;
      handle_raw_income_data(len);
    }
  }
}

static void handle_raw_income_data(uint32_t len)
{
  while (len)
  {
    uint32_t n = (len > sizeof(CDC_RxBuffer)) ? sizeof(CDC_RxBuffer) : len;
    int32_t r = USBD_CDC_ACM_ReadData(0, CDC_RxBuffer, n);
    if (r <= 0)
      break;
    for (int i = 0; i < r; ++i)
    {
      cobs_feed_byte(CDC_RxBuffer[i]);
    }
    len -= (uint32_t)r;
  }
}

static uint16_t crc16_ccitt(const uint8_t *d, uint32_t n)
{
  uint16_t c = 0xFFFF;
  while (n--)
  {
    c ^= (uint16_t)(*d++) << 8;
    for (int i = 0; i < 8; i++)
      c = (c & 0x8000) ? (c << 1) ^ 0x1021 : (c << 1);
  }
  return c;
}

static void cobs_reset_frame(void)
{
  rx_len = 0;
  code = 0;
  rem = 0;
  pending_zero = 0;
  dropping = 0;
}

static void cobs_reset_all(void)
{
  in_packet = 0;
  cobs_reset_frame();
}

static void cobs_finish_if_valid(void)
{
  if (!dropping && rx_len >= 2)
  {
    uint16_t got = (uint16_t)rx_frame[rx_len - 2] | ((uint16_t)rx_frame[rx_len - 1] << 8);
    uint16_t cal = crc16_ccitt(rx_frame, (uint16_t)(rx_len - 2));
    if (got == cal)
      handle_cmd(rx_frame, (uint16_t)(rx_len - 2));
  }
}

static void cobs_feed_byte(uint8_t b)
{
  if (b == 0x00)
  {
    // Packet Segment: Try to finfish last packet if we're in packet
    if (in_packet)
      cobs_finish_if_valid();
    // Start a new packet
    in_packet = 1;
    cobs_reset_frame();
    return;
  }

  if (!in_packet)
  {
    // Dump noise if we are not in packet
    return;
  }

  if (dropping)
  {
    // Dropping packets too long, until we met 0x00
    return;
  }

  // If the last packet is ended and we need to pad a 0x00 before next byte
  if (pending_zero)
  {
    // Insert this zero only if next byte is not a segment, but a new byte
    // (If the next byte is 0x00, the last byte is really the "last", and we should not insert this 0)
    // Now we got a non-0-byte -> it should be the byte from next packet
    if (rx_len < PROTO_MAX)
    {
      if (rx_len > 0) // <- Note that the first byte indicates positino of next 0, and is not a part of data
      {
        rx_frame[rx_len++] = 0;
      }
    }
    else
    {
      dropping = 1;
      return;
    }
    pending_zero = 0;
  }

  if (rem == 0)
  {
    // Starting a new chunk, expected to read a new code for
    code = b;
    if (code == 0)
    { // Illegal, drop until the next 0x00
      dropping = 1;
      return;
    }
    rem = (uint8_t)(code - 1);
    // in theory we need a 0 after code < 0xFF
    // we do that until we see next byte (see pending_zero logic)
    if (code < 0xFF)
      pending_zero = 1;
    else
      pending_zero = 0; // no 0 for 0xFF
    return;
  }

  // Read code data (rem > 0)
  if (rx_len < PROTO_MAX)
    rx_frame[rx_len++] = b;
  else
  {
    dropping = 1;
    return;
  }
  if (--rem == 0)
  {
    // until we see the next byte then we decide whether to pad a 0x00
  }
}
