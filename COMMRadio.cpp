#include "COMMRadio.h"

extern DSerial serial;

#ifndef MODFUNC
#define MODFUNC
int mod(int a, int b)
{ return (a%b+b)%b; }
#endif

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
    radioStub->txTimeout = false;
    radioStub->sendPacketAX25();
}
void taskWrapper(){
    radioStub->runTask();
}

COMMRadio::COMMRadio(DSPI &bitModeSPI_tx, DSPI &bitModeSPI_rx, DSPI &packetModeSPI, SX1276 &txRad, SX1276 &rxRad):
       Task(taskWrapper), bitSPI_tx(&bitModeSPI_tx), bitSPI_rx(&bitModeSPI_rx), packetSPI(&packetModeSPI), txRadio(&txRad), rxRadio(&rxRad)
{
    radioStub = this;
};

void COMMRadio::runTask(){
    for(int k = 0; k < 10; k++){
        for(int i = 0; i < 8; i++){
            //serial.print("Y");
            AX25Sync.rxBit();
        }
        if(AX25Sync.bytesInQue <= 0){
            break;
        }
    }
//    if(this->LDPCdecodeEnabled){
//        for(int k = 0; k < AX25RXframesInBuffer; k++){
//            int tmp = mod((AX25RXbufferIndex - AX25RXframesInBuffer + k), AX25_RX_FRAME_BUFFER);
//            if(this->AX25RXFrameBuffer[tmp].getPacketSize() == 64){
//                for(int k = 0; k < 10; k++){
//                    if(this->LDPCdecoder.iterateBitflip(&this->AX25RXFrameBuffer[tmp].FrameBytes[16])){
//                        serial.println("SUCCES!");
//                        for(int p = 0; p < 32; p++){
//                            serial.print(this->AX25RXFrameBuffer[tmp].FrameBytes[16+p], HEX);
//                            serial.print("|");
//                        }
//                        this->AX25RXFrameBuffer[tmp].FrameSize = this->AX25RXFrameBuffer[tmp].FrameSize-1;
//                        break;
//                    }
//                }
//            }
//        }
//    }
}

uint8_t COMMRadio::onTransmit(){
    //NOTE, SPI Bus is configured to MSB_first, meaning that the bit transmitted first should be the MSB.

    uint8_t outputByte = 0;
    if(txPacketReady){
        for(int i = 0; i < 8; i++){// Send 8bits per call
            //Start Sending
           if( txFlagInsert > 0) {
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
                    txFlagInsert--;
                }
            }else if(AX25TXframesInBuffer > 0){
                if(encoder.bitsInBuffer == 0){
                    //tx is ready for next bit
                        uint8_t inBit = (AX25TXFrameBuffer[mod((AX25TXbufferIndex - AX25TXframesInBuffer), AX25_TX_FRAME_BUFFER)].getBytes()[txIndex] >> txBitIndex) & 0x01;
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
                if(txIndex >= AX25TXFrameBuffer[mod((AX25TXbufferIndex - AX25TXframesInBuffer), AX25_TX_FRAME_BUFFER)].getSize()){
                    txIndex = 0;
                    AX25TXframesInBuffer = AX25TXframesInBuffer - 1;
                    if(AX25TXframesInBuffer != 0){
                        txFlagInsert += 2;
                    }else{
                        txFlagInsert += DOWNRAMP_BYTES;
                    }
                }
            }else {
                //no more data available, so stuff byte with zeros for transmission
                outputByte = outputByte | (encoder.txBit(0, false) << (7-i));
            }

            //serial.print(txBitIndex, HEX);
        }

        if(txFlagInsert <= 0 && AX25TXframesInBuffer <= 0){
            //end of transmission
            txRadio->setIdleMode(false);
            txPacketReady = false;
        }
        //serial.print(outputByte, HEX);
        return outputByte;
    }else{
        txRadio->setIdleMode(false);
        txPacketReady = false;
        return 0x00;
    }
};

void COMMRadio::onReceive(uint8_t data)
{
    this->notify();
    //Note: SPI Bus is configured MSB First, meaning that from the received Byte, the MSB was the first received Bit
    if(rxReady){
        //for(int i = 0; i < 8; i++){
           // uint8_t inBit = encoder.NRZIdecodeBit((data >> (7-i))& 0x01);
          //  inBit = encoder.descrambleBit(inBit);
            AX25Sync.queByte(data);
        //}
    }
};

void COMMRadio::init(){
    serial.println("COMMS booting...");
    this->initTX();
    this->initRX();
    MAP_Timer32_initModule(TIMER32_1_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT,
                    TIMER32_PERIODIC_MODE);
    this->rxReady = true;
    //serial.print(GIT_HASH);
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

    if(!txTimeout){
        serial.println("Setting Timer");

        MAP_Timer32_registerInterrupt(TIMER32_1_INTERRUPT, &sendPacketWrapper);
        MAP_Timer32_setCount(TIMER32_1_BASE, 48000000);
        MAP_Timer32_startTimer(TIMER32_1_BASE, true);
        txTimeout = true;
    }

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
    //return false is unsuccesful
    txReady = false;
    if(size < MAX_PACKET_SIZE && this->AX25TXframesInBuffer < AX25_TX_FRAME_BUFFER){
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
        AX25TXbufferIndex = mod(AX25TXbufferIndex + 1, AX25_TX_FRAME_BUFFER);
        AX25TXframesInBuffer++;

        txReady = true;
        return true;
    }else{
        txReady = true;
        return false;
    }
}

void COMMRadio::sendPacketAX25(){
    if(!txPacketReady){
        txFlagInsert += UPRAMP_BYTES;
        txPacketReady = true;
        txRadio->setIdleMode(true);
    }
    // else the radio is already on.

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
        serial.print("*******");
        int tmp = mod((AX25RXbufferIndex - AX25RXframesInBuffer + k), AX25_RX_FRAME_BUFFER);
        serial.print(tmp, DEC);
        serial.println("*******");
        uint8_t* frameData = this->AX25RXFrameBuffer[tmp].getBytes();
        for(int j = 0; j < this->AX25RXFrameBuffer[tmp].getSize(); j++){
            serial.print(frameData[j], HEX);
            serial.print("|");
        }
        serial.println();
        serial.println(" ============ ");
    }
    //TODO: OPMODE
    //serial.print(rxReady);
};

uint8_t COMMRadio::getNumberOfRXFrames(){
    return this->AX25RXframesInBuffer;
};

uint8_t COMMRadio::getSizeOfRXFrame(){
    int tmp = mod((AX25RXbufferIndex - AX25RXframesInBuffer), AX25_RX_FRAME_BUFFER);
    return this->AX25RXFrameBuffer[tmp].getSize();
}

uint8_t* COMMRadio::getRXFrame(){
    int tmp = mod((AX25RXbufferIndex - AX25RXframesInBuffer), AX25_RX_FRAME_BUFFER);
        return this->AX25RXFrameBuffer[tmp].getBytes();
}

void COMMRadio::popFrame(){
    AX25RXframesInBuffer--;
}
