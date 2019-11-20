#include "SX1276.h"
#include "DSPI.h"
#include "sx1276Enums.h"
#include "DSerial.h"
#include "AX25Frame.h"

#ifndef COMMRADIO_H_
#define COMMRADIO_H_
#define PACKET_SIZE    100
#define RF_MSG_SIZE    120

//July 14, 2009 Hallvard Furuseth
static const unsigned char BitsSetTable256[256] =
{
#   define B2(n) n,     n+1,     n+1,     n+2
#   define B4(n) B2(n), B2(n+1), B2(n+1), B2(n+2)
#   define B6(n) B4(n), B4(n+1), B4(n+1), B4(n+2)
    B6(0), B6(1), B6(1), B6(2)
};

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

    uint8_t txPacketBuffer[PACKET_SIZE] = {0};
    uint8_t UPRAMP_BYTES = 10;
    uint8_t DOWNRAMP_BYTES = 10;
    uint8_t txRFMessageBuffer[RF_MSG_SIZE] = {0};
    uint8_t countBits[256];

    uint8_t preamble[10] = {0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};

    uint8_t rxPreambleDetectBuffer[10] = {0};
    uint8_t txSize = 0;

    volatile bool txReady = false;
    volatile bool rxReady = false;
    volatile bool rxPrint = false;

    int txIndex = 0;
    int rxIndex = 0;

    uint8_t countNumberOfBits(uint8_t value);
    uint8_t countSimilarBits(uint8_t value1[], uint8_t value2[], int size, int value1Offset);
    uint8_t countSimilarBitsInverted(uint8_t value1[], uint8_t value2[], int size, int value1Offset);

    AX25Frame ax25Frame;

public:
    COMMRadio(DSPI &bitModeSPI_tx, DSPI &bitModeSPI_rx, DSPI &packetModeSPI, SX1276 &txRad, SX1276 &rxRad);
    void init();
    void initTX();
    void initRX();
    void onReceive(uint8_t data);
    uint8_t onTransmit();

    bool transmitData(uint8_t data[], uint8_t size);
    void toggleReceivePrint();
    unsigned char readRXReg(unsigned char address);
    unsigned char readTXReg(unsigned char address);
    void writeRXReg(unsigned char address, unsigned char value);
    void writeTXReg(unsigned char address, unsigned char value);
    void sendPacket();
    void sendPacketAX25();
};

#endif
