#include "COMMRadio.h"

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
    Console::log("SEND PACKET !");
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

bool COMMRadio::notified(){
    return (AX25Sync.bytesInQue > 0);  //return true if bytes in Queue.
}


void COMMRadio::runTask(){
    // Process 10 bytes out of buffer:
    for(int k = 0; k < 80; k++){
        if(AX25Sync.rxBit())
        {
            if(this->rxPacketBuffer[rxPacketBufferIndex-1].getBytes()[17] == 8) //packet to be processed instead of stored
            {
                //process and put pointer one back
                rxPacketBufferIndex--;
                Console::log("Packet Processed!  (Size: %d)", rxPacketBuffer[rxPacketBufferIndex].getSize());
            }
            else
            {
                //keep pointer one forward and increase packetsinBuffer
                rxPacketsInBuffer++;
                if(rxPacketsInBuffer > RX_MAX_FRAMES){
                    rxPacketsInBuffer = RX_MAX_FRAMES;
                }
                Console::log("Packet Buffered! (Size: %d) - Packets In Buffer: %d",rxPacketBuffer[rxPacketBufferIndex-1].getSize(),  rxPacketsInBuffer);
            }
        }
        if(AX25Sync.bytesInQue <= 0){// && APSync.bytesInQue <= 0 ){
            break;
        }
    }

   //process Received Packets

}

uint8_t COMMRadio::onTransmit(){
    //NOTE, SPI Bus is configured to MSB_first, meaning that the bit transmitted first should be the MSB.
//    Console::log("onTransmit!");
    uint8_t outputByte = 0;

    if(txEnabled)
    {
       for(int i = 0; i < 8; i++) // Send 8bits per call
       {
           if(!txFlagQue && !txPacketsInBuffer) //if no idle flags are qued, and no data is ready.
           {
               if(txIdleMode)
               {
                   txFlagQue++; //stay Idle by transmitting a flag
               }
               else
               {
                   txRadio->setIdleMode(false);
                   txEnabled = false;
                   return 0x00;
               }
           }


           if(txFlagQue) // if flags are qued, transmit
           {
               if(encoder.StuffBitsInBuffer)
               {  //send the Buffered Bit
                   outputByte = outputByte | (encoder.txBit(0, false) << (7-i));
               }
               else
               {   //send part flagbit (7E)
                   uint8_t inBit = (0x7E >> txBitIndex) & 0x01;
                   outputByte = outputByte | (encoder.txBit( inBit , false) << (7-i));
                   txBitIndex++;
               }

               //check if txBitIndex is high enough to roll over to next byte
               if(txBitIndex>=8){
                   txBitIndex = 0;
                   txFlagQue--;
               }

           }
           else if(txPacketsInBuffer) //no flags, but tx packet buffered
           {
               if(encoder.StuffBitsInBuffer)
               { // send stuffing bit first
                   outputByte = outputByte | (encoder.txBit(0, true) << (7-i));
               }
               else
               { //send next bit of packet
                   uint8_t inBit = (txPacketBuffer[mod((txPacketBufferIndex - txPacketsInBuffer), TX_MAX_FRAMES)].getBytes()[txIndex] >> txBitIndex) & 0x01;
                   outputByte = outputByte | (encoder.txBit( inBit , true) << (7-i));
                   txBitIndex++;
               }

               //check if bitindex is high enough to roll over next byte
               if(txBitIndex >= 8)
               {
                  txIndex++;
                  txBitIndex = 0;
               }

               //check if we are out of bytes in packet
               if(txIndex >= (txPacketBuffer[mod((txPacketBufferIndex - txPacketsInBuffer), TX_MAX_FRAMES)]).getSize())
               {
                   //roll over to next packet
                  txIndex = 0;
                  txPacketsInBuffer--;
                  if(txPacketsInBuffer != 0)
                  {
                      txFlagQue += 3; //if next packet available add, 3 flags to stack
                  }
                  else
                  {
                      txFlagQue += DOWNRAMP_BYTES;
                  }
               }
           }
           else // no flags are qued, and no data available, but radio is enabled, just idle the flag
           {
               //shouldnt happen
               Console::log("onTransmit: Shouldnt happen - Nothing is Queued");
           }

       }
       return outputByte;
    }
    else
    {
        //TX shouldnt be asking for this function, so turn off and pass 0
        Console::log("onTransmit: Shouldnt Happen - Transmitter should be off");
        txRadio->setIdleMode(false);
        return 0x00;
    }
};

void COMMRadio::onReceive(uint8_t data)
{
    //Note: SPI Bus is configured MSB First, meaning that from the received Byte, the MSB was the first received Bit
    AX25Sync.queByte(data);
};

void COMMRadio::init(){
    Console::log("Radio Object Starting...");
    this->initTX();
    this->initRX();
    MAP_Timer32_initModule(TIMER32_1_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT,
                    TIMER32_PERIODIC_MODE);
};

void COMMRadio::initTX(){
    // Initialise TX values
    // Modem set to FSK, deviation set to 1/2 datarate, gaussian filter enabled
    txRadio->init();

    if(txRadio->ping()){

        txConfig.modem = MODEM_FSK;
        txConfig.filtertype = BT_0_5;
        txConfig.bandwidth = 15000;
        txConfig.fdev = 4800;
        txConfig.datarate = 9600;

        txRadio->setFrequency(435000000);

        txRadio->enableBitMode(*bitSPI_tx, 0, onTransmitWrapper);
        txRadio->setTxConfig(&txConfig);

        Console::log("TX Radio Settings Set");
    }
    else{
        Console::log("TX Radio not Found");
    }
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

        rxRadio->setFrequency(145000000);

        rxRadio->RxChainCalibration();
        rxRadio->enableBitMode(*bitSPI_rx, onReceiveWrapper, 0);

        rxRadio->setRxConfig(&rxConfig);
        rxRadio->startReceiver();

        Console::log("RX Radio Settings Set");
    }else{
        Console::log("RX Radio not Found");
    }
};

bool COMMRadio::transmitData(uint8_t data[], uint8_t size){
    this->quePacketAX25(data, size);
//
    if(!txTimeout){
        Console::log("Setting Timer");

        MAP_Timer32_registerInterrupt(TIMER32_1_INTERRUPT, &sendPacketWrapper);
        MAP_Timer32_setCount(TIMER32_1_BASE, 48000000/2);
        MAP_Timer32_startTimer(TIMER32_1_BASE, true);
        txTimeout = true;
    }
//    radioStub->sendPacketAX25();

    return true;

};


bool COMMRadio::quePacketAX25(uint8_t data[], uint8_t size){
    //return false is unsuccesful
    if(size < MAX_PACKET_SIZE && this->txPacketsInBuffer < TX_MAX_FRAMES){
        TXDestination[6] = 0xE0;//(('A' & 0x0F) << 1) | 0xE0;
        TXSource[6] = 0x61;//(('B' & 0x0F) << 1) | 0x61;
        AX25Frame::setAdress(txPacketBuffer[txPacketBufferIndex], TXDestination, TXSource);
        AX25Frame::setControl(txPacketBuffer[txPacketBufferIndex], false);
        AX25Frame::setPID(txPacketBuffer[txPacketBufferIndex], 0xF0);
        AX25Frame::setPacket(txPacketBuffer[txPacketBufferIndex], data, size);
        AX25Frame::calculateFCS(txPacketBuffer[txPacketBufferIndex]);

        //txSize = txCLTUBuffer[0].getSize();
        //txRFMessageBuffer = txCLTUBuffer[txCLTUBufferIndex].getBytes();
        // Print RF Packet for Debug
//        serial.println("============================");
//        serial.print("PACKETIndex : ");
//        serial.print(txCLTUBufferIndex, DEC);
//        serial.print("  == AVAILABLE : ");
//        serial.print(txCLTUInBuffer+1, DEC);
//        serial.println();
//        for(int i = 0; i < txCLTUBuffer[txCLTUBufferIndex].getSize(); i++){
//            serial.print(this->txCLTUBuffer[txCLTUBufferIndex].getBytes()[i], HEX);
//            serial.print("|");
//        }
//        serial.println("");
//        serial.println("============================");
        txPacketBufferIndex = mod(txPacketBufferIndex + 1, TX_MAX_FRAMES);
        txPacketsInBuffer++;

        return true;
    }else{
        return false;
    }
}

void COMMRadio::sendPacketAX25(){
    if(!txPacketReady){
        txFlagQue += UPRAMP_BYTES;
        txEnabled = true;
        txRadio->setIdleMode(true);
    }
    // else the radio is already on.

    //rxPrint = true;
    //rxReady = false;
}

void COMMRadio::toggleReceivePrint(){
    //serial.print("Toggle RX Print: ");
    //serial.println(rxReady);

//    Console::log("");
//    Console::log(" ============ ");
//    Console::log("Printing Buffer:");
//    Console::log(" ============ ");
//    for(int k = 0; k < RX_MAX_FRAMES; k++){
//        int buf_index = mod((rxCLTUBufferIndex + 1 + k), RX_MAX_FRAMES);
//        if(rxCLTUBuffer[buf_index].isLocked == false && rxCLTUBuffer[buf_index].isReady == true){
//            Console::log("*******");
//            int packet_size = rxCLTUBuffer[buf_index].packetSize;
//
//            Console::log("Index of Packet: %d |  Packet Size:  %d", buf_index, packet_size);
//            Console::log("*******");
//            uint8_t* frameData = this->rxCLTUBuffer[buf_index].data;
//            for(int j = 0; j < packet_size; j++){
//                Console::log("%x ",frameData[j]);
//            }
//            Console::log("");
//            Console::log(" ============ ");
//        }
//    }
    //TODO: OPMODE
    //serial.print(rxReady);
};

void COMMRadio::toggleMode(){
    Console::log("Toggle ProtocolMode ");
//    advancedMode = !advancedMode;
};


uint8_t COMMRadio::getNumberOfRXFrames(){
    int count = 0;
//    for(int k = 0; k < RX_MAX_FRAMES; k++){
//        int buf_index = mod((rxCLTUBufferIndex + 1 + k), RX_MAX_FRAMES);
//        if(rxCLTUBuffer[buf_index].isLocked == false && rxCLTUBuffer[buf_index].isReady == true){
//            count++;
//        }
//    }
    return count;
};

uint8_t COMMRadio::getSizeOfRXFrame(){
//    for(int k = 0; k < RX_MAX_FRAMES; k++){
//        int buf_index = mod((rxCLTUBufferIndex + 1 + k), RX_MAX_FRAMES);
//        if(rxCLTUBuffer[buf_index].isLocked == false && rxCLTUBuffer[buf_index].isReady == true){
//            return rxCLTUBuffer[buf_index].packetSize;
//        }
//    }
    return 0;
}

uint8_t* COMMRadio::getRXFrame(){
//    for(int k = 0; k < RX_MAX_FRAMES; k++){
//        int buf_index = mod((rxCLTUBufferIndex + 1 + k), RX_MAX_FRAMES);
//        if(rxCLTUBuffer[buf_index].isLocked == false && rxCLTUBuffer[buf_index].isReady == true){
//            return rxCLTUBuffer[buf_index].data;
//        }
//    }
    return 0;
}

void COMMRadio::popFrame(){
//    for(int k = 0; k < RX_MAX_FRAMES; k++){
//        int buf_index = mod((rxCLTUBufferIndex + 1 + k), RX_MAX_FRAMES);
//        if(rxCLTUBuffer[buf_index].isLocked == false && rxCLTUBuffer[buf_index].isReady == true){
//            rxCLTUBuffer[buf_index].isReady = false;
//            break;
//        }
//    }
}
