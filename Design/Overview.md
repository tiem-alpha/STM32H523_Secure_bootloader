# Secure Bootloader — STM32H523 + TrustZone

Quy trình thiết kế Secure Bootloader chạy bare-metal trên STM32H523, hỗ trợ TrustZone, Dual-Bank firmware, verify firmware, transfer qua UART 115200 với ACK/timeout, Watchdog, Timer.  
Tool: **STM32CubeIDE**

---

## Quy trình tổng quan

```
Phase 1          Phase 2          Phase 3          Phase 4
Requirements  →  System Arch   →  HW Design     →  SW Arch
                                                      ↓
Phase 7          Phase 6          Phase 5
Production    ←  Testing       ←  Detail Design
```

---

## Structure Tree

```
Secure Bootloader STM32H523
├── 1. Requirements
│   ├── Use Case Diagram
│   ├── System Context Diagram
│   └── Requirements Traceability Matrix (RTM)
│
├── 2. System Architecture
│   ├── Block Diagram (HW)
│   ├── System Architecture Diagram
│   ├── Communication Architecture Table
│   ├── Protocol Message Format (UART Bootloader Frame)
│   ├── Security Architecture (TrustZone + Trust Chain)
│   └── Memory Map (Dual-Bank Flash + TrustZone partition)
│
├── 3. Hardware Design
│   ├── Pinout Table (UART + Boot pins)
│   ├── Power + Reset + Clock Tree
│   └── Timing Diagram (UART 115200)
│
├── 4. Software Architecture
│   ├── Software Layer Diagram
│   ├── Component Diagram
│   ├── Dependency Graph
│   ├── State Machine Diagram (Bootloader FSM)
│   ├── Sequence Diagram (Boot flow + FW transfer)
│   └── Data Flow Diagram
│
├── 5. Detail Design & Coding
│   ├── Flowchart: main boot flow
│   ├── Flowchart: UART receive & ACK
│   ├── Flowchart: firmware verify (CRC32 + SHA256)
│   ├── Flowchart: dual-bank switch
│   ├── Interrupt Flow Diagram (UART RX, IWDG, TIM)
│   └── Timing Diagram (SW — watchdog kick, timeout)
│
├── 6. Integration & Testing
│   ├── Test Coverage Diagram
│   ├── Fault Tree / FMEA
│   └── Boot Time Profile
│
└── 7. Production & Maintenance
    ├── Deployment Diagram
    ├── Firmware Update Sequence (UART-based)
    └── Factory Provisioning Flow
```

---

## Phase 1 — Requirements

**Duration:** 3–5 ngày  
**Goal:** Xác định rõ chức năng bootloader, security policy, và constraints hardware.

### 1.1 Use Case Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                  Secure Bootloader STM32H523                │
│                                                             │
│   ┌────────────────────┐   ┌──────────────────────────┐    │
│   │  Verify Firmware   │   │  Transfer FW via UART    │    │
│   │  (CRC32 + SHA256)  │   │  (115200, ACK/NACK/TO)   │    │
│   └────────────────────┘   └──────────────────────────┘    │
│   ┌────────────────────┐   ┌──────────────────────────┐    │
│   │  Switch Dual Bank  │   │  TrustZone Partition      │    │
│   │  (Bank A ↔ Bank B) │   │  (Secure / Non-Secure)   │    │
│   └────────────────────┘   └──────────────────────────┘    │
│   ┌────────────────────┐   ┌──────────────────────────┐    │
│   │  Watchdog Kick     │   │  Jump to Application     │    │
│   │  (IWDG management) │   │  (after verify OK)       │    │
│   └────────────────────┘   └──────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
         ↑                                    ↑
  [Field Engineer / PC Tool]          [MCU Boot ROM]
  (ymodem/custom tool qua UART)       (hardware trust root)
```

**Checklist:**
- [x] Actor: Field Engineer (upload FW qua UART), MCU Boot ROM (hardware RoT), Factory PC
- [x] Use case chính: Transfer FW, Verify FW, Switch Bank, Jump to App, Kick IWDG
- [x] `include`: Transfer FW → Verify FW (bắt buộc verify trước khi switch)
- [x] `extend`: Switch Bank → Anti-rollback check (optional nếu version counter enable)

---

### 1.2 System Context Diagram

```
                 Field Engineer / PC Tool
                      │ UART 115200 8N1
                      │ (custom frame + ACK)
                      │
BOOT0 pin ──────► [STM32H523 Bootloader]  ──SWD──► STM32CubeIDE Debug
(HIGH = bootloader    │                                (programming + debug)
 LOW  = run app)      │ internal bus
                      ├──► Bank A Flash (Active FW)
                      ├──► Bank B Flash (Pending FW)
                      ├──► OTP (root key hash, version counter)
                      └──► TrustZone SAU/IDAU boundary
```

**Checklist:**
- [x] UART: PC Tool → Bootloader (upload), Bootloader → PC Tool (ACK/NACK/status)
- [x] BOOT0 pin điều khiển entry vào bootloader mode
- [x] SWD chỉ dùng khi debug/programming, bị lock ở production
- [x] Internal: Bootloader truy cập Bank A, Bank B, OTP, SAU registers

---

### 1.3 Requirements Traceability Matrix (RTM)

| Req ID | Requirement | Design Doc | Module | Test Case | Status |
|--------|-------------|------------|--------|-----------|--------|
| BL-01 | Boot time < 200ms (verify + jump) | Boot flow diagram | boot_main.c | TC-BL-01 | Planned |
| BL-02 | Transfer FW qua UART 115200, frame có ACK + timeout 1s | Protocol message format | uart_xfer.c | TC-BL-02 | Planned |
| BL-03 | Verify firmware bằng CRC32 + SHA-256 | Security architecture | fw_verify.c | TC-BL-03 | Planned |
| BL-04 | Hỗ trợ dual-bank: switch Bank A ↔ Bank B | Memory map | bank_mgr.c | TC-BL-04 | Planned |
| BL-05 | TrustZone: Bootloader chạy ở Secure world | SW arch, SAU config | tz_config.c | TC-BL-05 | Planned |
| BL-06 | IWDG luôn active, kick trong vòng lặp chính | Interrupt + timing diagram | watchdog.c | TC-BL-06 | Planned |
| BL-07 | Timer timeout cho từng UART frame (1000ms) | Timing diagram | timer.c | TC-BL-07 | Planned |
| BL-08 | Anti-rollback: không cho phép flash FW version thấp hơn | Security arch | version_mgr.c | TC-BL-08 | Planned |
| BL-09 | Bare-metal — không dùng RTOS | SW layer diagram | all modules | TC-BL-09 | Planned |
| BL-10 | Develop bằng STM32CubeIDE | - | - | - | Confirmed |

---

### Supporting Documents (Phase 1)

| Document | Nội dung |
|----------|----------|
| Glossary | TrustZone, SAU, IDAU, BOOT0, Bank swap, OTP, anti-rollback |
| Risk Register | Flash erase thất bại, power loss mid-flash, UART noise, OTP burn lỗi |
| MoSCoW | Must: verify+transfer+dual bank / Should: anti-rollback / Could: ECDSA / Won't: BLE OTA |

**Deliverables:** SRS bootloader, Threat model, MoSCoW

---

## Phase 2 — System Architecture

**Duration:** 3–5 ngày  
**Goal:** Xác định TrustZone partition, memory map dual-bank, security chain.

### 2.1 Block Diagram (HW)

```
              ┌─────────────────────────────────────────────┐
              │              STM32H523 MCU                  │
              │                                             │
              │  ┌──────────────────────────────────────┐  │
              │  │  Secure World (TrustZone)             │  │
              │  │   Bootloader code + crypto engine     │  │
              │  │   OTP key storage, SAU config         │  │
              │  └──────────────────────────────────────┘  │
              │                                             │
              │  ┌──────────────────────────────────────┐  │
              │  │  Non-Secure World                    │  │
              │  │   Application A (Bank 1)             │  │
              │  │   Application B (Bank 2)             │  │
              │  └──────────────────────────────────────┘  │
              │                                             │
              │  UART1 (PA9/PA10) ──────────────────────────┼──► PC/Tool
              │  IWDG (independent WDT)                     │
              │  TIM2 (timeout timer)                        │
              │  FLASH (dual-bank, 512KB × 2)               │
              │  OTP (key hash + version counter)            │
              │  SWD (PA13/PA14) ───────────────────────────┼──► Debug
              └─────────────────────────────────────────────┘
                           │
                       BOOT0 pin ──► HIGH = Bootloader mode
                                     LOW  = Normal boot
```

---

### 2.2 System Architecture Diagram

```
┌──────────────────────────────────────────────────────────────┐
│                  PC / Field Engineer Tool                    │  ← External
│           (STM32CubeProgrammer / custom Python script)       │
└──────────────────────────────┬───────────────────────────────┘
                               │ UART 115200 8N1
                               │ Custom frame + ACK/NACK
┌──────────────────────────────▼───────────────────────────────┐
│                   Secure Bootloader (Secure World)           │
│                                                              │
│   ┌──────────────────────────────────────────────────────┐   │
│   │                 Application Layer                    │   │
│   │   Boot Manager │ FW Transfer │ Bank Switch Manager   │   │
│   ├──────────────────────────────────────────────────────┤   │
│   │                  Service Layer                       │   │
│   │   UART Service │ Verify Service │ Flash Service      │   │
│   ├──────────────────────────────────────────────────────┤   │
│   │                  Driver Layer                        │   │
│   │   uart_drv.c │ flash_drv.c │ crc_drv.c │ timer_drv  │   │
│   ├──────────────────────────────────────────────────────┤   │
│   │                   HAL Layer                          │   │
│   │  HAL_UART │ HAL_FLASH │ HAL_CRC │ HAL_TIM │ HAL_IWDG │   │
│   └──────────────────────────────────────────────────────┘   │
└──────────────────────────────────────────────────────────────┘
               │ SAU/IDAU boundary (Non-Secure callable)
┌──────────────▼───────────────────────────────────────────────┐
│               Non-Secure World (Application)                 │
│         Application Bank A  │  Application Bank B           │
└──────────────────────────────────────────────────────────────┘
               │
┌──────────────▼───────────────────────────────────────────────┐
│                       Hardware                               │
│  FLASH Dual-Bank │ UART1 │ IWDG │ TIM2 │ CRC Unit │ 
└──────────────────────────────────────────────────────────────┘
```

---

### 2.3 Communication Architecture

| Interface | Master | Slave | Protocol | Config | Pin | Ghi chú |
|-----------|--------|-------|----------|--------|-----|---------|
| UART1 | PC Tool | Bootloader | UART | 115200 8N1, no flow ctrl | PA9 (TX) / PA10 (RX) | FW transfer + ACK |
| SWD | Debug Probe | MCU | SWD | default | PA13 / PA14 | Chỉ dùng khi debug, lock production |
| BOOT0 | External pull | MCU | GPIO | INPUT, pull-down default | BOOT0 pin | HIGH = enter bootloader |
| FLASH Bank A | MCU | Flash | Internal | Write/Erase | Internal | 0x0800_0000 – active FW |
| FLASH Bank B | MCU | Flash | Internal | Write/Erase | Internal | 0x0810_0000 – pending FW |

**Sơ đồ topology:**

```
         ┌──────────────────────────────────────────────┐
         │               STM32H523                      │
         │  UART1 ──────────────────────────► PC Tool   │
         │  (TX: PA9, RX: PA10, 115200 8N1)  ◄──────────│
         │                                              │
         │  SWD ────────────────────────────► Probe     │
         │  (PA13/PA14)                                 │
         │                                              │
         │  BOOT0 ◄──────── Pull resistor (ext)         │
         └──────────────────────────────────────────────┘
```

---

### 2.4 Protocol Message Format — UART Bootloader Frame

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

### 2.5 Security Architecture — TrustZone + Trust Chain

```
┌──────────────────────────────────────────────────────────────┐
│                    Boot ROM (immutable)                      │
│    Hardware Root of Trust — không thể sửa                    │
└──────────────────────────────┬───────────────────────────────┘
                               │ executes
                               ▼
┌──────────────────────────────────────────────────────────────┐
│              Secure Bootloader (Secure World)                │
│                                                              │
│  1. Init TrustZone SAU / IDAU                                │
│  2.  load public key hash                          │
│  3. Verify FW header signature (SHA-256 + ECDSA or CRC32)   │
│  4. Anti-rollback check ( version counter)                │
│  5. CRC32 verify toàn bộ image                              │
│                                                              │
│  PASS ──────────────────────────────────────────────────┐   │
│  FAIL → Set error flag + IWDG reset (không jump)        │   │
└─────────────────────────────────────────────────────────│───┘
                                                          │
                    NSC Gateway (Non-Secure Callable)     │
                    (chỉ expose API tối thiểu)            │
                                                          ▼
┌──────────────────────────────────────────────────────────────┐
│             Application (Non-Secure World)                   │
│        Bank A (Active)  hoặc  Bank B (Pending)              │
└──────────────────────────────────────────────────────────────┘

Key Storage:
  - Public key hash lưu trong Bootloader Flash (read-only)
  - Private key KHÔNG có trên device (chỉ ở signing server)
  - Version counter lưu trong flash bootloader (chỉ tăng, không giảm)

TrustZone SAU Config:
  Region 0: Secure      — Bootloader code + data (0x0C00_0000 – 0x0C00_FFFF)
  Region 1: Non-Secure  — Application Bank A (0x0800_0000 – 0x080F_FFFF)
  Region 2: Non-Secure  — Application Bank B (0x0810_0000 – 0x081F_FFFF)
  Region 3: NSC         — Non-Secure Callable gate (veneer table)
```

**Checklist Security:**
- [x] Root of Trust: Boot ROM (hardware, immutable)
- [x] Firmware verify: CRC32 (nhanh) + SHA-256 (cryptographic)
- [x] Key storage: Bootloader flash (public key hash), private key chỉ ở server
- [x] Anti-rollback: version counter trong Bootloader flash, không thể decrement
- [x] TrustZone SAU: Bootloader chạy Secure, App chạy Non-Secure
- [x] Không jump vào App nếu verify fail
- [x] NSC gateway: expose tối thiểu API ra Non-Secure

---

### 2.6 Memory Map — Dual-Bank + TrustZone
#### Memory Layout Diagram

##### Tổng Quan Bộ Nhớ STM32H523

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        STM32H523 MEMORY MAP                                 │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│                              FLASH (512KB)                                  │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  0x0800 0000 ┌─────────────────────────────────────────────────────────┐   │
│              │                                                         │   │
│              │  Secure Bootloader (32KB)                              │   │
│              │  ┌─────────────────────────────────────────────────┐  │   │
│              │  │  Secure Vector Table          (1KB)            │  │   │
│              │  ├─────────────────────────────────────────────────┤  │   │
│              │  │  Bootloader Code              (28KB)          │  │   │
│              │  ├─────────────────────────────────────────────────┤  │   │
│              │  │  Public Key Storage            (2KB)            │  │   │
│              │  ├─────────────────────────────────────────────────┤  │   │
│              │  │  Secure Data (Keys, Certs)    (1KB)            │  │   │
│              │  └─────────────────────────────────────────────────┘  │   │
│              ├─────────────────────────────────────────────────────────┤   │
│              │                                                         │   │
│              │  Non-Secure App A - Bank A (224KB)                    │   │
│              │  ┌─────────────────────────────────────────────────┐  │   │
│              │  │  Non-Secure Vector Table      (1KB)           │  │   │
│              │  ├─────────────────────────────────────────────────┤  │   │
│              │  │  Application Code              (210KB)         │  │   │
│              │  ├─────────────────────────────────────────────────┤  │   │
│              │  │  Application Data              (13KB)          │  │   │
│              │  └─────────────────────────────────────────────────┘  │   │
│              ├─────────────────────────────────────────────────────────┤   │
│              │                                                         │   │
│              │  Non-Secure App B - Bank B (224KB)                    │   │
│              │  ┌─────────────────────────────────────────────────┐  │   │
│              │  │  Non-Secure Vector Table      (1KB)           │  │   │
│              │  ├─────────────────────────────────────────────────┤  │   │
│              │  │  Application Code              (210KB)         │  │   │
│              │  ├─────────────────────────────────────────────────┤  │   │
│              │  │  Application Data              (13KB)          │  │   │
│              │  └─────────────────────────────────────────────────┘  │   │
│              ├─────────────────────────────────────────────────────────┤   │
│              │                                                         │   │
│              │  Metadata Area (8KB)                                   │   │
│              │  ┌─────────────────────────────────────────────────┐  │   │
│              │  │  Bank A Info (Signature, Version, CRC)  (4KB)   │  │   │
│              │  ├─────────────────────────────────────────────────┤  │   │
│              │  │  Bank B Info (Signature, Version, CRC)  (4KB)   │  │   │
│              │  └─────────────────────────────────────────────────┘  │   │
│              │                                                         │   │
│  0x0808 0000 └─────────────────────────────────────────────────────────┘   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│                               SRAM (128KB)                                  │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  0x2000 0000 ┌─────────────────────────────────────────────────────────┐   │
│              │                                                         │   │
│              │  Secure SRAM (32KB)                                     │   │
│              │  ┌─────────────────────────────────────────────────┐  │   │
│              │  │  Secure Stack                    (4KB)           │  │   │
│              │  ├─────────────────────────────────────────────────┤  │   │
│              │  │  Secure Heap                     (4KB)          │  │   │
│              │  ├─────────────────────────────────────────────────┤  │   │
│              │  │  Secure Data (Keys, State)      (24KB)         │  │   │
│              │  └─────────────────────────────────────────────────┘  │   │
│              ├─────────────────────────────────────────────────────────┤   │
│              │                                                         │   │
│              │  Non-Secure SRAM (96KB)                                │   │
│              │  ┌─────────────────────────────────────────────────┐  │   │
│              │  │  Non-Secure Stack                  (8KB)         │  │   │
│              │  ├─────────────────────────────────────────────────┤  │   │
│              │  │  Non-Secure Heap                   (8KB)        │  │   │
│              │  ├─────────────────────────────────────────────────┤  │   │
│              │  │  Non-Secure Data (Buffers)       (80KB)        │  │   │
│              │  └─────────────────────────────────────────────────┘  │   │
│              │                                                         │   │
│  0x2001 8000 └─────────────────────────────────────────────────────────┘   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

#### Chi Tiết Các Vùng Nhớ

##### 1. Secure Bootloader (0x0800 0000 - 0x0800 8000)

| Offset | Size | Description |
|--------|------|-------------|
| 0x0000 | 1KB | Secure Vector Table |
| 0x0400 | 28KB | Bootloader Code |
| 0x7000 | 2KB | Public Key Storage |
| 0x7800 | 1KB | Secure Data (Keys, Certs) |
| 0x7C00 | 1KB | Secure Config |

##### 2. Bank A - Application Active (0x0800 8000 - 0x0801 C000)

| Offset | Size | Description |
|--------|------|-------------|
| 0x8000 | 1KB | Non-Secure Vector Table |
| 0x8400 | 210KB | Application Code |
| 0x34800 | 13KB | Application Data |

##### 3. Bank B - Application Update (0x0801 C000 - 0x0803 0000)

| Offset | Size | Description |
|--------|------|-------------|
| 0x1C000 | 1KB | Non-Secure Vector Table |
| 0x1C400 | 210KB | Application Code |
| 0x3C800 | 13KB | Application Data |

##### 4. Metadata Area (0x0803 0000 - 0x0803 2000)

| Offset | Size | Description |
|--------|------|-------------|
| 0x30000 | 4KB | Bank A Metadata |
| 0x31000 | 4KB | Bank B Metadata |

##### Metadata Structure

```c
typedef struct {
    uint32_t magic;           // Magic number (0xB001A001)
    uint32_t version;         // Firmware version
    uint32_t size;            // Firmware size
    uint32_t crc32;           // CRC32 checksum
    uint8_t signature[256];   // RSA/ECDSA signature
    uint32_t signature_algo;  // 0: RSA, 1: ECDSA
    uint32_t timestamp;       // Build timestamp
    uint8_t hash[32];         // SHA-256 hash
    uint32_t flags;           // Active/Valid flags
    uint32_t reserved[8];    // Reserved
} metadata_t;
```

##### Memory Protection

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        MEMORY PROTECTION                                    │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│  MPU Configuration                                                          │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Region 0: Secure Flash (0x08000000 - 0x08008000)                         │
│  - Read: Secure only                                                        │
│  - Write: Secure only                                                       │
│  - Execute: Secure only                                                     │
│                                                                             │
│  Region 1: Bank A (0x08008000 - 0x0801C000)                                │
│  - Read: Secure + Non-Secure                                                │
│  - Write: Secure only                                                      │
│  - Execute: Non-Secure                                                      │
│                                                                             │
│  Region 2: Bank B (0x0801C000 - 0x08030000)                                │
│  - Read: Secure + Non-Secure                                                │
│  - Write: Secure only                                                      │
│  - Execute: Non-Secure                                                      │
│                                                                             │
│  Region 3: Metadata (0x08030000 - 0x08032000)                              │
│  - Read: Secure only                                                        │
│  - Write: Secure only                                                      │
│                                                                             │
│  Region 4: Secure SRAM (0x20000000 - 0x20008000)                          │
│  - Read/Write/Execute: Secure only                                          │
│                                                                             │
│  Region 5: Non-Secure SRAM (0x20008000 - 0x20018000)                       │
│  - Read/Write/Execute: Non-Secure                                           │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

##### Bank Switching Logic

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        BANK SWITCHING                                        │
└─────────────────────────────────────────────────────────────────────────────┘

    ┌──────────────┐         ┌──────────────┐
    │    Bank A    │◀───────▶│    Bank B    │
    │  (Active)    │  Switch  │  (Update)    │
    │  0x08008000   │         │  0x0801C000  │
    └──────────────┘         └──────────────┘
            │                        │
            │                        │
            ▼                        ▼
    ┌──────────────────────────────────────────────┐
    │           Metadata Table                     │
    │  ┌────────────────────────────────────────┐ │
    │  │ Bank A: Active=1, Valid=1, Ver=1.0    │ │
    │  │ Bank B: Active=0, Valid=0, Ver=1.1    │ │
    │  └────────────────────────────────────────┘ │
    └──────────────────────────────────────────────┘
            │
            ▼
    ┌──────────────────────────────────────────────┐
    │           Vector Table Offset                 │
    │  - SCB->VTOR = 0x08008000 (Bank A)          │
    │  - SCB->VTOR = 0x0801C000 (Bank B)          │
    └──────────────────────────────────────────────┘
```

---

## Phase 3 — Hardware Design

**Duration:** N/A — dùng STM32H523 Nucleo / custom board  
**Goal:** Xác định pinout, clock, power cho bootloader.

### Pinout Table

| Pin | Function | Config | Ghi chú |
|-----|----------|--------|---------|
| PA9 | UART1_TX | AF7, Output PP, 50MHz | TX đến PC Tool |
| PA10 | UART1_RX | AF7, Input | RX từ PC Tool |
| PA13 | SWDIO | AF0 | Debug — lock sau production |
| PA14 | SWCLK | AF0 | Debug — lock sau production |
| BOOT0 | Boot mode select | GPIO Input, Pull-down | HIGH > 0.7VDD = bootloader mode |
| PC13 | LED Status | Output PP | Blink khi transfer, steady khi verify OK |

---

### Power + Reset + Clock Tree

```
VDD (3.3V)
    │
    ├──► STM32H523 Core
    │        │
    │   POR (Power-On Reset)
    │        │
    │   ┌────▼──────┐
    │   │ IWDG      │ ← Independent Watchdog
    │   │ (LSI 32kHz│   Reset nếu không kick trong 500ms
    │   └────┬──────┘
    │        │ IWDG reset
    │        ▼
    │   MCU System Reset
    │
BOOT0 pin
    │ HIGH → Bootloader entry
    │ LOW  → Normal app boot

Clock Tree (Bootloader mode):
HSI (64MHz internal RC)
    │
    ├──► PLL1 ──► SYSCLK (250MHz max, dùng 64MHz cho BL — tiết kiệm đơn giản)
    │                ├──► AHB  (64MHz) ──► CPU, Flash, DMA
    │                ├──► APB1 (32MHz) ──► UART1, TIM2
    │                └──► APB2 (64MHz) ──► (không dùng ở BL)
    │
LSI (32kHz)
    └──► IWDG clock (độc lập với HSI)
```

---

### Timing Diagram (HW) — UART 115200

```
Bit period = 1 / 115200 = 8.68 µs

UART Frame (1 start + 8 data + 1 stop = 10 bits):
─┐     ┌─┬─┬─┬─┬─┬─┬─┬─┬─────
 │START │0│1│2│3│4│5│6│7│STOP
─┘     └─┴─┴─┴─┴─┴─┴─┴─┘
 ←8.68µs→←── 8 × 8.68µs ──→

Transfer time 256 bytes = 256 × 10 bits / 115200 = 22.2ms
Overhead (header + CRC) = 7 bytes = 0.6ms
Total 1 CMD_DATA block (256B payload): ~23ms
Total 448KB FW: ~40 seconds ← acceptable for field update
```

---

## Phase 4 — Software Architecture

**Duration:** 3–5 ngày  
**Goal:** Thiết kế SW bare-metal, không RTOS, các module rõ ràng.

### 4.1 Software Layer Diagram

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    SOFTWARE ARCHITECTURE LAYERS                             │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│                        APPLICATION LAYER                                    │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐                │
│  │   App Bank A   │ │   App Bank B   │ │  Bootloader       │                │
│  │  (Active FW)   │ │  (Update FW)   │ │  (Secure BL)      │                │
│  └─────────────────┘ └─────────────────┘ └─────────────────┘                │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
                                  │
┌─────────────────────────────────┼──────────────────────────────────────────────────────────┐
│                         Service Layer                                                │
├───────────────────────────────── ──────────────────────────────────────────────────────────┤
│                                                                                            │
│  ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐  ┌─────────────────┐          │
│  │  Secure Boot    │ │  Switch Manager │ │  Crypto Module  │  |  UART manager   |          │
│  │                 │ │                 │ │                 │  |  - Queue        │          │
│  │  - Signature    │ │  - Bank Switch  │ │  - SHA256       │  |  - Parser(Encode/Decode)│  |
│  │  - Public Key   │ │  - Metadata     │ │  - RSA/ECDSA    │  |  - UART         │          |
│  │  - CRC Verify   │ │  - Rollback     │ │  - RNG          │  |  -CRC           |          │
│  └─────────────────┘ └─────────────────┘ └─────────────────┘  └─────────────────┘          │
│                                                                                            │
└────────────────────────────────────────────────────────────────────────────────────────────┘
                                    |
┌─────────────────────────────────────────────────────────────────────────────┐
│                         Middle Ware Layer                                   │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌───────┐ ┌───────┐ ┌─────────┐  ┌──────┐ ┌──────┐  ┌──────┐  ┌──────┐     │
│  │  SHA  │ │ Flash │ │ TIMER   │  | UART │ | Queue│  |Parser|  |CRC   |     |
│  └───────┘ └───────┘ └─────────┘  └──────┘ └──────┘  └──────┘  └──────┘     │
|  ┌───────┐  ┌───────┐ ┌───────┐                                             |
|  |RNG    |  │ PKA   │ │ SHA   │                                             |
|  └───────┘  └───────┘ └───────┘                                             |
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
                                  │
┌─────────────────────────────────┼───────────────────────────────────────────┐
│                          DRIVER LAYER                                       │
├─────────────────────────────────┼───────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐              │
│  │   HAL Drivers   │ │  Hardware Crypto│ │  Low Level      │              │
│  │                 │ │    Drivers      │ │   Drivers       │              │
│  │  - HAL_UART    │ │                 │ │                 │              │
│  │  - HAL_FLASH   │ │  - HAL_RNG      │ │  - UART DMA    │              │
│  │  - HAL_GPIO    │ │  - HAL_HASH     │ │  - Flash Ctrl  │              │
│  │  - HAL_PWR     │ │  - HAL_PKA      │ │  - MPU Config  │              │
│  │  - HAL_TIM     │ │  - HAL_CRYP     │ │  - DMA Config  │              │
│  └─────────────────┘ └─────────────────┘ └─────────────────┘              │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
                                  │
┌─────────────────────────────────┼───────────────────────────────────────────┐
│                        HARDWARE LAYER                                       │
├─────────────────────────────────┼───────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐              │
│  │   Core         │ │   Crypto HW    │ │  Peripherals    │              │
│  │                 │ │                 │ │                 │              │
│  │  - Cortex-M33  │ │  - RNG         │ │  - UART        │              │
│  │  - FPU        │ │  - PKA         │ │  - DMA         │              │
│  │  - MPU        │ │  - HASH        │ │  - GPIO        │              │
│  │  - TrustZone  │ │  - CRYP        │ │  - Flash       │              │
│  └─────────────────┘ └─────────────────┘ └─────────────────┘              │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘

Quy tắc: App → Service → Driver → HAL → HW (một chiều, bare-metal)
```
#### UART Software architecture
```
+------------------+
|   Application    |
+--------+---------+
         |
+--------v---------+
| Command patcher  |
+---+----------+---+
         |
+--------v---------+
|   uart_manager   |
+---+----------+---+
    |          |
    v          v
+-------+   +--------+
| queue |   | Paser |
+---+---+   +---+----+
    |           |
    +-----+-----+
          v
     +---------+
     | uart drv|
     +---------+
```

---

### 4.2 Component Diagram


```Mermaid
flowchart TB

    Boot[Boot Core]

    UART[UART Service]
    Parser[Protocol Parser]
    Verify[Verify Service]
    Bank[Bank Switch Service]
    FlashSvc[Flash Service]

    CRC[CRC Engine]
    SHA[SHA256 Engine]

    Queue[Queue]

    UARTDrv[UART Driver]
    FlashDrv[Flash Driver]
    TimerDrv[Timer Driver]
    IWDG[IWDG Driver]
    TZ[TrustZone Init]

    HAL_UART[HAL UART]
    HAL_FLASH[HAL FLASH]

    Boot --> UART
    Boot --> Verify
    Boot --> Bank
    Boot --> IWDG
    Boot --> TZ

    UART --> Parser
    UART --> Queue
    UART --> UARTDrv
    UART --> TimerDrv

    Verify --> CRC
    Verify --> SHA

    Bank --> FlashSvc

    FlashSvc --> FlashDrv

    UARTDrv --> HAL_UART
    FlashDrv --> HAL_FLASH
```

---

### 4.3 Dependency Graph

```
boot_main.c
    ├── tz_init.c
    │       └── (CMSIS SAU registers — no HAL)
    ├── uart_svc.c
    │       ├── queue.c  
    │       ├── uart_drv.c
    │       │       └── HAL_UART
    │       ├── timer_drv.c
    │       │       └── HAL_TIM (TIM2)
    │       └── frame_parser.c
    │               └── crc16.c  ← shared
    ├── verify_svc.c
    │       ├── crc_drv.c
    │       │       └── HAL_CRC (hardware CRC unit)
    │       ├── sha256.c (SW)
    │       
    ├── flash_svc.c
    │       └── flash_drv.c
    │               └── HAL_FLASH_Ex (bank erase/program)
    ├── bank_mgr.c
    │       ├── flash_svc.c  (shared)
    │       ├── verify_svc.c (shared)
    │       └── version_mgr.c
    │               
    └── iwdg_drv.c
            └── HAL_IWDG
```

**Shared modules:**

| Module | Used by | Ghi chú |
|--------|---------|---------|
| `crc16.c` | frame_parser, uart_svc | Frame integrity — stateless |
| `flash_svc.c` | bank_mgr, verify_svc | Bank A/B erase+program |
| `timer_drv.c` | uart_svc (timeout) | TIM2 one-shot countdown |



### 4.4 Class Diagram
TBD

---

### 4.4 State Machine Diagram — Bootloader FSM

```
                  POWER_ON / RESET
                        │
                        ▼
                  ┌──────────┐
                  │  TZ_INIT  │──tz_fail──────────────────► FAULT
                  └────┬─────┘  (SAU config error)
                  tz_ok │
                        ▼
                  ┌──────────┐
                  │  HW_INIT  │──hw_fail──────────────────► FAULT
                  └────┬─────┘  (UART/TIM/IWDG init error)
                  hw_ok │
                        ▼
                  ┌────────────────┐
                  │  BOOT_CHECK    │
                  │ (check BOOT0,  │
                  │  check flags)  │
                  └──────┬─────────┘
                         │
              ┌──────────┴──────────┐
              │                     │
         BOOT0=HIGH            BOOT0=LOW
         force_update=1        force_update=0
              │                     │
              ▼                     ▼
       ┌────────────┐        ┌──────────────┐
       │  TRANSFER  │        │  FW_VERIFY   │
       │  (receive  │◄──────►│  (verify     │
       │   via UART)│ abort  │  active bank)│
       └──────┬─────┘        └──────┬───────┘
              │                     │
         done │              ┌──────┴──────┐
              │              │             │
              ▼           verify_ok    verify_fail
       ┌────────────┐        │             │
       │  FW_VERIFY │        ▼             ▼
       │  (verify   │   ┌──────────┐   ┌──────────┐
       │  new image)│   │  JUMP    │   │  RECOVER │
       └──────┬─────┘   │  to App  │   │  (try    │
              │         └──────────┘   │  Bank B  │
        ┌─────┴──────┐                 │  or FAULT│
        │            │                 └──────────┘
     ok │        fail│
        ▼            ▼
  ┌──────────┐  ┌──────────┐
  │  SWITCH  │  │  ABORT   │
  │  BANK    │  │  (keep   │
  │  + REBOOT│  │  old FW) │
  └──────────┘  └──────────┘

States: TZ_INIT, HW_INIT, BOOT_CHECK, TRANSFER, FW_VERIFY, SWITCH_BANK, JUMP, RECOVER, ABORT, FAULT
```

---

### 4.5 Sequence Diagram (Architecture-level) — Normal Boot Flow

```
boot_main    tz_init    hw_init    verify_svc    bank_mgr    App
    │             │          │           │             │        │
    │ tz_init()   │          │           │             │        │
    │────────────►│          │           │             │        │
    │   OK        │          │           │             │        │
    │◄────────────│          │           │             │        │
    │ hw_init()   │          │           │             │        │
    │─────────────────────► │           │             │        │
    │   OK                   │           │             │        │
    │◄─────────────────────── │          │             │        │
    │ check BOOT0             │          │             │        │
    │ BOOT0=LOW               │          │             │        │
    │ verify_active_bank()    │          │             │        │
    │──────────────────────────────────►│             │        │
    │                                    │ read header │        │
    │                                    │ check magic │        │
    │                                    │ crc32_check │        │
    │                                    │ sha256_check│        │
    │                                    │ version_check        │
    │          OK                        │             │        │
    │◄───────────────────────────────────│             │        │
    │ get_active_bank()                  │             │        │
    │─────────────────────────────────────────────────►│        │
    │ Bank A, entry = 0x0800_1000        │             │        │
    │◄─────────────────────────────────────────────────│        │
    │ disable_iwdg_or_hand_off()         │             │        │
    │ jump_to_app(0x0800_1000)           │             │        │
    │────────────────────────────────────────────────────────► │
    │                                                          │
    │                                                      App runs
```

---

### 4.6 Sequence Diagram — UART Firmware Transfer

```
PC Tool         uart_svc       flash_svc     verify_svc    bank_mgr
    │                │              │              │             │
    │ CMD_SYNC       │              │              │             │
    │───────────────►│              │              │             │
    │ ACK(OK)        │              │              │             │
    │◄───────────────│              │              │             │
    │                │              │              │             │
    │ CMD_ERASE(B)   │              │              │             │
    │───────────────►│ erase_bank(B)│              │             │
    │                │─────────────►│              │             │
    │                │    OK        │              │             │
    │ ACK(OK)        │◄─────────────│              │             │
    │◄───────────────│              │              │             │
    │                │              │              │             │
    │ CMD_DATA[0]    │              │              │             │
    │───────────────►│ write_block()│              │             │
    │                │─────────────►│              │             │
    │ ACK(OK)        │◄─────────────│              │             │
    │◄───────────────│              │              │             │
    │ CMD_DATA[1..N] │              │              │             │
    │  (repeat)      │   ...        │              │             │
    │                │              │              │             │
    │ CMD_VERIFY     │              │              │             │
    │───────────────►│              │ verify_crc32 │             │
    │                │──────────────────────────► │             │
    │                │              │   sha256     │             │
    │                │              │   version_chk│             │
    │ ACK(OK)        │              │              │             │
    │◄───────────────│              │              │             │
    │                │              │              │             │
    │ CMD_SWITCH(B)  │              │              │ set_active(B)
    │───────────────►│──────────────────────────────────────────►│
    │                │              │              │  write flag │
    │ ACK(OK)        │              │              │  reboot     │
    │◄───────────────│              │              │             │
    │ [device reboots]              │              │             │
```

---

### 4.7 Data Flow Diagram

```
[PC Tool — Python/STM32CubeProg]
      │ Raw .bin file + version metadata
      │ UART 115200 → custom frame
      ▼
[uart_svc — frame_parser]
      │ Validate SOF, CRC16, SEQ
      │ Extract CMD + PAYLOAD
      ▼
[Command Dispatcher — boot_main]
      │
      ├── CMD_DATA → [flash_svc]
      │                   │ uint32_t offset + uint8_t block[256]
      │                   ▼ write to Bank B (0x0810_0000 + offset)
      │
      ├── CMD_VERIFY → [verify_svc]
      │                   │ Read Bank B image
      │                   │ Compute CRC32 (HW unit)
      │                   │ Compute SHA-256 (SW)
      │                   │ Check version counter (OTP)
      │                   ▼ PASS / FAIL
      │
      └── CMD_SWITCH → [bank_mgr]
                          │ Write boot flags → FLASH Option Bytes
                          │ HAL_FLASH_OB_Launch()
                          ▼ System reset → Boot from Bank B
```

---

## Phase 5 — Detail Design & Coding

**Duration:** 1–2 tuần  
**Goal:** Logic chi tiết từng module, viết code STM32CubeIDE.

### 5.1 Flowchart — Main Boot Flow

```
          [RESET]
              │
     ┌────────▼────────┐
     │  tz_init()      │──FAIL──► FAULT_Handler() → infinite loop + LED blink
     └────────┬────────┘
              │OK
     ┌────────▼────────┐
     │  hw_init()      │──FAIL──► FAULT_Handler()
     │ UART, TIM, IWDG │
     │ CRC, FLASH      │
     └────────┬────────┘
              │OK
     ┌────────▼────────┐
     │ IWDG_Start()    │  ← kick interval 500ms, timeout 1s
     └────────┬────────┘
              │
     ┌────────▼────────┐
     │ Read BOOT0 pin  │
     └────────┬────────┘
              │
    ┌─────────┴──────────┐
BOOT0=HIGH           BOOT0=LOW
    │                    │
    ▼                    ▼
[TRANSFER_MODE]    [VERIFY_ACTIVE]
    │               verify active bank CRC + version
    │               ┌─────────┴─────────┐
    │            PASS               FAIL
    │               │                  │
    │               ▼              [RECOVER]
    │         [JUMP_TO_APP]         try other bank
    │                               if fail → FAULT
    │
[Receive frames via UART]
    │
    │ CMD_ERASE → CMD_DATA loop → CMD_VERIFY
    │
[Verify new image]
    ┌──────────┴──────────┐
  PASS                  FAIL
    │                    │
[CMD_SWITCH]         [ABORT]
[REBOOT]             keep old FW
                     send NACK
```

---

### 5.2 Flowchart — UART Receive & ACK

```
     [Frame Receive Loop]
              │
     ┌────────▼────────┐
     │ TIM2_Start(600) │  ← start timeout timer
     └────────┬────────┘
              │
     ┌────────▼────────┐
     │ Wait UART byte  │
     └────────┬────────┘
              │
    ┌──────────┴──────────┐
TIM2 expired          Byte received
    │                     │
    ▼                     │ Accumulate into rx_buf
[Send NACK ERR_TIMEOUT]   │
[Return ERR]              ▼
                   [Got full frame?]
                   (SOF found + LEN bytes received)
                          │NO → wait more bytes
                          │YES
                   ┌──────▼──────┐
                   │ CRC16 check │
                   └──────┬──────┘
                           │
              ┌────────────┴────────────┐
           CRC OK                    CRC FAIL
              │                         │
       ┌──────▼──────┐           [Send NACK ERR_CRC]
       │ SEQ check   │
       └──────┬──────┘
               │
    ┌──────────┴──────────┐
  SEQ OK               SEQ wrong (duplicate)
    │                      │
    │                [Send ACK, discard]
    ▼
[Dispatch CMD]
    │
    ▼
[Execute command]
    │
[Send ACK(seq, OK)] or [Send NACK(seq, err_code)]
    │
[TIM2_Stop()]
[expected_seq++]
```

---

### 5.3 Flowchart — Firmware Verify

```
    [verify_firmware(bank, expected_crc32, size)]
              │
     ┌────────▼────────┐
     │ Read FW header  │  (magic, version, size, crc32, sha256)
     └────────┬────────┘
              │
     ┌────────▼────────┐
     │ Magic valid?    │──NO──► Return ERR_INVALID_IMAGE
     │ (0xB007AB1E)    │
     └────────┬────────┘
              │YES
     ┌────────▼────────┐
     │ Size in range?  │──NO──► Return ERR_SIZE
     │ (0 < size ≤ 448KB)
     └────────┬────────┘
              │YES
     ┌────────▼────────────┐
     │ HW CRC32 compute    │  IWDG_Refresh() trong loop
     │ (loop 1KB blocks)   │  ← tránh IWDG reset trong lúc compute
     └────────┬────────────┘
              │
     ┌────────▼────────┐
     │ CRC32 match?    │──NO──► Return ERR_CRC_MISMATCH
     │ (computed vs    │
     │  header value)  │
     └────────┬────────┘
              │YES
     ┌────────▼────────────┐
     │ SHA-256 compute     │  IWDG_Refresh() trong loop
     │ (SW implementation) │
     └────────┬────────────┘
              │
     ┌────────▼────────┐
     │ SHA256 match?   │──NO──► Return ERR_SHA256_MISMATCH
     │ (vs header)     │
     └────────┬────────┘
              │YES
     ┌────────▼────────┐
     │ Read OTP version│
     │ counter         │
     └────────┬────────┘
              │
     ┌────────▼───────────────┐
     │ FW version >= OTP ver? │──NO──► Return ERR_ROLLBACK
     └────────┬───────────────┘
              │YES
     ┌────────▼────────┐
     │ Return VERIFY_OK│
     └─────────────────┘
```

---

### 5.4 Flowchart — Dual Bank Switch

```
    [bank_switch(target_bank)]
              │
     ┌────────▼────────┐
     │ Verify target   │
     │ bank first      │──FAIL──► Return ERR_VERIFY_BEFORE_SWITCH
     └────────┬────────┘
              │OK
     ┌────────▼────────────────────┐
     │ HAL_FLASH_OB_Unlock()       │
     └────────┬────────────────────┘
              │
     ┌────────▼────────────────────┐
     │ Read Option Bytes           │
     │ (FLASH_OPTSR_CUR)           │
     └────────┬────────────────────┘
              │
     ┌────────▼────────────────────┐
     │ Set SWAP_BANK bit           │
     │ (target_bank=B → set bit 0) │
     └────────┬────────────────────┘
              │
     ┌────────▼────────────────────┐
     │ HAL_FLASH_OB_Program()      │
     └────────┬────────────────────┘
              │
     ┌────────▼────────────────────┐
     │ HAL_FLASH_OB_Lock()         │
     └────────┬────────────────────┘
              │
     ┌────────▼────────────────────┐
     │ Burn OTP version counter    │
     │ (new version → OTP)         │  ← chỉ burn sau khi OB đã ghi OK
     └────────┬────────────────────┘
              │
     ┌────────▼────────────────────┐
     │ HAL_FLASH_OB_Launch()       │  ← triggers system reset, boots new bank
     └─────────────────────────────┘
     (function không return — MCU reset)
```

---

### 5.5 Interrupt Flow Diagram

**Interrupt Priority Map (bare-metal, không RTOS):**

| IRQ Source | NVIC Priority | Worst-case ISR Time | Tác vụ trong ISR | Ghi chú |
|-----------|---------------|--------------------|--------------------|---------|
| `IWDG` | — | — | Hardware reset (không có ISR) | Cứng, độc lập |
| `USART1_IRQn` | 1 (cao nhất) | 2 µs | Read RXDR → rx_buf, set flag | Dùng flag, không blocking |
| `TIM2_IRQn` | 2 | 0.5 µs | Set timeout_flag = 1 | Frame timeout 1000ms |
| `HardFault_IRQn` | — (fixed) | — | LED blink + infinite loop | Debug fault trap |

**UART RX Interrupt Flow:**

```
UART1 HW (RXDR not empty)
     │ assert USART1_IRQn
     ▼
NVIC (priority 1)
     │ context save (bare-metal: registers onto stack)
     ▼
USART1_IRQHandler()
     │
     ├── Read UART1->RDR → rx_byte
     ├── Clear RXNE flag (auto-cleared by read)
     ├── rx_buf[rx_idx++] = rx_byte
     ├── if (rx_idx >= expected_len) → rx_complete_flag = 1
     └── return from ISR
     │
     ▼
Resume main loop
     │
main loop polls rx_complete_flag
     │ flag=1
     ▼
process_frame()   ← xử lý trong main, không trong ISR
```

**TIM2 Timeout Interrupt Flow:**

```
TIM2 ARR (1000ms) expires
     │ assert TIM2_IRQn
     ▼
TIM2_IRQHandler()
     │
     ├── Clear UIF flag (TIM2->SR &= ~TIM_SR_UIF)
     ├── timeout_flag = 1
     ├── TIM2_Stop()      ← one-shot, stop sau khi fire
     └── return from ISR
     │
main loop: if (timeout_flag) → send_nack(ERR_TIMEOUT) → reset transfer state
```

---

### 5.6 Timing Diagram (SW) — Watchdog & Timeout

```
Time (ms) → 0    100   200   300   400   500   600   700   800   900  1000
──────────────────────────────────────────────────────────────────────────
IWDG kick  │█    │     │     │     │█    │     │     │     │█    │     │
           │     │     │     │     │     │     │     │     │     │     │
TIM2 frame │██████████████████████████████████████████████████████████████►TIMEOUT
timeout    │←───────────────── 1000ms ─────────────────────────────────►│
           │                                                             │
           │ [Normal: ACK received at t=300ms → TIM2 stop, kick IWDG]  │
           │█    │     │█    │     │     │     │     │     │     │     │
TIM2 stop  │     │     │█←stop     │     │     │     │     │     │     │

IWDG timeout = 1000ms (config: prescaler /32, reload = 1250 → ~1s)
IWDG kick interval in BL = every 500ms (in main loop + in CRC compute loop)
TIM2 frame timeout = 1000ms (one-shot per frame)

Worst case: large flash erase (Bank 512KB) = ~3s
→ Must kick IWDG inside flash_erase() loop per sector
```

---

## Phase 6 — Integration & Testing

**Duration:** 1 tuần  
**Goal:** Verify toàn bộ luồng bootloader end-to-end.

### 6.1 Test Coverage Diagram

```
Module              Unit Test    Integration    System E2E
──────────────────────────────────────────────────────────
tz_init.c           TC-TZ-01         ✅             ✅
uart_svc.c          TC-UART-01       ✅             ✅
frame_parser.c      TC-FRM-01        ✅             ✅
verify_svc.c        TC-VFY-01        ✅             ✅
flash_svc.c         TC-FLS-01        ✅             ✅
bank_mgr.c          TC-BNK-01        ✅             ✅
timer_drv.c         TC-TIM-01        ✅             ✅
iwdg_drv.c          TC-WDG-01        ⬜ (HW only)   ✅
otp_drv.c           TC-OTP-01        ⬜ (once/board) ⬜
version_mgr.c       TC-VER-01        ✅             ✅
```

**System E2E Test Scenarios:**

| TC ID | Scenario | Expected Result |
|-------|----------|-----------------|
| TC-E2E-01 | BOOT0=LOW, valid FW Bank A → normal boot | Jump to App A trong <200ms |
| TC-E2E-02 | BOOT0=HIGH → upload FW → verify → switch → reboot | App B runs |
| TC-E2E-03 | Upload FW corrupt CRC → CMD_VERIFY | NACK ERR_VERIFY, no switch |
| TC-E2E-04 | Upload FW version < current OTP version | NACK ERR_ROLLBACK |
| TC-E2E-05 | UART timeout (no data for 1000ms) | NACK ERR_TIMEOUT, transfer aborted |
| TC-E2E-06 | No IWDG kick simulation (disable kick) | MCU resets after 1s |
| TC-E2E-07 | Power loss mid flash write → reboot | Boot old bank, corrupt bank ignored |
| TC-E2E-08 | Both banks corrupt | FAULT state, no jump, LED error pattern |
| TC-E2E-09 | TrustZone: App tries to access Secure region | HardFault (SAU violation) |

---

### 6.2 Fault Tree / FMEA

**Fault Tree — "Device không boot sau update":**

```
                Device không boot sau update
                            │
          ┌─────────────────┼──────────────────┐
          │                 │                  │
     New FW corrupt    Bank switch fail    Power loss
          │                 │              mid-program
     ┌────┴────┐       ┌────┴────┐              │
     │CRC fail │       │OB write │         [Recover:
     │SHA fail │       │ fail    │          boot old bank]
     │Transfer │       │Reset OB │
     │ corrupt │       │ fail    │
     └─────────┘       └─────────┘
```

**FMEA Table:**

| Failure Mode | Cause | Effect | Severity | Probability | Detection | Action |
|---|---|---|---|---|---|---|
| Flash write fail mid-transfer | Power loss, ESD | Corrupt Bank B | High | Low | CRC verify trước switch | Không switch nếu verify fail |
| UART frame CRC error | Noise, bad cable | Wrong FW data written | High | Medium | CRC16 per frame | NACK + retry, max 3x |
| IWDG reset during flash erase | Forgot kick | Incomplete erase | High | Low | SW: kick IWDG in erase loop | Implement kick per sector |
| Anti-rollback OTP burn fail | OTP already burned | Version not updated | Medium | Low | Read-back verify after burn | Halt if OTP verify fails |
| TrustZone SAU misconfigured | Wrong region config | App access Secure | Critical | Low | HardFault in testing | Test SAU boundary in TC-TZ-01 |
| Dual bank flag not set | OB write fail | Always boot Bank A | Medium | Low | Verify OB after write | Read-back OB before launch |

---

### 6.3 Boot Time Profile

```
Time (ms) →  0     10    20    30    50    100   150   200
────────────────────────────────────────────────────────────
TZ_INIT      │████│
HW_INIT      │     │████│
BOOT_CHECK   │          │█│
CRC32_VERIFY │            │████████████│         (448KB @ 64MHz ~50ms)
SHA256_VERIFY│                         │█████████████│  (SW, ~80ms)
JUMP         │                                        │█│

Total: ~150ms ✅  (budget: 200ms)

Note: SHA256 SW implementation trên Cortex-M33 @ 64MHz cho 448KB ≈ 80ms
      Nếu cần nhanh hơn: dùng hardware crypto engine của H523 (HASH)
      → giảm còn ~10ms → total < 70ms
```

---

## Phase 7 — Production & Maintenance

### 7.1 Deployment Diagram

```
[Factory PC / CI Server]
      │ STM32CubeProgrammer (SWD)
      │ Flash bootloader + factory keys
      │
[STM32H523 Device]
      │ BOOT0 = LOW (normal boot)
      │ SWD locked (RDP Level 2 sau factory)
      │
[Field Update via UART]
      │ PC Tool / custom Python script
      │ Connect UART1 (PA9/PA10)
      │ Pull BOOT0 HIGH
      │ Run update sequence
      │ BOOT0 LOW → reboot
      │
[Production Device]
      TrustZone active
      IWDG running
      Active bank verified on every boot
```

---

### 7.2 Firmware Update Sequence (UART-based — field update)

```
PC Tool                         Bootloader (STM32H523)
    │                                    │
    │ 1. Pull BOOT0 HIGH, reset MCU      │
    │────────────────────────────────────►│ [TZ_INIT → HW_INIT → TRANSFER_MODE]
    │                                    │
    │ 2. CMD_SYNC                        │
    │───────────────────────────────────►│
    │    ACK(OK)                         │
    │◄───────────────────────────────────│
    │                                    │
    │ 3. CMD_INFO                        │
    │───────────────────────────────────►│
    │    CMD_INFO_RSP (bl_ver, active,   │
    │    fw_size, fw_crc)                │
    │◄───────────────────────────────────│
    │ [PC: check version, decide update] │
    │                                    │
    │ 4. CMD_ERASE(bank=B)               │
    │───────────────────────────────────►│ [Erase Bank B, kick IWDG per sector]
    │    ACK(OK)                         │
    │◄───────────────────────────────────│
    │                                    │
    │ 5. CMD_DATA × N  (256B blocks)     │
    │───────────────────────────────────►│ [Write to Bank B]
    │    ACK(OK) per block               │
    │◄───────────────────────────────────│
    │                                    │
    │ 6. CMD_VERIFY(expected_crc32, size)│
    │───────────────────────────────────►│ [CRC32 + SHA256 + version check]
    │    ACK(OK) or NACK(ERR_VERIFY)     │
    │◄───────────────────────────────────│
    │                                    │
    │ 7. CMD_SWITCH(bank=B)              │
    │───────────────────────────────────►│ [Write OB SWAP_BANK, burn OTP ver]
    │    ACK(OK)                         │ [HAL_FLASH_OB_Launch → reset]
    │◄───────────────────────────────────│
    │                                    │ [MCU reboots, boots Bank B]
    │ 8. Pull BOOT0 LOW                  │
    │    [MCU boots App B automatically] │
    │                                    │
    │ FAIL at step 6 → no switch, keep Bank A
    │ FAIL at step 7 → OB not launched, keep Bank A
```

---

### 7.3 Factory Provisioning Flow

```
Factory PC (STM32CubeProgrammer)     Device (STM32H523 DUT)
     │                                          │
     │ 1. Power on, BOOT0=HIGH (bootrom mode)   │
     │─────────────────────────────────────────►│
     │                                          │
     │ 2. Read Chip UID (96-bit)                │
     │◄─────────────────────────────────────────│
     │ [Register UID to provisioning server]    │
     │                                          │
     │ 3. Flash Bootloader (SWD)                │
     │    (bl_vX.X.bin to 0x0C00_0000)          │
     │─────────────────────────────────────────►│
     │ 4. Verify Bootloader CRC                 │
     │◄─────────────────────────────────────────│
     │                                          │
     │ 5. Flash App A (SWD)                     │
     │    (app_vX.X.bin to 0x0800_0000)         │
     │─────────────────────────────────────────►│
     │ 6. Verify App A CRC                      │
     │◄─────────────────────────────────────────│
     │                                          │
     │ 7. Burn OTP: Public Key Hash (32B)        │
     │─────────────────────────────────────────►│
     │ 8. Burn OTP: Version counter = 1         │
     │─────────────────────────────────────────►│
     │ 9. Verify OTP read-back                  │
     │◄─────────────────────────────────────────│
     │                                          │
     │ 10. Configure TrustZone SAU (via SWD)    │
     │─────────────────────────────────────────►│
     │                                          │
     │ 11. Set RDP Level 1 (Option Bytes)        │
     │     (disable JTAG mass read)             │
     │─────────────────────────────────────────►│
     │                                          │
     │ 12. BOOT0=LOW, reboot                    │
     │─────────────────────────────────────────►│
     │                                          │
     │ 13. Run production test (UART)           │
     │     - Verify boot, version, TZ boundary  │
     │─────────────────────────────────────────►│
     │ 14. PASS / FAIL                          │
     │◄─────────────────────────────────────────│
     │                                          │
     │ [PASS] Apply QC label, ship              │
     │ [FAIL] Log UID + error, quarantine       │
```

**Deliverables:** BL binary v1.0, Python PC tool, Factory test script, OTP burn procedure

---

## Tóm tắt thứ tự vẽ diagram

| # | Diagram | Phase | Mục đích |
|---|---------|-------|---------|
| 1 | Use Case Diagram | 1 | Chức năng BL từ góc nhìn Field Engineer |
| 2 | System Context Diagram | 1 | UART, BOOT0, SWD, internal flash |
| 3 | Block Diagram (HW) | 2 | TrustZone Secure/Non-Secure blocks |
| 4 | System Architecture | 2 | Layers: App/Svc/Drv/HAL + TZ boundary |
| 5 | Communication Architecture | 2 | UART 115200, SWD, OTP, Flash banks |
| 6 | Protocol Message Format | 2 | Custom UART frame + ACK/NACK |
| 7 | Memory Map | 2 | Dual-bank Flash + TrustZone partition |
| 8 | Security Architecture | 2 | Trust chain, SAU, OTP, anti-rollback |
| 9 | Pinout + Clock Tree | 3 | UART pins, HSI 64MHz, IWDG LSI |
| 10 | Timing Diagram (HW) | 3 | UART 115200 bit timing, transfer time |
| 11 | Software Layer Diagram | 4 | Bare-metal layers, TZ layer |
| 12 | Component Diagram | 4 | uart_svc, verify_svc, flash_svc, bank_mgr |
| 13 | Dependency Graph | 4 | Build-time deps, shared modules |
| 14 | State Machine | 4 | TZ_INIT→BOOT_CHECK→TRANSFER/VERIFY→JUMP |
| 15 | Sequence Diagram (arch) | 4 | Normal boot + UART transfer |
| 16 | Data Flow Diagram | 4 | .bin → frame → flash → verify → switch |
| 17 | Flowchart: main boot | 5 | Chi tiết BOOT0 check, verify, jump |
| 18 | Flowchart: UART ACK | 5 | Frame receive, timeout, ACK/NACK |
| 19 | Flowchart: FW verify | 5 | CRC32 + SHA256 + version check |
| 20 | Flowchart: bank switch | 5 | OB write, OTP burn, OB_Launch |
| 21 | Interrupt Flow Diagram | 5 | UART RX ISR, TIM2 timeout ISR |
| 22 | Timing Diagram (SW) | 5 | IWDG kick interval, TIM2 timeout |
| 23 | Fault Tree / FMEA | 6 | Flash fail, corrupt FW, IWDG reset |
| 24 | Test Coverage Diagram | 6 | TC map per module, E2E scenarios |
| 25 | Boot Time Profile | 6 | Verify < 200ms budget |
| 26 | Deployment Diagram | 7 | Factory SWD + Field UART topology |
| 27 | FW Update Sequence (UART) | 7 | Full update flow với rollback |
| 28 | Factory Provisioning Flow | 7 | OTP burn + RDP + production test |

---

## Glossary

| Term | Definition |
|------|------------|
| TrustZone | ARM security extension: chia MCU thành Secure và Non-Secure world |
| SAU | Security Attribution Unit — cấu hình vùng Secure/Non-Secure/NSC |
| IDAU | Implementation Defined Attribution Unit — hardware security attribution |
| NSC | Non-Secure Callable — vùng Flash đặc biệt, Non-Secure có thể call vào |
| OTP | One-Time Programmable — Flash region chỉ ghi được một lần |
| Anti-rollback | Cơ chế ngăn cài FW version thấp hơn, dùng OTP counter |
| Dual-bank | Flash có 2 bank độc lập (A và B), có thể swap boot bank |
| Bank swap | Thay đổi Option Bytes SWAP_BANK để MCU boot từ bank khác |
| Option Bytes (OB) | Flash config register: RDP, SWAP_BANK, WRP... |
| RDP | Read-out Protection Level 0/1/2 — bảo vệ đọc Flash |
| IWDG | Independent Watchdog — reset MCU nếu SW không kick đúng hạn |
| TIM2 | General-purpose timer dùng làm one-shot timeout cho UART frame |
| CRC32 | 32-bit cyclic redundancy check — verify toàn bộ FW image |
| SHA-256 | Cryptographic hash 256-bit — verify tính toàn vẹn FW |
| ECDSA | Elliptic Curve Digital Signature — ký FW bằng private key |
| HAL | Hardware Abstraction Layer (STM32 HAL library từ CubeMX) |
| SRS | Software Requirements Specification |
| DUT | Device Under Test |
| BL | Bootloader |
| FW | Firmware |
| BOOT0 | Pin trên STM32, kéo HIGH để vào bootloader mode |
