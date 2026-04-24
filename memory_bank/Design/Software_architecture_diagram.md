# Software Architecture Diagram

## Tổng Quan Phần Mềm

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    SOFTWARE ARCHITECTURE LAYERS                             │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│                        APPLICATION LAYER                                    │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐              │
│  │   App Bank A   │ │   App Bank B   │ │  Bootloader    │              │
│  │  (Active FW)   │ │  (Update FW)   │ │  (Secure BL)   │              │
│  └─────────────────┘ └─────────────────┘ └─────────────────┘              │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
                                  │
┌─────────────────────────────────┼───────────────────────────────────────────┐
│                        MIDDLEWARE LAYER                                     │
├─────────────────────────────────┼───────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐              │
│  │  Secure Boot    │ │  Update Manager │ │  Crypto Module  │              │
│  │  Manager        │ │                 │ │                 │              │
│  │  - Signature    │ │  - Bank Switch  │ │  - SHA256       │              │
│  │  - Public Key   │ │  - Metadata     │ │  - RSA/ECDSA    │              │
│  │  - CRC Verify   │ │  - Rollback     │ │  - RNG          │              │
│  └─────────────────┘ └─────────────────┘ └─────────────────┘              │
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
```

## STM32H523 Hardware Crypto Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    HARDWARE CRYPTO MODULES                                  │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│                        CRYPTO PERIPHERALS                                    │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                         RNG (Random Number Generator)               │   │
│  │  ┌────────────────────────────────────────────────────────────────┐  │   │
│  │  │  Features:                                                    │  │   │
│  │  │  - 32-bit random numbers                                     │  │   │
│  │  │  - NIST SP 800-90B compliant                                │  │   │
│  │  │  - Seedable for cryptographic use                            │  │   │
│  │  │  - Interrupt/DMA support                                    │  │   │
│  │  └────────────────────────────────────────────────────────────────┘  │   │
│  │                                                                   │   │
│  │  API: HAL_RNG_GenerateRandom32(), HAL_RNG_GenerateRandom()       │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                    PKA (Public Key Accelerator)                   │   │
│  │  ┌────────────────────────────────────────────────────────────────┐  │   │
│  │  │  Features:                                                    │  │   │
│  │  │  - RSA up to 4096-bit                                        │  │   │
│  │  │  - ECDSA P-256, P-384, P-521                                 │  │   │
│  │  │  - ECC key generation                                        │  │   │
│  │  │  - Modular exponentiation                                     │  │   │
│  │  └────────────────────────────────────────────────────────────────┘  │   │
│  │                                                                   │   │
│  │  API: HAL_PKA_Encrypt(), HAL_PKA_Decrypt(), HAL_PKA_Verify()     │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                         HASH (SHA256)                              │   │
│  │  ┌────────────────────────────────────────────────────────────────┐  │   │
│  │  │  Features:                                                    │  │   │
│  │  │  - SHA-256, SHA-384, SHA-512                                 │  │   │
│  │  │  - MD5                                                       │  │   │
│  │  │  - Hardware acceleration                                     │  │   │
│  │  │  - DMA support                                               │  │   │
│  │  └────────────────────────────────────────────────────────────────┘  │   │
│  │                                                                   │   │
│  │  API: HAL_HASH_Init(), HAL_HASH_Append(), HAL_HASH_Finish()      │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                      CRYP (AES/DES)                                │   │
│  │  ┌────────────────────────────────────────────────────────────────┐  │   │
│  │  │  Features:                                                    │  │   │
│  │  │  - AES-128/192/256                                           │  │   │
│  │  │  - ECB, CBC, CTR, GCM modes                                 │  │   │
│  │  │  - DES/Triple DES                                            │  │   │
│  │  │  - DMA support                                               │  │   │
│  │  └────────────────────────────────────────────────────────────────┘  │   │
│  │                                                                   │   │
│  │  API: HAL_CRYP_Init(), HAL_CRYP_Encrypt(), HAL_CRYP_Decrypt()     │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## HAL Driver Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        HAL DRIVER STRUCTURE                                 │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│                           HAL CORE                                          │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  stm32h5xx_hal.h (Main HAL header)                                 │   │
│  │  stm32h5xx_hal_conf.h (Configuration)                              │   │
│  │  stm32h5xx_hal_def.h (Common definitions)                         │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
                                  │
        ┌─────────────────────────┼─────────────────────────┐
        │                         │                         │
        ▼                         ▼                         ▼
┌───────────────┐         ┌───────────────┐         ┌───────────────┐
│  UART HAL    │         │  FLASH HAL   │         │  CRYPTO HAL   │
├───────────────┤         ├───────────────┤         ├───────────────┤
│ HAL_UART_Def  │         │ HAL_FLASH_Def │         │ HAL_RNG_Def   │
│ HAL_UART      │         │ HAL_FLASH     │         │ HAL_RNG       │
│ HAL_UARTEx    │         │ HAL_FLASHEx   │         │ HAL_HASH_Def  │
└───────────────┘         └───────────────┘         │ HAL_HASH      │
                                                      │ HAL_PKA_Def   │
                                                      │ HAL_PKA       │
                                                      │ HAL_CRYP_Def  │
                                                      │ HAL_CRYP      │
                                                      └───────────────┘
```

## Baremetal Implementation

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      BAREMETAL ARCHITECTURE                                 │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│                        NO RTOS - DIRECT HARDWARE                            │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                    Main Loop (Super Loop)                         │   │
│  │                                                                   │   │
│  │  int main(void)                                                   │   │
│  │  {                                                                │   │
│  │      HAL_Init();              // Initialize HAL                 │   │
│  │      SystemClock_Config();    // Configure clocks               │   │
│  │      MX_GPIO_Init();          // Initialize GPIO                │   │
│  │      MX_UART_Init();          // Initialize UART                │   │
│  │      MX_RNG_Init();           // Initialize RNG                 │   │
│  │      MX_HASH_Init();          // Initialize HASH                │   │
│  │      MX_PKA_Init();           // Initialize PKA                 │   │
│  │                                                                       │   │
│  │      while (1)                                                       │   │
│  │      {                                                                │   │
│  │          // State machine handling                                  │   │
│  │          Bootloader_StateMachine();                                │   │
│  │          Watchdog_Refresh();                                       │   │
│  │      }                                                                │   │
│  │  }                                                                    │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                    Interrupt Handlers                              │   │
│  │                                                                   │   │
│  │  void USART1_IRQHandler(void)    // UART RX interrupt             │   │
│  │  void DMA1_Channel1_IRQHandler(void) // DMA complete              │   │
│  │  void RNG_IRQHandler(void)      // RNG ready interrupt           │   │
│  │  void HardFault_Handler(void)   // Fault handling                │   │
│  │  void SysTick_Handler(void)     // System timer                  │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## TrustZone Software Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    TRUSTZONE ARCHITECTURE                                    │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│                        SECURE WORLD                                         │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Secure Bootloader (Secure World)                                  │   │
│  │                                                                   │   │
│  │  ┌───────────────┐ ┌───────────────┐ ┌───────────────┐             │   │
│  │  │ Secure Core  │ │ Crypto HW    │ │ Secure Data  │             │   │
│  │  │               │ │               │ │               │             │   │
│  │  │ - BL Entry   │ │ - RNG        │ │ - Keys       │             │   │
│  │  │ - State Mach │ │ - PKA        │ │ - Certs      │             │   │
│  │  │ - MPU Config │ │ - HASH       │ │ - Metadata   │             │   │
│  │  │ - TrustZone  │ │ - CRYP       │ │ - Config     │             │   │
│  │  └───────────────┘ └───────────────┘ └───────────────┘             │   │
│  │                                                                   │   │
│  │  Secure Flash: 0x08000000 - 0x08008000 (32KB)                   │   │
│  │  Secure SRAM: 0x20000000 - 0x20008000 (32KB)                     │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                      NON-SECURE WORLD                                       │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Application (Non-Secure World)                                    │   │
│  │                                                                   │   │
│  │  ┌───────────────┐ ┌───────────────┐ ┌───────────────┐             │   │
│  │  │ App Core     │ │ Non-Secure   │ │ App Data     │             │   │
│  │  │               │ │ Peripherals  │ │               │             │   │
│  │  │ - Main Loop  │ │               │ │               │             │   │
│  │  │ - State Mach │ │ - UART       │ │ - Buffers    │             │   │
│  │  │ - Call Gate  │ │ - GPIO       │ │ - Stack      │             │   │
│  │  │ - veneer     │ │ - TIM        │ │ - Heap       │             │   │
│  │  └───────────────┘ └───────────────┘ └───────────────┘             │   │
│  │                                                                   │   │
│  │  Non-Secure Flash: 0x08008000 - 0x08030000 (Bank A/B)           │   │
│  │  Non-Secure SRAM: 0x20008000 - 0x20018000 (96KB)                 │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                        SECURITY ATTRIBUTION                                 │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  SAU (Secure Attribution Unit) Configuration                     │   │
│  │                                                                   │   │
│  │  Region 0: Secure Flash    0x08000000 - 0x08008000  NS=0         │   │
│  │  Region 1: Bank A          0x08008000 - 0x0801C000  NS=1         │   │
│  │  Region 2: Bank B          0x0801C000 - 0x08030000  NS=1         │   │
│  │  Region 3: Metadata        0x08030000 - 0x08032000  NS=0         │   │
│  │  Region 4: Secure SRAM     0x20000000 - 0x20008000  NS=0         │   │
│  │  Region 5: Non-Secure SRAM 0x20008000 - 0x20018000  NS=1         │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Module Structure

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        MODULE HIERARCHY                                      │
└─────────────────────────────────────────────────────────────────────────────┘

Secure Bootloader/
├── Core/
│   ├── Inc/
│   │   ├── main.h              # Main header
│   │   ├── stm32h5xx_hal_conf.h # HAL configuration
│   │   ├── stm32h5xx_it.h     # Interrupt handlers
│   │   └── log.h              # Debug logging
│   └── Src/
│       ├── main.c             # Main entry
│       ├── system_stm32h5xx.c # System init
│       ├── stm32h5xx_hal_msp.c # HAL MSP
│       └── stm32h5xx_it.c    # ISR implementations
│
├── Drivers/
│   └── STM32H5xx_HAL_Driver/
│       ├── Src/
│       │   ├── stm32h5xx_hal.c        # HAL core
│       │   ├── stm32h5xx_hal_rng.c   # RNG driver
│       │   ├── stm32h5xx_hal_hash.c  # HASH driver
│       │   ├── stm32h5xx_hal_pka.c   # PKA driver
│       │   ├── stm32h5xx_hal_cryp.c  # CRYP driver
│       │   ├── stm32h5xx_hal_flash.c # FLASH driver
│       │   ├── stm32h5xx_hal_uart.c  # UART driver
│       │   └── stm32h5xx_hal_dma.c   # DMA driver
│       └── Inc/
│           ├── stm32h5xx_hal_rng.h
│           ├── stm32h5xx_hal_hash.h
│           ├── stm32h5xx_hal_pka.h
│           ├── stm32h5xx_hal_cryp.h
│           └── ...
│
├── Secure/
│   ├── bl_secure.c            # Secure functions
│   ├── bl_verify.c           # Signature verification
│   ├── bl_crypto.c           # Crypto operations
│   └── bl_metadata.c         # Metadata management
│
└── NonSecure/
    ├── bl_update.c            # Firmware update
    ├── bl_comm.c             # Communication
    └── bl_rollback.c         # Rollback handling
```

## API Summary

### HAL Crypto APIs

```c
// RNG API
HAL_StatusTypeDef HAL_RNG_Init(RNG_HandleTypeDef *hrng);
uint32_t HAL_RNG_GenerateRandom32(RNG_HandleTypeDef *hrng);
HAL_StatusTypeDef HAL_RNG_GenerateRandom(RNG_HandleTypeDef *hrng, uint32_t *pRandom);

// HASH API
HAL_StatusTypeDef HAL_HASH_Init(HASH_HandleTypeDef *hhash, uint32_t algo);
HAL_StatusTypeDef HAL_HASH_Append(HASH_HandleTypeDef *hhash, uint8_t *pInBuffer, uint32_t Size);
HAL_StatusTypeDef HAL_HASH_Finish(HASH_HandleTypeDef *hhash, uint8_t *pInBuffer, uint32_t Size, uint8_t *pOutBuffer);

// PKA API
HAL_StatusTypeDef HAL_PKA_Init(PKA_HandleTypeDef *hpka);
HAL_StatusTypeDef HAL_PKA_RSASSA_Verify(PKA_HandleTypeDef *hpka, PKA_RSASSA_VerifyParamsTypeDef *pParams);

// CRYP API
HAL_StatusTypeDef HAL_CRYP_Init(CRYP_HandleTypeDef *hcryp);
HAL_StatusTypeDef HAL_CRYP_Encrypt(CRYP_HandleTypeDef *hcryp, uint32_t *pInput, uint16_t Size, uint32_t *pOutput);
HAL_StatusTypeDef HAL_CRYP_Decrypt(CRYP_HandleTypeDef *hcryp, uint32_t *pInput, uint16_t Size, uint32_t *pOutput);
```

### Custom APIs

```c
// Bootloader APIs
int BL_Init(void);
int BL_VerifyFirmware(uint32_t bank);
int BL_SwitchBank(uint32_t targetBank);
int BL_JumpToApp(uint32_t appAddress);

// Update APIs
int UPDATE_Start(void);
int UPDATE_ReceiveData(uint8_t *data, uint32_t len);
int UPDATE_Complete(void);

// Crypto APIs
int CRYPTO_ComputeHash(uint8_t *data, uint32_t len, uint8_t *hash);
int CRYPTO_VerifySignature(uint8_t *hash, uint8_t *signature, uint8_t *publicKey);
int CRYPTO_GenerateRandom(uint8_t *buffer, uint32_t len);
```

## Memory Usage

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        MEMORY ALLOCATION                                    │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│                        FLASH (512KB)                                        │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Secure Bootloader (32KB)                                                  │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  .text    (Code)         : 20KB                                    │   │
│  │  .rodata  (Const)        : 4KB                                     │   │
│  │  .secure  (Keys)         : 4KB                                     │   │
│  │  .vector  (Vectors)     : 1KB                                     │   │
│  │  .reserved               : 3KB                                    │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Bank A (224KB)                                                            │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  .text    (Code)         : 180KB                                   │   │
│  │  .rodata  (Const)        : 10KB                                    │   │
│  │  .data    (Data)         : 2KB                                     │   │
│  │  .bss     (Zero-init)    : 10KB                                    │   │
│  │  .stack   (Stack)        : 8KB                                     │   │
│  │  .heap    (Heap)         : 14KB                                    │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Bank B (224KB)                                                            │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Same structure as Bank A                                          │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Metadata (8KB)                                                            │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Bank A Info: 4KB                                                   │   │
│  │  Bank B Info: 4KB                                                  │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│                        SRAM (128KB)                                         │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Secure SRAM (32KB)                                                        │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Secure Stack: 4KB                                                  │   │
│  │  Secure Heap: 4KB                                                   │   │
│  │  Secure Data: 24KB                                                 │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
│  Non-Secure SRAM (96KB)                                                    │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │  Non-Secure Stack: 8KB                                              │   │
│  │  Non-Secure Heap: 8KB                                               │   │
│  │  Non-Secure Data: 80KB                                              │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```