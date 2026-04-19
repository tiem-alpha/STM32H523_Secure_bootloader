# UART Protocol Design

## Packet Format
| Field | Size | Description |
|------|------|------------|
| START | 1B | 0xAA |
| LEN | 2B | Length của DATA |
| DATA | N | Payload |
| CRC16 | 2B | CRC của LEN + DATA |
| END | 1B | 0x55 |

## Notes
- CRC check trước khi xử lý
- Reject nếu sai format

## Commands
- START_UPDATE
- SEND_CHUNK
- END_UPDATE
- ACK / NACK

## Error Handling
- Timeout → reset state
- CRC fail → request resend