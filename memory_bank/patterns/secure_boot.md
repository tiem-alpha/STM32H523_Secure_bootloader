# Secure Boot

## Flow

1. Load firmware header
2. Extract signature
3. Hash firmware
4. Verify bằng public key
5. Nếu OK → run
6. Nếu fail → rollback

## Storage
- Public key: trong bootloader (read-only)
- Signature: trong firmware image

## Notes
- Không trust firmware nếu chưa verify
- Có thể dùng ECDSA P-256