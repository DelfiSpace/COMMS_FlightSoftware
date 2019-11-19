#include "COMMRadio.h"

extern DSerial serial;

COMMRadio* radioStub;
uint8_t onTransmitWrapper(){
    serial.println("transmitter stub!");
    return radioStub->onTransmit();
};
void onReceiveWrapper(uint8_t data){
    serial.println("receiver stub!");
    radioStub->onReceive(data);
};

COMMRadio::COMMRadio(DSPI &bitModeSPI_tx, DSPI &bitModeSPI_rx, DSPI &packetModeSPI, SX1276 &txRad, SX1276 &rxRad):
    bitSPI_tx(&bitModeSPI_tx), bitSPI_rx(&bitModeSPI_rx), packetSPI(&packetModeSPI), txRadio(&txRad), rxRadio(&rxRad)
{
    radioStub = this;
};



uint8_t COMMRadio::onTransmit()
{
    return 0x55;
    serial.println("TEST_TX");
    if(txReady && txIndex < txSize){
        txIndex++;
        return txBuffer[txIndex - 1];
    }else{
        txReady = false;
        txRadio->setIdleMode(false);
        return 0x00;
    }
};

void COMMRadio::onReceive(uint8_t data)
{
    //Replace rxArray by a push-pop construction?
    rxBuffer[rxIndex % 256] = data;

    //TODO: when sequence detected. forward bytes to 'packet' array
    //TODO: OPMODE?

    rxIndex++;

    // Debug print for now:
    serial.println("TEST_RX");
};

void COMMRadio::init(){
    serial.println("COMMS booting...");
    this->initTX();
    this->initRX();
};

void COMMRadio::initTX(){
    // Initialise TX values
    // GMSK:
    // Modem set to FSK, deviation set to 1/2 datarate, gaussian filter enabled
    txRadio->init();

    if(txRadio->ping()){

        txConfig.modem = MODEM_FSK;
        txConfig.filtertype = BT_0_5;
        txConfig.bandwidth = 15000;
        txConfig.power = 14;
        txConfig.fdev = 1200;
        txConfig.datarate = 2400;

        txRadio->setFrequency(435000000);

        txRadio->enableBitMode(*bitSPI_tx, 0, onTransmitWrapper);
        txRadio->setTxConfig(&txConfig);

        serial.println("TX Radio Settings Set");
    }
    else{
        serial.println("TX Radio not Found");
    }
};

void COMMRadio::initRX(){
    // Initialise RX values
    // GMSK:
    // Modem set to FSK, deviation set to 1/2 datarate, gaussian filter enabled
    if(rxRadio->ping()){
        rxConfig.modem = MODEM_FSK;
        rxConfig.filtertype = BT_0_5;
        rxConfig.bandwidth = 15000;
        rxConfig.bandwidthAfc = 83333;
        rxConfig.fdev = 1200;
        rxConfig.datarate = 2400;

        rxRadio->setFrequency(435000000);

        rxRadio->RxChainCalibration();
        rxRadio->enableBitMode(*bitSPI_rx, onReceiveWrapper, 0);

        rxRadio->setRxConfig(&rxConfig);
        rxRadio->startReceiver();
        serial.println("RX Radio Settings Set");
    }else{
        serial.println("RX Radio not Found");
    }
};

void COMMRadio::transmitData(uint8_t data[], uint8_t size){
    //txSize = size;
    //for(int i = 0; i < size; i++){
    //    txBuffer[i] = data[i];
    //}
    //txIndex = 0;
    //txReady = true;
    txRadio->setIdleMode(true);
    serial.println("TRANSMITTER IDLE MODE SET TRUE");
};

void COMMRadio::toggleReceivePrint(){
    //rxReady = ~rxReady;

    //TODO: OPMODE
    serial.print(rxReady);
};

unsigned char COMMRadio::readRXReg(unsigned char address){
    return (rxRadio->readRegister(address));
};

unsigned char COMMRadio::readTXReg(unsigned char address){
    return (txRadio->readRegister(address));
};

void COMMRadio::writeTXReg(unsigned char address, unsigned char value){
    (txRadio->writeRegister(address,value));
};

void COMMRadio::writeRXReg(unsigned char address, unsigned char value){
    (rxRadio->writeRegister(address,value));
};
