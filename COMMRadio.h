#include "AX25Encoder.h"
#include "SX1276.h"
#include "DSPI.h"
#include "sx1276Enums.h"
#include "DSerial.h"
#include "AX25Frame.h"
#include "AX25Synchronizer.h"
#include "Task.h"

#ifndef COMMRADIO_H_
#define COMMRADIO_H_
#define PACKET_SIZE    100
#define UPRAMP_BYTES   70
#define DOWNRAMP_BYTES 20

#define AX25_TX_FRAME_BUFFER 20

//July 14, 2009 Hallvard Furuseth
static const unsigned char BitsSetTable256[256] =
{
#   define B2(n) n,     n+1,     n+1,     n+2
#   define B4(n) B2(n), B2(n+1), B2(n+1), B2(n+2)
#   define B6(n) B4(n), B4(n+1), B4(n+1), B4(n+2)
    B6(0), B6(1), B6(1), B6(2)
};

class COMMRadio : public Task
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
    //uint8_t* txRFMessageBuffer;

    volatile bool txReady = false;
    volatile bool txPacketReady = false;

    volatile bool rxReady = false;

    int txIndex = 0;
    int txBitIndex = 0;
    int txFlagInsert = 0;

    AX25Frame AX25RXFrameBuffer[AX25_RX_FRAME_BUFFER];
    int AX25RXframesInBuffer = 0;
    int AX25RXbufferIndex = 0;

    AX25Frame AX25TXFrameBuffer[AX25_TX_FRAME_BUFFER];
    int AX25TXframesInBuffer = 0;
    int AX25TXbufferIndex = 0;

    AX25Encoder encoder;
    AX25Frame TXFrame;
    AX25Synchronizer AX25Sync = AX25Synchronizer(AX25RXFrameBuffer, AX25RXframesInBuffer, AX25RXbufferIndex);


public:
    COMMRadio(DSPI &bitModeSPI_tx, DSPI &bitModeSPI_rx, DSPI &packetModeSPI, SX1276 &txRad, SX1276 &rxRad);
    void runTask();
    void init();
    void initTX();
    void initRX();
    void onReceive(uint8_t data);
    uint8_t onTransmit();

    bool transmitData(uint8_t data[], uint8_t size);
    void toggleReceivePrint();

    void sendPacket();
    bool quePacketAX25(uint8_t dataIn[], uint8_t size);
    void sendPacketAX25();

    uint8_t TXDestination[7] = {0x82,0x98,0x98,0x40,0x40,0x40,0xFF};
    uint8_t TXSource[7]      = {0x40,0x40,0x40,0x40,0x40,0x40,0xFF};
};

#endif
