# Interrupt Diagram

## Tổng Quan Interrupt

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        INTERRUPT ARCHITECTURE                               │
└─────────────────────────────────────────────────────────────────────────────┘

                        ┌─────────────────┐
                        │     NVIC        │
                        │ (Nested Vector  │
                        │  Interrupt Ctrl)│
                        └────────┬────────┘
                                 │
        ┌────────────────────────┼────────────────────────┐
        │                        │                        │
        ▼                        ▼                        ▼
┌───────────────┐      ┌───────────────┐      ┌───────────────┐
│  Secure       │      │  Non-Secure   │      │   System      │
│  Peripherals  │      │  Peripherals  │      │  Exceptions   │
├───────────────┤      ├───────────────┤      ├───────────────┤
│ - RNG         │      │ - UART        │      │ - Reset       │
│ - PKA         │      │ - I2C         │      │ - NMI         │
│ - FLASH       │      │ - SPI         │      │ - HardFault   │
│ - HSEM        │      │ - GPIO        │      │ - SysTick     │
└───────────────┘      └───────────────┘      └───────────────┘
```

## Interrupt Priority Layout

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        PRIORITY LEVELS                                     │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                            │
│  Priority 0 (Highest)                                                      │
│  ┌────────────────────────────────────────────────────────────────────┐   │
│  │  System Exceptions                                                 │   │
│  │  - Reset, NMI, HardFault, MemManage, BusFault, UsageFault        │   │
│  └────────────────────────────────────────────────────────────────────┘   │
│                                                                            │
│  Priority 1                                                               │
│  ┌────────────────────────────────────────────────────────────────────┐   │
│  │  Secure Interrupts (TrustZone)                                     │   │
│  │  - RNG (Random Number Generator)                                   │   │
│  │  - PKA (Public Key Accelerator)                                    │   │
│  │  - FLASH (Flash Controller)                                         │   │
│  │  - HSEM (Hardware Semaphore)                                        │   │
│  └────────────────────────────────────────────────────────────────────┘   │
│                                                                            │
│  Priority 2-3                                                            │
│  ┌────────────────────────────────────────────────────────────────────┐   │
│  │  Communication Peripherals                                          │   │
│  │  - UART (USART1, USART2, UART)                                      │   │
│  │  - SPI (SPI1, SPI2, SPI3)                                            │   │
│  │  - I2C (I2C1, I2C2, I2C3)                                            │   │
│  └────────────────────────────────────────────────────────────────────┘   │
│                                                                            │
│  Priority 4-5 (Lowest)                                                    │
│  ┌────────────────────────────────────────────────────────────────────┐   │
│  │  General Peripherals                                                │   │
│  │  - GPIO (EXTI)                                                      │   │
│  │  - DMA                                                              │   │
│  │  - TIM (Timers)                                                     │   │
│  └────────────────────────────────────────────────────────────────────┘   │
│                                                                            │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Interrupt Sources Chi Tiết

### Secure World Interrupts (Priority 1)

| IRQ | Name | Source | Function | Priority |
|-----|------|--------|----------|----------|
| 0 | RNG | RNG | Random Number Gen | 1 |
| 1 | PKA | PKA | Crypto Operations | 1 |
| 2 | FLASH | FLASH | Flash Operations | 1 |
| 3 | HSEM | HSEM | Semaphore | 1 |

### Non-Secure World Interrupts (Priority 2-5)

| IRQ | Name | Source | Function | Priority |
|-----|------|--------|----------|----------|
| 10 | USART1 | USART1 | UART Debug | 2 |
| 11 | USART2 | USART2 | UART Update | 2 |
| 12 | SPI1 | SPI1 | SPI Comm | 3 |
| 13 | I2C1 | I2C1 | I2C Comm | 3 |
| 20 | TIM1 | TIM1 | Timer | 4 |
| 21 | TIM2 | TIM2 | Timer | 4 |
| 30 | DMA1 | DMA1 | DMA Transfer | 5 |
| 31 | DMA2 | DMA2 | DMA Transfer | 5 |
| 40 | EXTI0 | GPIO | External Interrupt | 5 |
| 41 | EXTI1 | GPIO | External Interrupt | 5 |

## Interrupt Flow Diagram

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        INTERRUPT FLOW                                      │
└─────────────────────────────────────────────────────────────────────────────┘

    ┌──────────┐
    │  Event   │ (UART RX, Timer, etc.)
    └────┬─────┘
         │
         ▼
┌─────────────────┐
│   Check Mask   │──── No ──▶ Ignore
└────┬────────────┘
     │ Yes
     ▼
┌─────────────────┐
│  Check Priority│
│  (Secure vs    │
│   Non-Secure)  │
└────┬────────────┘
     │
     ▼
┌─────────────────┐
│   NVIC Check   │──── No ──▶ Drop (Lower priority)
└────┬────────────┘
     │ Yes
     ▼
┌─────────────────┐
│  Save Context  │ (R0-R3, R12, LR, PC, xPSR)
└────┬────────────┘
     │
     ▼
┌─────────────────┐
│  Vector Table   │──────▶ ISR Handler
└────┬────────────┘
     │
     ▼
┌─────────────────┐
│   Execute ISR  │
└────┬────────────┘
     │
     ▼
┌─────────────────┐
│ Restore Context│
└────┬────────────┘
     │
     ▼
┌─────────────────┐
│   Return to    │
│   Main Code    │
└─────────────────┘
```

## TrustZone Interrupt Handling

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    TRUSTZONE INTERRUPT ROUTING                               │
└─────────────────────────────────────────────────────────────────────────────┘

                    ┌─────────────────────┐
                    │      VTOR           │
                    │ (Vector Table Addr) │
                    └──────────┬──────────┘
                               │
           ┌───────────────────┼───────────────────┐
           │                   │                   │
           ▼                   ▼                   ▼
    ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
    │   Secure    │     │  Non-Secure  │     │   System    │
    │   Vector    │     │   Vector    │     │  Exceptions │
    │   Table     │     │   Table     │     │   Table     │
    └─────────────┘     └─────────────┘     └─────────────┘
           │                   │                   │
           │                   │                   │
           ▼                   ▼                   ▼
    ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
    │ Secure ISR │     │Non-Secure ISR│    │  Exception  │
    │ (TrustZone)│     │ (App)       │    │  Handler    │
    └─────────────┘     └─────────────┘     └─────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│  Security Attribution:                                                      │
│  - Secure interrupts → Secure Vector Table                                  │
│  - Non-Secure interrupts → Non-Secure Vector Table (via veneer)           │
│  - System exceptions → Always Secure Vector Table                          │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Interrupt Latency

| Interrupt Type | Latency (Cycles) | Description |
|----------------|------------------|-------------|
| Secure ISR | 12 | Direct entry |
| Non-Secure ISR | 15 | Via veneer/switch |
| System Exception | 10 | Fast path |
| DMA | 0 (Async) | No CPU intervention |