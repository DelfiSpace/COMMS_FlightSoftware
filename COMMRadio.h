
#include "SX1276.h"
#include "DSPI.h"
#include "sx1276Enums.h"


class COMMRadio
{
protected:
    DSPI *bitSPI_tx;
    DSPI *bitSPI_rx;
    DSPI *packetSPI;

    SX1276 *txRadio;
    SX1276 *rxRadio;

    TxConfig_t txConfig;
    RxConfig_t rxConfig;

    uint8_t txBuffer[256] = {0};
    uint8_t rxBuffer[256] = {0};
    uint8_t txSize = 0;

    volatile bool txReady = false;
    volatile bool rxReady = false;

    uint8_t txIndex = 0;
    uint8_t rxIndex = 0;

public:
    COMMRadio(DSPI &bitModeSPI_tx, DSPI &bitModeSPI_rx, DSPI &packetModeSPI, SX1276 &txRad, SX1276 &rxRad);
    void init();
    void initTX();
    void initRX();
    void onReceive(uint8_t data);
    uint8_t onTransmit();

    void transmitData(uint8_t data[], uint8_t size);
    void toggleReceivePrint();

};
