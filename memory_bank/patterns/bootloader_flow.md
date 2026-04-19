# Bootloader Flow

## Boot Sequence

1. Reset
2. Check metadata
3. Nếu firmware valid → jump
4. Nếu update pending → vào update mode

## Update Flow

1. Receive START_UPDATE
2. Erase target bank
3. Receive chunks
4. Write flash
5. Verify CRC toàn bộ
6. Verify signature
7. Update metadata
8. Switch bank
9. Reset

## Jump to App
- Set MSP
- Jump to reset handler