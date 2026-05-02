### Communication Architecture

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

