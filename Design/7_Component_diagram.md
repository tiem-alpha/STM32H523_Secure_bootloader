## Component Diagram
- Hệ thống có những khối nào
- Interface giữa các khối
- Kết nối runtime
- Ai gọi ai
  
```Mermaid
flowchart TB

    Boot[Boot Core]

    UART[UART Service]
    Parser[Protocol Parser]
    Verify[Verify Service]
    Bank[Bank Switch Service]
    FlashSvc[Flash Service]

    CRC[CRC Engine]
    SHA[SHA256 Engine]

    Queue[Queue]

    UARTDrv[UART Driver]
    FlashDrv[Flash Driver]
    TimerDrv[Timer Driver]
    IWDG[IWDG Driver]
    TZ[TrustZone Init]

    HAL_UART[HAL UART]
    HAL_FLASH[HAL FLASH]

    Boot --> UART
    Boot --> Verify
    Boot --> Bank
    Boot --> IWDG
    Boot --> TZ

    UART --> Parser
    UART --> Queue
    UART --> UARTDrv
    UART --> TimerDrv

    Verify --> CRC
    Verify --> SHA

    Bank --> FlashSvc

    FlashSvc --> FlashDrv

    UARTDrv --> HAL_UART
    FlashDrv --> HAL_FLASH
```
