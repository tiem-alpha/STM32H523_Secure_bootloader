# Sequence Diagram

## Tổng Quan Sequence Diagrams

### 1. Firmware Update Sequence

```
┌─────────┐     ┌─────────┐     ┌─────────┐     ┌─────────┐
│ Laptop  │     │  ESP32  │     │ STM32H5 │     │  Flash  │
│ (Host)  │     │ (Bridge)│     │   BL    │     │ (Bank)  │
└────┬────┘     └────┬────┘     └────┬────┘     └────┬────┘
     │               │               │               │
     │ 1. Connect    │               │               │
     │──────────────▶│               │               │
     │               │               │               │
     │ 2. TCP Handshake             │               │
     │◀──────────────│               │               │
     │               │               │               │
     │ 3. Send FW + Sig             │               │
     │──────────────▶│───UART───────▶│               │
     │               │               │               │
     │               │               │ 4. Store Data │
     │               │               │──────────────▶│
     │               │               │               │
     │               │               │ 5. Verify Sig │
     │               │               │──────────────▶│
     │               │               │               │
     │               │               │ 6. CRC Check  │
     │               │               │──────────────▶│
     │               │               │               │
     │               │               │ 7. Update OK  │
     │◀──────────────│◀──────────────│               │
     │               │               │               │
     │ 8. ACK        │               │               │
     │──────────────▶│               │               │
     │               │               │               │
```

### 2. Boot Sequence

```
┌─────────┐     ┌─────────┐     ┌─────────┐     ┌─────────┐
│  Power  │     │   BL    │     │ Metadata │     │  Flash  │
│   On    │     │  Start  │     │  Table  │     │  Bank   │
└────┬────┘     └────┬────┘     └────┬────┘     └────┬────┘
     │               │               │               │
     │ 1. Reset     │               │               │
     │──────────────▶│               │               │
     │               │               │               │
     │               │ 2. Init HW   │               │
     │               │──────────────▶│               │
     │               │               │               │
     │               │ 3. Read Active│              │
     │               │    Bank       │               │
     │               │──────────────▶│               │
     │               │               │               │
     │               │ 4. Return Bank│               │
     │               │◀──────────────│               │
     │               │               │               │
     │               │ 5. Read Metadata              │
     │               │──────────────▶│               │
     │               │               │               │
     │               │ 6. Return Meta│               │
     │               │◀──────────────│               │
     │               │               │               │
     │               │ 7. Verify CRC│               │
     │               │──────────────▶│               │
     │               │               │               │
     │               │ 8. CRC OK     │               │
     │               │◀──────────────│               │
     │               │               │               │
     │               │ 9. Jump to App│               │
     │               │──────────────▶│               │
     │               │               │               │
```

### 3. Bank Switch Sequence

```
┌─────────┐     ┌─────────┐     ┌─────────┐     ┌─────────┐
│   BL    │     │  Bank A │     │  Bank B │     │ Metadata│
│         │     │ (Active)│     │(Update) │     │  Table  │
└────┬────┘     └────┬────┘     └────┬────┘     └────┬────┘
     │               │               │               │
     │ 1. New FW Ready│               │               │
     │               │               │               │
     │               │ 2. Read Bank B │               │
     │               │──────────────▶│               │
     │               │               │               │
     │               │ 3. Return Data│               │
     │               │◀──────────────│               │
     │               │               │               │
     │               │ 4. Verify Sig │               │
     │               │──────────────▶│               │
     │               │               │               │
     │               │ 5. Sig Valid  │               │
     │               │◀──────────────│               │
     │               │               │               │
     │               │               │ 6. Mark Valid │
     │               │               │───────────────▶│
     │               │               │               │
     │               │               │ 7. Update OK  │
     │               │               │◀──────────────│
     │               │               │               │
     │ 8. Update Metadata           │               │
     │──────────────────────────────────────────────▶│
     │               │               │               │
     │ 9. Switch Bank│               │               │
     │◀──────────────│               │               │
     │               │               │               │
     │10. Set Active │               │               │
     │──────────────▶│               │               │
     │               │               │               │
     │11. Clear Active│              │               │
     │◀──────────────│               │               │
     │               │               │               │
```

### 4. Signature Verification Sequence

```
┌─────────┐     ┌─────────┐     ┌─────────┐     ┌─────────┐
│   BL    │     │  SHA256 │     │   PKA   │     │  Flash  │
│         │     │  Hash   │     │  (Crypto)│     │  (Key)  │
└────┬────┘     └────┬────┘     └────┬────┘     └────┬────┘
     │               │               │               │
     │ 1. Start Verify               │               │
     │──────────────▶│               │               │
     │               │               │               │
     │ 2. Read FW   │               │               │
     │──────────────▶│               │               │
     │               │               │               │
     │ 3. Compute Hash              │               │
     │◀──────────────│               │               │
     │               │               │               │
     │ 4. Read PubKey│               │               │
     │──────────────▶│               │               │
     │               │               │               │
     │ 5. Return Key│               │               │
     │◀──────────────│               │               │
     │               │               │               │
     │ 6. Read Sig  │               │               │
     │──────────────▶│               │               │
     │               │               │               │
     │ 7. Return Sig│               │               │
     │◀──────────────│               │               │
     │               │               │               │
     │ 8. Verify    │               │               │
     │──────────────▶│               │               │
     │               │               │               │
     │ 9. Result    │               │               │
     │◀──────────────│               │               │
     │               │               │               │
     │10. Valid/Invalid              │               │
     │◀──────────────│               │               │
     │               │               │               │
```

### 5. UART Data Reception Sequence

```
┌─────────┐     ┌─────────┐     ┌─────────┐     ┌─────────┐
│  ESP32  │     │  UART   │     │   DMA   │     │  SRAM   │
│         │     │  (HW)   │     │         │     │ Buffer  │
└────┬────┘     └────┬────┘     └────┬────┘     └────┬────┘
     │               │               │               │
     │ 1. Send Data │               │               │
     │──────────────▶│               │               │
     │               │               │               │
     │               │ 2. RX Not Empty              │
     │               │──────────────▶│               │
     │               │               │               │
     │               │               │ 3. DMA Transfer              │
     │               │               │──────────────▶│
     │               │               │               │
     │               │               │ 4. Complete  │               │
     │               │               │◀──────────────│               │
     │               │               │               │
     │               │ 5. IRQ       │               │
     │               │◀──────────────│               │
     │               │               │               │
     │               │ 6. Notify BL │               │
     │               │──────────────▶│               │
     │               │               │               │
     │               │ 7. Process   │               │
     │               │──────────────▶│               │
     │               │               │               │
```

### 6. Error Handling Sequence

```
┌─────────┐     ┌─────────┐     ┌─────────┐     ┌─────────┐
│   BL    │     │  UART   │     │  Flash  │     │  Watch  │
│         │     │  Error  │     │  Error  │     │  Dog    │
└────┬────┘     └────┬────┘     └────┬────┘     └────┬────┘
     │               │               │               │
     │ 1. Error Detect               │               │
     │◀──────────────│               │               │
     │               │               │               │
     │ 2. Log Error  │               │               │
     │──────────────▶│               │               │
     │               │               │               │
     │ 3. Retry?    │               │               │
     │◀──────────────│               │               │
     │               │               │               │
     │ 4. Yes        │               │               │
     │──────────────▶│               │               │
     │               │               │               │
     │ 5. Max Retry? │               │               │
     │──────────────▶│               │               │
     │               │               │               │
     │ 6. No         │               │               │
     │◀──────────────│               │               │
     │               │               │               │
     │ 7. Rollback   │               │               │
     │──────────────▶│               │               │
     │               │               │               │
     │ 8. Reset Sys │               │               │
     │──────────────▶│               │               │
     │               │               │               │
     │ 9. Watchdog  │               │               │
     │◀──────────────│               │               │
     │               │               │               │
```

## Sequence Summary

| Sequence | Trigger | Main Actions | Duration |
|-----------|---------|--------------|----------|
| Firmware Update | Host request | Receive → Verify → Store | ~30s |
| Boot | Power on/RST | Init → Read Meta → Verify → Jump | ~100ms |
| Bank Switch | Update complete | Mark valid → Switch → Jump | ~50ms |
| Signature Verify | Boot/Update | Hash → Verify → Result | ~200ms |
| UART Reception | Data received | DMA → IRQ → Process | ~10ms |
| Error Handling | Any error | Log → Retry/Rollback | Variable |