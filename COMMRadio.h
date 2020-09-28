#include "PQPacket.h"
#include "AX25Encoder.h"
#include "SX1276.h"
#include "DSPI.h"
#include "sx1276Enums.h"
#include "AX25Frame.h"
#include "AX25Synchronizer.h"
#include "Task.h"
#include "Console.h"
#include "InternalCommandHandler.h"
#include "PQ9Frame.h"
#include "PQ9Message.h"
#include "busMaster.h"

#ifndef COMMRADIO_H_
#define COMMRADIO_H_
#define PACKET_SIZE    100
#define UPRAMP_BYTES   200
#define DOWNRAMP_BYTES 20

#define COMMS_RESET_PORT GPIO_PORT_P9
#define COMMS_RESET_PIN  GPIO_PIN1

#define PA_PORT GPIO_PORT_P2
#define PA_ENABLE_PIN GPIO_PIN7
#define PA_MED_PIN GPIO_PIN6
#define PA_HIGH_PIN GPIO_PIN0

#define TX_MAX_FRAMES 300
#define RX_MAX_FRAMES 300
#define OVERRIDE_MAX_FRAMES 300

#define TX_TIMEOUT 5 //timeout to transmit packages

class COMMRadio : public Task
{
protected:
    DSPI *bitSPI_tx;
    DSPI *bitSPI_rx;
    DSPI *packetSPI;


    InternalCommandHandler<PQ9Frame,PQ9Message> *cmdHandler;
    BusMaster<PQ9Frame, PQ9Message> *busOverride;


    volatile bool txEnabled = false;
    volatile bool txPacketReady = false;

    //txRadioSettings:
    uint8_t powerByte = 0;


    int txIndex = 0;    //current byte index in TX frame
    int txBitIndex = 0; //current bit index in TX frame
    int txFlagQue = 0;  //how many flags are qued. (priority over frame)
    bool txIdleMode = false; //false: turn off tx after transmission, true: stay on after transmission

    AX25Encoder encoder;

    PQPacket txPacketBuffer[TX_MAX_FRAMES];
    int txPacketBufferWriteIndex = 0;
    int txPacketBufferReadIndex = 0;

    PQPacket rxPacketBuffer[RX_MAX_FRAMES];
    int rxPacketsInBuffer = 0;
    int rxPacketBufferIndex = 0;

    PQPacket overridePacketBuffer[OVERRIDE_MAX_FRAMES];
    int overridePacketBufferIndex = 0;
    int overridePacketsInBuffer = 0;

    AX25Synchronizer AX25Sync = AX25Synchronizer();




public:
    COMMRadio(DSPI &bitModeSPI_tx, DSPI &bitModeSPI_rx, DSPI &packetModeSPI, SX1276 &txRad, SX1276 &rxRad);
    void setcmdHandler(InternalCommandHandler<PQ9Frame,PQ9Message> &cmdhand);
    void setbusMaster(BusMaster<PQ9Frame, PQ9Message> &busmstr);

    void resetEPS();

    bool notified( void );

    int txPacketsInBuffer();

    SX1276 *txRadio;
    SX1276 *rxRadio;
    TxConfig_t txConfig;
    RxConfig_t rxConfig;

    unsigned int txBitrate = PQPACKET_DOWNLINK_BITRATE;
    unsigned int rxBitrate = PQPACKET_UPLINK_BITRATE;

    uint8_t targetPAPower = 1;
    volatile bool doEnableFlag = false;

    void runTask();
    void init();
    void initTX();
    void initRX();

    void onReceive(uint8_t data);
    uint8_t onTransmit();

    bool transmitData(uint8_t data[], uint8_t size);
    void setIdleMode(bool idle);

    bool quePacketAX25(uint8_t dataIn[], uint8_t size);
    void enableTransmit();
    void disableTransmit();
    void enablePA(uint8_t targetLevel);
    void disablePA();

    uint8_t TXDestination[7] = {('G' & 0x0F) << 1,('R' & 0x0F) << 1,('O' & 0x0F) << 1,('U' & 0x0F) << 1,('N' & 0x0F) << 1,('D' & 0x0F) << 1,0xFF};
    uint8_t TXSource[7]      = {('D' & 0x0F) << 1,('L' & 0x0F) << 1,('F' & 0x0F) << 1,('I' & 0x0F) << 1,('P' & 0x0F) << 1,('Q' & 0x0F) << 1,0xFF};
    volatile bool txTimeout = false;

    uint8_t getNumberOfRXFrames();
    uint8_t getSizeOfRXFrame();
    uint8_t* getRXFrame();
    void popFrame();


    signed short getRXRSSI();

    signed short lastFreqError;
    signed short lastRSSI;

};

#endif
