# Project Overview - STM32H523 Bootloader

## Goal
Thiết kế bootloader cho STM32H523 với các tính năng:
- Firmware update qua UART
- Secure boot (verify chữ ký firmware)
- Dual bank firmware
- Resume khi mất điện giữa chừng
- Trustzone

## Key Features
- UART protocol custom
- CRC16 kiểm tra integrity
- Digital signature verification (ECDSA / RSA)
- Dual bank switching an toàn
- Power-loss recovery

## Constraints
- RAM hạn chế
- Flash chia thành 2 bank
- Không dùng OS (bare-metal)

## Success Criteria
- Update firmware không brick
- Verify firmware trước khi chạy
- Resume update sau reset