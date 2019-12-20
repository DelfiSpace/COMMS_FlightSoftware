#include <CLTUPacket.h>
#include <LDPCDecoder.h>
#include "AX25Encoder.h"
#include "SX1276.h"
#include "DSPI.h"
#include "sx1276Enums.h"
#include "DSerial.h"
#include "AX25Frame.h"
#include "AX25Synchronizer.h"
#include "Task.h"
#include "APSynchronizer.h"

#ifndef COMMRADIO_H_
#define COMMRADIO_H_
#define PACKET_SIZE    100
#define UPRAMP_BYTES   70
#define DOWNRAMP_BYTES 20

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

    AX25Encoder encoder;

    CLTUPacket txCLTUBuffer[TX_FRAME_BUFFER];
    int txCLTUInBuffer = 0;
    int txCLTUBufferIndex = 0;

    CLTUPacket rxCLTUBuffer[RX_FRAME_BUFFER];
    int rxCLTUBufferIndex = 0;

    APSynchronizer APSync = APSynchronizer(rxCLTUBuffer, rxCLTUBufferIndex);
    AX25Synchronizer AX25Sync = AX25Synchronizer(rxCLTUBuffer, rxCLTUBufferIndex);

    LDPCDecoder LDPCdecoder;

    bool advancedMode = false;
    //true = AP, false = AX25;

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
    void toggleMode();

    void sendPacket();
    bool quePacketAX25(uint8_t dataIn[], uint8_t size);
    void sendPacketAX25();

    uint8_t TXDestination[7] = {0x82,0x98,0x98,0x40,0x40,0x40,0xFF};
    uint8_t TXSource[7]      = {0x40,0x40,0x40,0x40,0x40,0x40,0xFF};
    volatile bool txTimeout = false;

    uint8_t getNumberOfRXFrames();
    uint8_t getSizeOfRXFrame();
    uint8_t* getRXFrame();
    void popFrame();
};

#endif
