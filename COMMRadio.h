#include <LDPCDecoder.h>
#include <PQPacket.h>
#include "AX25Encoder.h"
#include "SX1276.h"
#include "DSPI.h"
#include "sx1276Enums.h"
#include "AX25Frame.h"
#include "AX25Synchronizer.h"
#include "Task.h"
#include "APSynchronizer.h"
#include "Console.h"

#ifndef COMMRADIO_H_
#define COMMRADIO_H_
#define PACKET_SIZE    100
#define UPRAMP_BYTES   70
#define DOWNRAMP_BYTES 20

#define TX_MAX_FRAMES 10
#define RX_MAX_FRAMES 10

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

    volatile bool txEnabled = false;
    volatile bool txPacketReady = false;

    int txIndex = 0;    //current byte index in TX frame
    int txBitIndex = 0; //current bit index in TX frame
    int txFlagQue = 0;  //how many flags are qued. (priority over frame)
    bool txIdleMode = false; //false: turn off tx after transmission, true: stay on after transmission

    AX25Encoder encoder;

    PQPacket txPacketBuffer[TX_MAX_FRAMES];
    int txPacketsInBuffer = 0;
    int txPacketBufferIndex = 0;

    PQPacket rxPacketBuffer[RX_MAX_FRAMES];
    int rxPacketBufferIndex = 0;
    int rxPacketsInBuffer = 0;

    AX25Synchronizer AX25Sync = AX25Synchronizer(rxPacketBuffer, rxPacketBufferIndex);


public:
    COMMRadio(DSPI &bitModeSPI_tx, DSPI &bitModeSPI_rx, DSPI &packetModeSPI, SX1276 &txRad, SX1276 &rxRad);
    bool notified( void );

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
