# Dual Bank + Resume Strategy

## Banks
- Bank A: Active
- Bank B: Update target

## Metadata Structure (Flash)
- active_bank
- update_in_progress
- last_written_address
- firmware_size
- crc

## Resume Flow

Khi boot:
- Nếu update_in_progress = true:
    → resume từ last_written_address

## Power Loss Case
- Ghi metadata sau mỗi chunk
- Không switch bank cho đến khi verify OK

## Safety Rules
- Không overwrite bank đang chạy
- Chỉ switch khi firmware valid