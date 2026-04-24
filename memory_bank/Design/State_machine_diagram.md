# State Machine Diagram

## Tổng Quan State Machine

### Main Bootloader State Machine

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    SECURE BOOTLOADER STATE MACHINE                          │
└─────────────────────────────────────────────────────────────────────────────┘

                              ┌─────────────┐
                              │   RESET     │
                              │  (Power On) │
                              └──────┬──────┘
                                     │
                                     ▼
                              ┌─────────────┐
                              │   INIT      │
                              │  Hardware   │
                              └──────┬──────┘
                                     │
                                     ▼
                              ┌─────────────┐
                              │  CHECK_BOOT │
                              │  Mode Pin   │
                              └──────┬──────┘
                                     │
              ┌──────────────────────┼──────────────────────┐
              │                      │                      │
              ▼                      ▼                      ▼
     ┌──────────────┐       ┌──────────────┐       ┌──────────────┐
     │  NORMAL_BOOT │       │   UPDATE     │       │   FACTORY   │
     │  (Default)   │       │  (UART Mode) │       │  (Reset)    │
     └──────┬───────┘       └──────┬───────┘       └──────┬───────┘
            │                      │                      │
            ▼                      ▼                      ▼
     ┌──────────────┐       ┌──────────────┐       ┌──────────────┐
     │  READ_META   │       │  WAIT_DATA   │       │  ERASE_ALL   │
     │  Read Bank   │       │  Receive FW  │       │  Factory     │
     └──────┬───────┘       └──────┬───────┘       └──────┬───────┘
            │                      │                      │
            ▼                      ▼                      ▼
     ┌──────────────┐       ┌──────────────┐       ┌──────────────┐
     │  VERIFY_CRC  │       │  VERIFY_SIG  │       │  REBOOT      │
     │  Check CRC  │       │  Signature   │       │  Normal Boot │
     └──────┬───────┘       └──────┬───────┘       └──────────────┘
            │                      │                      │
     ┌──────┴───────┐       ┌──────┴───────┐
     │              │       │              │
     ▼              ▼       ▼              ▼
┌─────────┐   ┌─────────┐ ┌─────────┐   ┌─────────┐
│  CRC_OK │   │ CRC_ERR │ │ SIG_OK  │   │ SIG_ERR │
└────┬────┘   └────┬────┘ └────┬────┘   └────┬────┘
     │              │       │              │
     ▼              ▼       ▼              ▼
┌─────────┐   ┌─────────┐ ┌─────────┐   ┌─────────┐
│  JUMP   │   │ ROLLBACK│ │ STORE   │   │ ABORT   │
│  APP    │   │  Bank   │ │  FW     │   │  Error  │
└─────────┘   └─────────┘ └────┬────┘   └─────────┘
                               │
                               ▼
                        ┌─────────────┐
                        │  SWITCH    │
                        │  BANK      │
                        └──────┬──────┘
                               │
                               ▼
                        ┌─────────────┐
                        │  JUMP_APP   │
                        │  New Bank   │
                        └─────────────┘
```

## Chi Tiết Từng State

### 1. RESET State

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              RESET                                          │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Entry: Power-on, NRST, Wake-up from Standby                                │
│                                                                             │
│  Actions:                                                                   │
│  - Initialize stack pointer                                                │
│  - Set vector table                                                         │
│  - Configure default clocks                                                │
│                                                                             │
│  Exit Condition: Hardware initialized                                       │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 2. INIT State

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              INIT                                           │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Entry: From RESET                                                          │
│                                                                             │
│  Actions:                                                                   │
│  - Configure PLL (250MHz)                                                   │
│  - Initialize Flash (0 wait states)                                        │
│  - Configure MPU (Secure/Non-Secure regions)                                │
│  - Initialize RNG                                                           │
│  - Initialize UART for debug                                               │
│  - Initialize Watchdog                                                     │
│                                                                             │
│  Exit Condition: All peripherals ready                                      │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 3. CHECK_BOOT State

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           CHECK_BOOT                                        │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Entry: From INIT                                                           │
│                                                                             │
│  Actions:                                                                   │
│  - Read BOOT0 pin (GPIO)                                                   │
│  - Read Option Bytes                                                       │
│  - Check for factory reset request                                         │
│                                                                             │
│  Transitions:                                                               │
│  - BOOT0 = 0 → NORMAL_BOOT                                                 │
│  - BOOT0 = 1 → UPDATE                                                      │
│  - Factory reset → FACTORY                                                 │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 4. NORMAL_BOOT State

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           NORMAL_BOOT                                        │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Entry: From CHECK_BOOT (BOOT0 = 0)                                         │
│                                                                             │
│  Actions:                                                                   │
│  - Read active bank from metadata                                          │
│  - Verify metadata integrity                                               │
│  - Check firmware version                                                   │
│                                                                             │
│  Exit Condition: Bank identified                                           │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 5. READ_META State

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           READ_META                                          │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Entry: From NORMAL_BOOT                                                    │
│                                                                             │
│  Actions:                                                                   │
│  - Read metadata from Flash                                                │
│  - Parse version, size, CRC                                                 │
│  - Validate magic number                                                    │
│                                                                             │
│  Exit Condition: Metadata read                                             │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 6. VERIFY_CRC State

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           VERIFY_CRC                                        │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Entry: From READ_META                                                      │
│                                                                             │
│  Actions:                                                                   │
│  - Compute CRC32 of firmware                                               │
│  - Compare with stored CRC                                                 │
│  - Log result                                                               │
│                                                                             │
│  Transitions:                                                               │
│  - CRC match → JUMP_APP                                                    │
│  - CRC mismatch → ROLLBACK                                                 │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 7. UPDATE State

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           UPDATE                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Entry: From CHECK_BOOT (BOOT0 = 1)                                        │
│                                                                             │
│  Actions:                                                                   │
│  - Initialize UART for firmware receive                                    │
│  - Configure DMA for fast reception                                        │
│  - Send ready signal to host                                               │
│  - Start timeout timer                                                     │
│                                                                             │
│  Exit Condition: Connection established                                     │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 8. WAIT_DATA State

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           WAIT_DATA                                         │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Entry: From UPDATE                                                         │
│                                                                             │
│  Actions:                                                                   │
│  - Receive firmware packets                                                │
│  - Store to temporary buffer                                               │
│  - Check packet sequence                                                   │
│  - Update progress indicator                                               │
│                                                                             │
│  Transitions:                                                               │
│  - All data received → VERIFY_SIG                                          │
│  - Timeout → ABORT                                                         │
│  - Invalid packet → ABORT                                                  │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 9. VERIFY_SIG State

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           VERIFY_SIG                                         │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Entry: From WAIT_DATA (all data received)                                 │
│                                                                             │
│  Actions:                                                                   │
│  - Compute SHA-256 hash of firmware                                        │
│  - Read public key from secure storage                                     │
│  - Verify RSA/ECDSA signature                                               │
│  - Log verification result                                                 │
│                                                                             │
│  Transitions:                                                               │
│  - Signature valid → STORE_FW                                             │
│  - Signature invalid → ABORT                                               │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 10. STORE_FW State

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           STORE_FW                                           │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Entry: From VERIFY_SIG (signature valid)                                   │
│                                                                             │
│  Actions:                                                                   │
│  - Erase target bank                                                       │
│  - Write firmware to Flash                                                 │
│  - Write metadata (version, CRC, signature)                               │
│  - Verify write                                                            │
│                                                                             │
│  Exit Condition: Firmware stored                                           │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 11. SWITCH_BANK State

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                          SWITCH_BANK                                         │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Entry: From STORE_FW                                                       │
│                                                                             │
│  Actions:                                                                   │
│  - Update metadata (set new bank active)                                   │
│  - Clear old bank active flag                                               │
│  - Synchronize metadata                                                    │
│  - Reset watchdog timer                                                    │
│                                                                             │
│  Exit Condition: Bank switched                                             │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 12. JUMP_APP State

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           JUMP_APP                                           │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Entry: From VERIFY_CRC (CRC OK) or SWITCH_BANK                            │
│                                                                             │
│  Actions:                                                                   │
│  - Disable interrupts                                                      │
│  - Deinitialize unused peripherals                                         │
│  - Set vector table offset to new bank                                      │
│  - Load stack pointer from new app                                         │
│  - Load PC from reset vector                                                │
│                                                                             │
│  Note: This is the final jump - no return                                  │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## State Transition Table

| From State | To State | Trigger | Action |
|------------|----------|---------|--------|
| RESET | INIT | Auto | Initialize HW |
| INIT | CHECK_BOOT | Ready | Check boot mode |
| CHECK_BOOT | NORMAL_BOOT | BOOT0=0 | Normal boot |
| CHECK_BOOT | UPDATE | BOOT0=1 | Enter update |
| CHECK_BOOT | FACTORY | Factory reset | Factory reset |
| NORMAL_BOOT | READ_META | Auto | Read metadata |
| READ_META | VERIFY_CRC | Meta ready | Verify CRC |
| VERIFY_CRC | JUMP_APP | CRC OK | Jump to app |
| VERIFY_CRC | ROLLBACK | CRC Error | Rollback |
| UPDATE | WAIT_DATA | Connected | Wait data |
| WAIT_DATA | VERIFY_SIG | Data complete | Verify sig |
| WAIT_DATA | ABORT | Timeout/Error | Abort |
| VERIFY_SIG | STORE_FW | Sig OK | Store firmware |
| VERIFY_SIG | ABORT | Sig Error | Abort |
| STORE_FW | SWITCH_BANK | Stored | Switch bank |
| SWITCH_BANK | JUMP_APP | Switched | Jump to new app |