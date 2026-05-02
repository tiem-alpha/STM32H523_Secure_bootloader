### Protocol Message Format — UART Bootloader Frame

Đây là giao thức tùy chỉnh (custom) cho UART bootloader. Không dùng Ymodem/Xmodem để giữ đơn giản và có full control ACK/timeout.

#### Frame Structure

```
Byte:  [ 0 ]  [ 1 ]  [ 2 ]  [ 3..4 ]  [ 5..N-3 ]  [ N-2 .. N-1 ] [N]
       SOF    CMD    SEQ    LEN(16LE) PAYLOAD       CRC16(CCITT)   EOF

SOF     = 0xAC          (1 byte, fixed sync)
CMD     = command type  (1 byte, xem bảng bên dưới)
SEQ     = 0x00–0xFF     (sequence number, tăng mỗi frame, wrap 0xFF→0x00)
LEN     = uint16_t LE   (số byte PAYLOAD, 0–1024)
PAYLOAD = 0..1024 bytes (tuỳ CMD)
CRC16   = CRC-16/MCRF4XX  (tính từ CMD đến cuối PAYLOAD, 2 bytes LE)
EOF = 0xBB
```

#### Bảng CMD

| CMD | Hex | Direction | Payload | Mô tả |
|-----|-----|-----------|---------|-------|
| CMD_SYNC | 0x01 | PC → BL | none | Handshake, kiểm tra bootloader alive |
| CMD_INFO | 0x02 | PC → BL | none | Lấy thông tin: version, bank active, FW size |
| CMD_ERASE | 0x03 | PC → BL | `uint8_t bank` (0=A, 1=B) | Erase bank B trước khi write |
| CMD_DATA | 0x04 | PC → BL | `uint32_t offset` + `uint8_t data[N]` | Ghi block firmware (max 256B) |
| CMD_VERIFY | 0x05 | PC → BL | `uint32_t expected_crc32` + `uint32_t size` | Verify CRC32 toàn bộ image |
| CMD_SWITCH | 0x06 | PC → BL | `uint8_t target_bank` | Swap active bank và reboot |
| CMD_ABORT | 0x07 | PC → BL | none | Hủy transfer, giữ bank hiện tại |
| ACK | 0xAA | BL → PC | `uint8_t seq` + `uint8_t status` | status: 0=OK, 1=ERR, 2=CRC_FAIL |
| NACK | 0xBB | BL → PC | `uint8_t seq` + `uint8_t err_code` | err_code: xem bảng error |
| CMD_INFO_RSP | 0x82 | BL → PC | `uint8_t bl_ver[4]` + `uint8_t active_bank` + `uint32_t fw_size` + `uint32_t fw_crc` | Response cho CMD_INFO |

#### Error Codes (NACK payload)

| Code | Hex | Mô tả |
|------|-----|-------|
| ERR_CRC | 0x01 | CRC frame không khớp |
| ERR_SEQ | 0x02 | Sequence number không đúng |
| ERR_TIMEOUT | 0x03 | Timeout nhận frame (> 1000ms) |
| ERR_FLASH | 0x04 | Flash erase/write lỗi |
| ERR_VERIFY | 0x05 | Verify CRC32 image thất bại |
| ERR_ROLLBACK | 0x06 | Version FW nhỏ hơn version hiện tại |
| ERR_SIZE | 0x07 | FW size vượt quá bank capacity |
| ERR_BANK | 0x08 | Bank không hợp lệ |

#### Ví dụ CMD_SYNC exchange

```
PC  → BL:  AC  01  00  00 00  9A  1F  BB         (SOF, CMD_SYNC, SEQ=0, LEN=0, CRC , EOF)
BL  → PC:  AC  AA  00  02 00  00 00  CA 98  BB  (SOF, ACK, SEQ=0, LEN=2, seq=0, status=OK, CRC, EOF)
```

#### Ví dụ CMD_DATA block (offset=0x0000, 4 bytes data)

```
PC  → BL:  AC  04  01  06 00  00 00 00 00  DE AD 93 C7  BB  CRC_LO CRC_HI
           SOF CMD SEQ LEN=6  [offset=0]   [data 4B]
BL  → PC:  AC  AA  01  02 00  01 00  56  8A  BB CRC_LO CRC_HI
           (ACK, seq=1, status=OK)
```

#### ACK / Timeout Flow

```
PC                              Bootloader
 │                                   │
 │ Send frame (CMD_DATA)              │
 │──────────────────────────────────►│
 │                                   │ TIM2 start (600ms timeout)
 │                                   │ Receive bytes
 │                                   │ CRC check
 │                                   │ Flash write
 │       ACK (seq, OK)               │ TIM2 stop
 │◄──────────────────────────────────│
 │                                   │
 │ [Nếu không nhận ACK trong 600ms] │
 │ → PC retry (tối đa 3 lần)         │
 │ → Sau 3 lần fail → Abort          │
```

**Checklist Protocol:**
- [x] SOF byte 0x55 để detect frame start
- [x] SEQ number để detect duplicate / lost frame
- [x] CRC16-CCITT bảo vệ toàn bộ từ CMD đến PAYLOAD
- [x] Timeout 1000ms mỗi frame (TIM2)
- [x] ACK/NACK sau mỗi frame
- [x] Max payload 1024 bytes để tránh buffer overflow

---