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
            } else if(txInsertFlag) {
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

                //check if we're done sending flags
                if(txIndex > 10){
                    txInsertFlag = false;
                    txIndex = 0;
                    //serial.println("INSERTED!");
                }
            } else if(!txPacketSend){
                if(encoder.bitsInBuffer == 0){
                    //tx is ready for next bit
                        uint8_t inBit = (AX25TXFrameBuffer[(AX25TXbufferIndex - AX25TXframesInBuffer) % AX25_TX_FRAME_BUFFER].getBytes()[txIndex] >> txBitIndex) & 0x01;
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
                if(txIndex >= AX25TXFrameBuffer[(AX25TXbufferIndex - AX25TXframesInBuffer) % AX25_TX_FRAME_BUFFER].getSize()){
                    txIndex = 0;
                    AX25TXframesInBuffer = AX25TXframesInBuffer - 1;
                    txInsertFlag = true;
                    //serial.println("Sent Packet");
                    if(AX25TXframesInBuffer == 0){
                        txPacketSend = true;
                    }
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
            //txReady = true;
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
        //txReady = true;
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
                this->AX25RXFrameBuffer[AX25RXframesInBuffer] = AX25Sync.receivedFrame;
                this->AX25RXframesInBuffer++;
            }
        }
    }
};

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
    this->quePacketAX25(data, size);
    //this->quePacketAX25(data, size);
    //this->quePacketAX25(data, size);
    //this->quePacketAX25(data, size);
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
//    txReady = false; //currently sending, do not allow new data
//
//    //Zero stuff Packet
//    for(int i = txSize; i < PACKET_SIZE; i++){
//        txPacketBuffer[i] = 0;
//    }
//
//    //Create Preamble:
//    for(int i = 0; i < 10; i++){
//        this->txRFMessageBuffer[i] = preamble[i];
//    }
//    //Fill RF Message Buffer now (TODO: PROTOCOL + FEC)
//    serial.println("============================");
//    serial.println("PACKET:: ");
//    for(int i = 0; i < PACKET_SIZE; i++){
//        this->txRFMessageBuffer[10+i] = txPacketBuffer[i];
//    }
//    for(int i = 0; i < RF_MSG_SIZE; i++){
//            serial.print(this->txRFMessageBuffer[i], HEX);
//    }
//    serial.println("");
//    serial.println("============================");
//
//    txIndex = 0;
//    txRadio->setIdleMode(true);
}

bool COMMRadio::quePacketAX25(uint8_t data[], uint8_t size){
    //Zero stuff the information Packet
    txReady = false;

    TXDestination[6] = 0xE0;//(('A' & 0x0F) << 1) | 0xE0;
    TXSource[6] = 0x61;//(('B' & 0x0F) << 1) | 0x61;
    AX25TXFrameBuffer[AX25TXbufferIndex].setAdress(TXDestination, TXSource);
    AX25TXFrameBuffer[AX25TXbufferIndex].setControl(false);
    AX25TXFrameBuffer[AX25TXbufferIndex].setPID(0xF0);
    AX25TXFrameBuffer[AX25TXbufferIndex].setPacket(data, size);
    AX25TXFrameBuffer[AX25TXbufferIndex].calculateFCS();

    //txSize = AX25TXFrameBuffer[0].getSize();
    //txRFMessageBuffer = AX25TXFrameBuffer[AX25TXbufferIndex].getBytes();
    // Print RF Packet for Debug
    serial.println("============================");
    serial.print("PACKETIndex : ");
    serial.print(AX25TXbufferIndex, DEC);
    serial.print("  == AVAILABLE : ");
    serial.print(AX25TXframesInBuffer+1, DEC);
    serial.println();
    for(int i = 0; i < AX25TXFrameBuffer[AX25TXbufferIndex].getSize(); i++){
        serial.print(this->AX25TXFrameBuffer[AX25TXbufferIndex].getBytes()[i], HEX);
        serial.print("|");
    }
    serial.println("");
    serial.println("============================");
    AX25TXbufferIndex++;
    AX25TXframesInBuffer++;

    txReady = true;
    return true;
}

void COMMRadio::sendPacketAX25(){
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
    serial.print(this->AX25RXframesInBuffer, DEC);
    serial.println();
    serial.println(" ============ ");
    for(int k = 0; k < AX25RXframesInBuffer; k++){
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
