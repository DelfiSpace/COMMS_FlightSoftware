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

    // Receive bits out of buffer:
    if(advancedMode){
        for(int k = 0; k < 10; k++){
            APSync.rxBit();
            if(APSync.bytesInQue <= 0){
                break;
            }
        }
    }else{
        for(int k = 0; k < 10; k++){
            AX25Sync.rxBit();
            if(AX25Sync.bytesInQue <= 0){
                break;
            }
        }
    }

    // If codeblock ready, decode
    for(int k = 0; k < RX_FRAME_BUFFER; k++){
        int buf_index = mod((rxCLTUBufferIndex + 1 + k), RX_FRAME_BUFFER);
        if(rxCLTUBuffer[buf_index].isLocked == false && rxCLTUBuffer[buf_index].isReady == true && rxCLTUBuffer[buf_index].isCoded == true){
            bool decoded = false;
            for(int j = 0; j < 20; j++){
                if(LDPCDecoder::iterateBitflip(rxCLTUBuffer[buf_index].data)){
                    rxCLTUBuffer[buf_index].packetSize = 32;
                    rxCLTUBuffer[buf_index].isCoded = false;
                    serial.print("DECODED PACKET!  :");
                    serial.print(j,DEC);
                    serial.println();
                    decoded = true;
                    break;
                }
            }
            if(!decoded){
                serial.println("BROKEN PACKET!");
                rxCLTUBuffer[buf_index].isCoded = false;
                rxCLTUBuffer[buf_index].isNotRecoverable = true;
                break;
            }
        }
    }


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
            }else if(txCLTUInBuffer > 0){
                if(encoder.bitsInBuffer == 0){
                    //tx is ready for next bit
                        uint8_t inBit = (txCLTUBuffer[mod((txCLTUBufferIndex - txCLTUInBuffer), TX_FRAME_BUFFER)].getBytes()[txIndex] >> txBitIndex) & 0x01;
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
                if(txIndex >= txCLTUBuffer[mod((txCLTUBufferIndex - txCLTUInBuffer), TX_FRAME_BUFFER)].getSize()){
                    txIndex = 0;
                    txCLTUInBuffer = txCLTUInBuffer - 1;
                    if(txCLTUInBuffer != 0){
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

        if(txFlagInsert <= 0 && txCLTUInBuffer <= 0){
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
        if(advancedMode){
            APSync.queByte(data);
        }else{
            AX25Sync.queByte(data);
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
        MAP_Timer32_setCount(TIMER32_1_BASE, 2*48000000);
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
    if(size < MAX_PACKET_SIZE && this->txCLTUInBuffer < TX_FRAME_BUFFER){
        TXDestination[6] = 0xE0;//(('A' & 0x0F) << 1) | 0xE0;
        TXSource[6] = 0x61;//(('B' & 0x0F) << 1) | 0x61;
        AX25Frame::setAdress(txCLTUBuffer[txCLTUBufferIndex], TXDestination, TXSource);
        AX25Frame::setControl(txCLTUBuffer[txCLTUBufferIndex], false);
        AX25Frame::setPID(txCLTUBuffer[txCLTUBufferIndex], 0xF0);
        AX25Frame::setPacket(txCLTUBuffer[txCLTUBufferIndex], data, size);
        AX25Frame::calculateFCS(txCLTUBuffer[txCLTUBufferIndex]);

        //txSize = txCLTUBuffer[0].getSize();
        //txRFMessageBuffer = txCLTUBuffer[txCLTUBufferIndex].getBytes();
        // Print RF Packet for Debug
        serial.println("============================");
        serial.print("PACKETIndex : ");
        serial.print(txCLTUBufferIndex, DEC);
        serial.print("  == AVAILABLE : ");
        serial.print(txCLTUInBuffer+1, DEC);
        serial.println();
        for(int i = 0; i < txCLTUBuffer[txCLTUBufferIndex].getSize(); i++){
            serial.print(this->txCLTUBuffer[txCLTUBufferIndex].getBytes()[i], HEX);
            serial.print("|");
        }
        serial.println("");
        serial.println("============================");
        txCLTUBufferIndex = mod(txCLTUBufferIndex + 1, TX_FRAME_BUFFER);
        txCLTUInBuffer++;

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
    serial.println("Printing Buffer:");
    serial.println(" ============ ");
    for(int k = 0; k < RX_FRAME_BUFFER; k++){
        int buf_index = mod((rxCLTUBufferIndex + 1 + k), RX_FRAME_BUFFER);
        if(rxCLTUBuffer[buf_index].isLocked == false && rxCLTUBuffer[buf_index].isReady == true){
            serial.println("*******");
            int packet_size = rxCLTUBuffer[buf_index].packetSize;
            if (packet_size > 64){
                serial.print("||WARNING||");
                packet_size = 1;
            }
            serial.print("Index of Packet:  ");
            serial.print(buf_index, DEC);
            serial.print("  |   Packet Size:  ");
            serial.print(packet_size, DEC);
            serial.println();
            serial.println("*******");
            uint8_t* frameData = this->rxCLTUBuffer[buf_index].data;
            for(int j = 0; j < packet_size; j++){
                serial.print(frameData[j], HEX);
                serial.print("|");
            }
            serial.println();
            serial.println(" ============ ");
        }
    }
    //TODO: OPMODE
    //serial.print(rxReady);
};

void COMMRadio::toggleMode(){
    serial.println("Toggle ProtocolMode ");
    advancedMode = !advancedMode;
};


uint8_t COMMRadio::getNumberOfRXFrames(){
    int count = 0;
    for(int k = 0; k < RX_FRAME_BUFFER; k++){
        int buf_index = mod((rxCLTUBufferIndex + 1 + k), RX_FRAME_BUFFER);
        if(rxCLTUBuffer[buf_index].isLocked == false && rxCLTUBuffer[buf_index].isReady == true){
            count++;
        }
    }
    return count;
};

uint8_t COMMRadio::getSizeOfRXFrame(){
    for(int k = 0; k < RX_FRAME_BUFFER; k++){
        int buf_index = mod((rxCLTUBufferIndex + 1 + k), RX_FRAME_BUFFER);
        if(rxCLTUBuffer[buf_index].isLocked == false && rxCLTUBuffer[buf_index].isReady == true){
            return rxCLTUBuffer[buf_index].packetSize;
        }
    }
    return 0;
}

uint8_t* COMMRadio::getRXFrame(){
    for(int k = 0; k < RX_FRAME_BUFFER; k++){
        int buf_index = mod((rxCLTUBufferIndex + 1 + k), RX_FRAME_BUFFER);
        if(rxCLTUBuffer[buf_index].isLocked == false && rxCLTUBuffer[buf_index].isReady == true){
            return rxCLTUBuffer[buf_index].data;
        }
    }
    return 0;
}

void COMMRadio::popFrame(){
    for(int k = 0; k < RX_FRAME_BUFFER; k++){
        int buf_index = mod((rxCLTUBufferIndex + 1 + k), RX_FRAME_BUFFER);
        if(rxCLTUBuffer[buf_index].isLocked == false && rxCLTUBuffer[buf_index].isReady == true){
            rxCLTUBuffer[buf_index].isReady = false;
            break;
        }
    }
}
