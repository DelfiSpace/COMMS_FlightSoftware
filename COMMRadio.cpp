#include "COMMRadio.h"

extern DSerial serial;

unsigned char reverseByteOrder(unsigned char x)
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
    //serial.println("SEND PACKET (stub)!");
    MAP_Timer32_clearInterruptFlag(TIMER32_1_BASE);
    radioStub->sendPacketAX25();
}

COMMRadio::COMMRadio(DSPI &bitModeSPI_tx, DSPI &bitModeSPI_rx, DSPI &packetModeSPI, SX1276 &txRad, SX1276 &rxRad):
    bitSPI_tx(&bitModeSPI_tx), bitSPI_rx(&bitModeSPI_rx), packetSPI(&packetModeSPI), txRadio(&txRad), rxRadio(&rxRad)
{
    radioStub = this;
};



uint8_t COMMRadio::onTransmit(){
    //NOTE, SPI Bus is configured to MSB_first, meaning that the bit transmitted first should be the MSB.

    uint8_t outputByte = 0;
    if(txPacketReady){
        for(int i = 0; i < 8; i++){// Send 8bits per call
            //Start Sending
            if(!txUprampSend){ //First Check the UpRamp!!
                if(encoder.bitsInBuffer == 0){  //check if no more bits in send buffer
                    uint8_t inBit = (0x7E >> txBitIndex) & 0x01;
                    outputByte = outputByte | (encoder.txBit( inBit , false) << (7-i));
                    txBitIndex++;
                }else{
                    //send the Buffered Bit
                    outputByte = outputByte | (encoder.txBit(0, false) << (7-i));
                }

                //check if txBitIndex is high enough to roll over to next byte
                if(txBitIndex>=8){
                    txBitIndex = 0;
                    txIndex++;
                }

                //check if we're done sending Upramp
                if(txIndex >= UPRAMP_BYTES){
                    txUprampSend = true;
                    txIndex = 0;
                }
            } else if(!txPacketSend){
                if(encoder.bitsInBuffer == 0){
                    //tx is ready for next bit
                        uint8_t inBit = (txRFMessageBuffer[txIndex] >> txBitIndex) & 0x01;
                        outputByte = outputByte | (encoder.txBit( inBit , true) << (7-i));
                        txBitIndex++;
                }else{
                    //out of bytes, pad the last parts with zero
                    outputByte = outputByte | (encoder.txBit(0, true) << (7-i));
                    //serial.print("[P]");
                }
                if(txBitIndex >= 8){
                    txIndex++;
                    txBitIndex = 0;
                }
                if(txIndex >= txSize){
                    txIndex = 0;
                    txPacketSend = true;
                }
            }else if(!txDownrampSend){ //Lastly, check DownRamp
                if(encoder.bitsInBuffer == 0){  //check if no more bits in send buffer
                    uint8_t inBit = (0x7E >> txBitIndex) & 0x01;
                    outputByte = outputByte | (encoder.txBit( inBit , false) << (7-i));
                    txBitIndex++;
                }else{
                    //send the Buffered Bit
                    outputByte = outputByte | (encoder.txBit(0, false) << (7-i));
                }

                //check if txBitIndex is high enough to roll over to next byte
                if(txBitIndex>=8){
                    txBitIndex = 0;
                    txIndex++;
                }

                //check if we're done sending Downramp
                if(txIndex >= DOWNRAMP_BYTES){
                    txDownrampSend = true;
                    txIndex = 0;
                }
            }else {
                //no more data available, so stuff byte with zeros for transmission
                outputByte = outputByte | (encoder.txBit(0, false) << (7-i));
            }

            //serial.print(txBitIndex, HEX);
        }

        if(txPacketSend && txUprampSend && txDownrampSend){
            //end of transmission
            txRadio->setIdleMode(false);
            txSize = 0;
            txReady = true;
            txPacketReady = false;
        }

        if(false){
            for(int i = 0; i < 8; i++){
                serial.print((outputByte >> (7-i)) & 0x01, DEC);
            }
            serial.print("|");
        }
        //serial.print(outputByte, HEX);
        return outputByte;
    }else{
        txRadio->setIdleMode(false);
        txSize = 0;
        txReady = true;
        txPacketReady = false;
        return 0x00;
    }
};

void COMMRadio::onReceive(uint8_t data)
{
    //Note: SPI Bus is configured MSB First, meaning that from the received Byte, the MSB was the first received Bit
    if(rxReady){
        for(int i = 0; i < 8; i++){
            uint8_t inBit = encoder.NRZIdecodeBit((data >> (7-i))& 0x01);
            inBit = encoder.descrambleBit(inBit);
            if(AX25Sync.rxBit(inBit)){
                this->AX25RXFrameBuffer[AX25FramesInBuffer] = AX25Sync.receivedFrame;
                this->AX25FramesInBuffer++;
            }
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
        txConfig.fdev = 4800;
        txConfig.datarate = 9600;

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
        rxConfig.fdev = 4800;
        rxConfig.datarate = 9600;

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

    TXDestination[6] = 0xE0;//(('A' & 0x0F) << 1) | 0xE0;
    TXSource[6] = 0x61;//(('B' & 0x0F) << 1) | 0x61;
    TXFrame.setAdress(TXDestination, TXSource);
    TXFrame.setControl(false);
    TXFrame.setPID(0xF0);
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

    txPacketReady = true;
    txIndex = 0;
    txBitIndex = 0;
    txPacketSend = false;
    txUprampSend = false;
    txDownrampSend = false;
    txRadio->setIdleMode(true);
    //rxPrint = true;
    //rxReady = false;
}

void COMMRadio::toggleReceivePrint(){
    //serial.print("Toggle RX Print: ");
    //serial.println(rxReady);

    serial.println();
    serial.println(" ============ ");
    serial.print("Amount of Frames in Buffer: ");
    serial.print(this->AX25FramesInBuffer, DEC);
    serial.println();
    serial.println(" ============ ");
    for(int k = 0; k < AX25FramesInBuffer; k++){
        uint8_t* frameData = this->AX25RXFrameBuffer[k].getBytes();
        for(int j = 0; j < this->AX25RXFrameBuffer[k].getSize(); j++){
            serial.print(frameData[j], HEX);
            serial.print("|");
        }
        serial.println();
        serial.println(" ============ ");
    }
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
