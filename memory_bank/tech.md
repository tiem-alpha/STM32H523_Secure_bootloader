# Technical Context

## MCU
- STM32H523
- Flash dual bank
- Có hardware crypto (nếu dùng)

## Tools
- STM32CubeIDE
- STM32 HAL / LL
- Python tool để gửi firmware qua UART

## Protocol
UART packet:
[START][LEN(2B)][DATA][CRC16][END]

## Security
- Public key lưu trong flash (bootloader)
- Firmware chứa signature
- Verify trước khi jump

## Memory Layout (draft)
- Bootloader: 0x08000000
- Bank A (App1)
- Bank B (App2)
- Metadata region (state, flags)

## Dependencies
- CRC16 lib
- Crypto lib (mbedTLS hoặc custom)