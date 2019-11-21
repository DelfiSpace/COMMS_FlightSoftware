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

unsigned char reverseByte(unsigned char x)
{
    static const unsigned char table[] = {
        0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
        0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
        0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
        0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
        0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
        0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
        0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
        0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
        0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
        0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
        0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
        0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
        0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
        0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
        0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
        0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
        0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
        0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
        0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
        0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
        0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
        0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
        0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
        0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
        0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
        0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
        0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
        0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
        0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
        0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
        0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
        0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
    };
    return table[x];
}


COMMRadio::COMMRadio(DSPI &bitModeSPI_tx, DSPI &bitModeSPI_rx, DSPI &packetModeSPI, SX1276 &txRad, SX1276 &rxRad):
    bitSPI_tx(&bitModeSPI_tx), bitSPI_rx(&bitModeSPI_rx), packetSPI(&packetModeSPI), txRadio(&txRad), rxRadio(&rxRad)
{
    radioStub = this;
};



uint8_t COMMRadio::onTransmit()
{
    uint8_t outputByte;
    if(txIndex < (txSize + UPRAMP_BYTES + DOWNRAMP_BYTES) ){
        if(txIndex < UPRAMP_BYTES){
            outputByte = encoder.AX25_FLAG;
        }else if(txIndex < (UPRAMP_BYTES + txSize)){
            outputByte = txRFMessageBuffer[txIndex - UPRAMP_BYTES];
        }else{
            outputByte = encoder.AX25_FLAG;
        }

        txIndex++;
        if(txIndex >= (txSize + UPRAMP_BYTES + DOWNRAMP_BYTES) ){
            txRadio->setIdleMode(false);
            txSize = 0;
            txReady = true;
        }

    }else{
        txRadio->setIdleMode(false);
        txSize = 0;
        outputByte = 0x00;
        txReady = true;
    }
    serial.print(outputByte, HEX);
    serial.print("|");

    //Prepare outputByte using bitwise operations



    return outputByte;
};

void COMMRadio::onReceive(uint8_t data)
{
    if(rxReady){
        //Replace rxArray by a push-pop construction?
        rxPreambleDetectBuffer[rxIndex % 10] = reverseByte(data);//encoder.NRZIdecodeByte(data);

        //TODO: when sequence detected. forward bytes to 'packet' array
        //preAmble detection
        //if(countSimilarBits(rxPreambleDetectBuffer, preamble, 10, rxIndex) > 78 || countSimilarBitsInverted(rxPreambleDetectBuffer, preamble, 10, rxIndex) > 78){
        //    serial.println("RX DETECTED~!");
        //    serial.print("====  ");
        //    serial.println("");
        //}else{
        //    //serial.println("No preamble found");
        //}

        //TODO: OPMODE?
        rxIndex++;

        // Debug print for now:
        if(rxPrint){
            serial.print(reverseByte(data), HEX);
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

    //Zero stuff the information Packet
    for(int i = txSize; i < PACKET_SIZE; i++){
        txPacketBuffer[i] = 0;
    }

    TXFrame.setAdress(TXDestination, TXSource);
    TXFrame.setControl(false);
    TXFrame.setPacket(txPacketBuffer, txSize);
    TXFrame.calculateFCS();

    txSize = TXFrame.getSize();
    txRFMessageBuffer = TXFrame.getBytes();
    // Print RF Packet for Debug
    serial.println("============================");
    serial.println("PACKET:: ");
    for(int i = 0; i < txSize; i++){
            serial.print(this->txRFMessageBuffer[i], HEX);
            serial.print("|");
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
