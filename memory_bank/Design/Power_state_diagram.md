# Power State Diagram

## Tổng Quan Power States

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        POWER STATE MACHINE                                  │
└─────────────────────────────────────────────────────────────────────────────┘

                              ┌─────────────┐
                              │   RUN       │
                              │  (Active)   │
                              └──────┬──────┘
                                     │
                    ┌────────────────┼────────────────┐
                    │                │                │
                    ▼                ▼                ▼
           ┌──────────────┐ ┌──────────────┐ ┌──────────────┐
           │    SLEEP     │ │    STOP      │ │   STANDBY    │
           │  (CPU Idle)  │ │ (Low Power)  │ │ (Deep Sleep) │
           └──────┬───────┘ └──────┬───────┘ └──────┬───────┘
                  │                │                │
                  └────────────────┼────────────────┘
                                     │
                              ┌──────┴──────┐
                              │   SHUTDOWN  │
                              │ (Power Off) │
                              └─────────────┘
```

## Chi Tiết Từng Power State

### 1. RUN Mode (Active)

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              RUN MODE                                        │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Description: Full performance, all peripherals active                      │
│                                                                             │
│  Power Consumption: ~150mA @ 250MHz                                        │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  CPU: 250MHz (PLL)                                                  │   │
│  │  Flash: Enabled (0 Wait State)                                     │   │
│  │  SRAM: Full Power                                                  │   │
│  │  Peripherals: All Active                                            │   │
│  │  Crypto: Available (RNG, PKA)                                       │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Use Cases:                                                                  │
│  - Firmware update process                                                  │
│  - Signature verification                                                   │
│  - Application execution                                                   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 2. SLEEP Mode (CPU Idle)

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                             SLEEP MODE                                       │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Description: CPU halted, peripherals active                                │
│                                                                             │
│  Power Consumption: ~45mA                                                   │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  CPU: Halted (WFI instruction)                                    │   │
│  │  Flash: Enabled                                                     │   │
│  │  SRAM: Retention                                                    │   │
│  │  Peripherals: Active (UART, Timer, DMA)                            │   │
│  │  Wake-up: Any enabled interrupt                                    │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Entry:  __WFI() instruction                                               │
│  Exit:   Any enabled interrupt                                              │
│                                                                             │
│  Use Cases:                                                                  │
│  - Waiting for UART data                                                    │
│  - Idle loop                                                                │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 3. STOP Mode (Low Power)

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                             STOP MODE                                       │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Description: Core powered down, retention SRAM                            │
│                                                                             │
│  Power Consumption: ~5mA (with RTC), ~1mA (without RTC)                   │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  CPU: Powered Off                                                  │   │
│  │  Flash: Partial Retention                                          │   │
│  │  SRAM: Retention (32KB Secure + 96KB Non-Secure)                  │   │
│  │  RTC: Running (if enabled)                                          │   │
│  │  LPUART: Available (if enabled)                                    │   │
│  │  Wake-up: RTC, LPUART, GPIO                                        │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Entry:  HAL_PWR_EnterSTOPMode()                                           │
│  Exit:   Any enabled wake-up source                                        │
│                                                                             │
│  Use Cases:                                                                  │
│  - Waiting for firmware update                                              │
│  - Low power idle                                                          │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 4. STANDBY Mode (Deep Sleep)

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           STANDBY MODE                                       │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Description: Maximum power saving, minimal retention                       │
│                                                                             │
│  Power Consumption: ~2µA (with RTC), ~0.5µA (without RTC)                  │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  CPU: Powered Off                                                  │   │
│  │  Flash: Off                                                       │   │
│  │  SRAM: Off (or 4KB retention option)                             │   │
│  │  RTC: Running (if enabled)                                        │   │
│  │  I/O: State retention                                              │   │
│  │  Wake-up: RTC, GPIO, BOR                                          │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Entry:  HAL_PWR_EnterSTANDBYMode()                                        │
│  Exit:   RTC alarm, GPIO toggle, BOR, Reset                                │
│                                                                             │
│  Use Cases:                                                                  │
│  - Deep sleep between updates                                               │
│  - Battery powered operation                                               │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 5. SHUTDOWN Mode

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           SHUTDOWN MODE                                      │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Description: Complete power off                                            │
│                                                                             │
│  Power Consumption: ~50nA                                                   │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  CPU: Off                                                          │   │
│  │  Flash: Off                                                       │   │
│  │  SRAM: Off                                                        │   │
│  │  RTC: Off                                                         │   │
│  │  I/O: High impedance                                              │   │
│  │  Wake-up: Reset only                                              │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Entry:  Power off or VDD removal                                          │
│  Exit:   Reset only                                                        │
│                                                                             │
│  Use Cases:                                                                  │
│  - Complete power down                                                      │
│  - Battery removal                                                          │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Power State Transitions

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        STATE TRANSITIONS                                     │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────┐    WFI     ┌─────────┐   DeepSleep  ┌──────────┐
│   RUN   │──────────▶│  SLEEP  │─────────────▶│  STANDBY │
└─────────┘            └─────────┘              └──────────┘
     ▲                    │                          │
     │                    │ Wake-up                  │ Reset
     │                    ▼                          ▼
     │              ┌─────────┐              ┌──────────┐
     └─────────────│   RUN   │              │  SHUTDOWN │
                   └─────────┘              └──────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│  Transition Events:                                                         │
│                                                                             │
│  RUN → SLEEP:    WFI() instruction                                          │
│  SLEEP → RUN:    Any interrupt                                             │
│  RUN → STANDBY:  HAL_PWR_EnterSTANDBYMode()                                │
│  STANDBY → RUN:  RTC alarm, GPIO, BOR                                       │
│  STANDBY → SHUTDOWN: Reset pin                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Power Consumption Summary

| Mode | Current | Wake-up Time | Use Case |
|------|---------|--------------|----------|
| RUN | 150mA | 0µs | Active processing |
| SLEEP | 45mA | 2µs | Idle wait |
| STOP | 1-5mA | 10µs | Low power wait |
| STANDBY | 0.5-2µA | 50µs | Deep sleep |
| SHUTDOWN | 50nA | 200µs | Power off |

## Secure Bootloader Power Profile

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    BOOTLOADER POWER PROFILE                                 │
└─────────────────────────────────────────────────────────────────────────────┘

Phase 1: Power-on → RUN (150mA)
  - Boot from shutdown
  - Initialize clocks

Phase 2: Signature Verify → RUN (150mA)
  - Load firmware
  - Compute SHA-256
  - Verify RSA/ECDSA signature

Phase 3: Bank Switch → SLEEP (45mA)
  - Update metadata
  - Switch active bank

Phase 4: Jump to App → STANDBY (2µA)
  - Transfer to application
  - Enter low power mode
```