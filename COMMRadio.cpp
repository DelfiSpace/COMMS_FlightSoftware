#include "COMMRadio.h"

extern DSerial serial;

COMMRadio* radioStub;
uint8_t onTransmitWrapper(){
    //serial.println("transmitter stub!");
    return radioStub->onTransmit();
};
void onReceiveWrapper(uint8_t data){
    //serial.println("receiver stub!");
    radioStub->onReceive(data);
};
void sendPacketWrapper(){
    serial.println("SEND PACKET (stub)!");
    MAP_Timer32_clearInterruptFlag(TIMER32_1_BASE);
    radioStub->sendPacketAX25();
}

COMMRadio::COMMRadio(DSPI &bitModeSPI_tx, DSPI &bitModeSPI_rx, DSPI &packetModeSPI, SX1276 &txRad, SX1276 &rxRad):
    bitSPI_tx(&bitModeSPI_tx), bitSPI_rx(&bitModeSPI_rx), packetSPI(&packetModeSPI), txRadio(&txRad), rxRadio(&rxRad)
{
    radioStub = this;
};



uint8_t COMMRadio::onTransmit()
{
    if(txIndex < RF_MSG_SIZE){
        txIndex++;
        //serial.print(txRFMessageBuffer[txIndex - 1],HEX);
        //serial.println("");
        return txRFMessageBuffer[txIndex - 1];
    }else{
        txRadio->setIdleMode(false);
        txSize = 0;
        txReady = true;
        return 0x00;
    }
};

void COMMRadio::onReceive(uint8_t data)
{
    if(rxReady){
        //Replace rxArray by a push-pop construction?
        rxPreambleDetectBuffer[rxIndex % 10] = data;

        //TODO: when sequence detected. forward bytes to 'packet' array
        //preAmble detection
        if(countSimilarBits(rxPreambleDetectBuffer, preamble, 10, rxIndex) > 78 || countSimilarBitsInverted(rxPreambleDetectBuffer, preamble, 10, rxIndex) > 78){
            serial.println("RX DETECTED~!");
            serial.print("====  ");
            serial.println("");
        }else{
            //serial.println("No preamble found");
        }

        //TODO: OPMODE?
        rxIndex++;

        // Debug print for now:
        if(rxPrint){
            serial.print(data, HEX);
        }
    }
};

uint8_t COMMRadio::countNumberOfBits(uint8_t value){
    return BitsSetTable256[value];
}

uint8_t COMMRadio::countSimilarBits(uint8_t value1[], uint8_t value2[], int size, int value1Offset){
    uint8_t count = 0;
    for(int i = 0; i < size; i++){
        uint8_t testValue = (uint8_t) ~(value1[(i-size+value1Offset) % size] ^ value2[2]);
        count += countNumberOfBits(testValue);
    }
    //serial.print(count, DEC);
    //serial.println("");
    return count;
}

uint8_t COMMRadio::countSimilarBitsInverted(uint8_t value1[], uint8_t value2[], int size, int value1Offset){
    uint8_t count = 0;
        for(int i = 0; i < size; i++){
            uint8_t testValue = (uint8_t) ~(value1[(i-size+value1Offset) % size] ^ ((uint8_t) ~value2[2]));
            count += countNumberOfBits(testValue);
        }
        //serial.print(count, DEC);
        //serial.println("");
        return count;
}

void COMMRadio::init(){
    serial.println("COMMS booting...");
    this->initTX();
    this->initRX();
    MAP_Timer32_initModule(TIMER32_1_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT,
                    TIMER32_PERIODIC_MODE);
    this->rxReady = true;
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
        txConfig.fdev = 600;
        txConfig.datarate = 1200;

        txRadio->setFrequency(435000000);

        txRadio->enableBitMode(*bitSPI_tx, 0, onTransmitWrapper);
        txRadio->setTxConfig(&txConfig);

        serial.println("TX Radio Settings Set");
    }
    else{
        serial.println("TX Radio not Found");
    }

    txReady = true;
};

void COMMRadio::initRX(){
    // Initialise RX values
    // GMSK:
    // Modem set to FSK, deviation set to 1/2 data-rate, Gaussian filter enabled
    if(rxRadio->ping()){
        rxConfig.modem = MODEM_FSK;
        rxConfig.filtertype = BT_0_5;
        rxConfig.bandwidth = 15000;
        rxConfig.bandwidthAfc = 83333;
        rxConfig.fdev = 600;
        rxConfig.datarate = 1200;

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

bool COMMRadio::transmitData(uint8_t data[], uint8_t size){
    if(txSize + size > 256 || !txReady){
        return false;
    }

    for(int i = 0; i < size; i++){
        txPacketBuffer[txSize + i] = data[i];
    }
    txSize += size;

    //serial.println(txBuffer[0]);
    serial.println("Setting Timer");

    MAP_Timer32_registerInterrupt(TIMER32_1_INTERRUPT, &sendPacketWrapper);
    MAP_Timer32_setCount(TIMER32_1_BASE, 48000000);
    MAP_Timer32_startTimer(TIMER32_1_BASE, true);

    //MAP_Timer32_setCount(TIMER32_1_BASE, 48000000);
    //MAP_Timer32_startTimer(TIMER32_1_BASE, true);

    return true;

};

void COMMRadio::sendPacket(){
    txReady = false; //currently sending, do not allow new data

    //Zero stuff Packet
    for(int i = txSize; i < PACKET_SIZE; i++){
        txPacketBuffer[i] = 0;
    }

    //Create Preamble:
    for(int i = 0; i < 10; i++){
        this->txRFMessageBuffer[i] = preamble[i];
    }
    //Fill RF Message Buffer now (TODO: PROTOCOL + FEC)
    serial.println("============================");
    serial.println("PACKET:: ");
    for(int i = 0; i < PACKET_SIZE; i++){
        this->txRFMessageBuffer[10+i] = txPacketBuffer[i];
    }
    for(int i = 0; i < RF_MSG_SIZE; i++){
            serial.print(this->txRFMessageBuffer[i], HEX);
    }
    serial.println("");
    serial.println("============================");

    txIndex = 0;
    txRadio->setIdleMode(true);
}

void COMMRadio::sendPacketAX25(){
    txReady = false; //currently sending, do not allow new data

    //Zero stuff Packet
    for(int i = txSize; i < PACKET_SIZE; i++){
        txPacketBuffer[i] = 0;
    }

    //Create UpRamp:
    for(int i = 0; i < UPRAMP_BYTES; i++){
        this->txRFMessageBuffer[i] = encoder.AX25_FLAG;
    }
    //Fill RF Message(TODO: PROTOCOL + FEC)
    for(int i = 0; i < PACKET_SIZE; i++){
        this->txRFMessageBuffer[UPRAMP_BYTES+i] = txPacketBuffer[i];
    }
    //Create DownRamp
    for(int i = 0; i < RF_MSG_SIZE; i++){
        this->txRFMessageBuffer[UPRAMP_BYTES+PACKET_SIZE+i] = encoder.AX25_FLAG;
    }

    //Scramble RF message
    for(int i = 0; i < RF_MSG_SIZE; i++){
        txRFMessageBuffer[i] = encoder.NRZIencodeByte(encoder.scrambleByte(txRFMessageBuffer[i]));
    }

    // Print RF Packet for Debug
    serial.println("============================");
    serial.println("PACKET:: ");
    for(int i = 0; i < RF_MSG_SIZE; i++){
            serial.print(this->txRFMessageBuffer[i], HEX);
    }
    serial.println("");
    serial.println("============================");

    txIndex = 0;
    txRadio->setIdleMode(true);
}

void COMMRadio::toggleReceivePrint(){
    //serial.print("Toggle RX Print: ");
    //serial.println(rxReady);
    rxPrint = !rxPrint;

    //TODO: OPMODE
    //serial.print(rxReady);
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
