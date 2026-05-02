



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