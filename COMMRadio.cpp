#include "COMMRadio.h"

#ifndef MODFUNC
#define MODFUNC
int mod(int a, int b)
{ return a<0 ? (a%b+b)%b : a%b; }
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
    MAP_Timer32_clearInterruptFlag(TIMER32_1_BASE);
    radioStub->txTimeout = false;
    radioStub->doEnableFlag = true;
}
void taskWrapper(){
    radioStub->runTask();
}

COMMRadio::COMMRadio(DSPI &bitModeSPI_tx, DSPI &bitModeSPI_rx, DSPI &packetModeSPI, SX1276 &txRad, SX1276 &rxRad):
       Task(taskWrapper), bitSPI_tx(&bitModeSPI_tx), bitSPI_rx(&bitModeSPI_rx), packetSPI(&packetModeSPI), txRadio(&txRad), rxRadio(&rxRad)
{
    radioStub = this;
};

void COMMRadio::setcmdHandler(InternalCommandHandler<PQ9Frame,PQ9Message> &cmdhand)
{
    cmdHandler = &cmdhand;
}

void COMMRadio::setbusMaster(BusMaster<PQ9Frame, PQ9Message> &busmstr)
{
    busOverride = &busmstr;
}

bool COMMRadio::notified(){
    return (AX25Sync.bytesInQue() > 1) || busOverride->waitingForReply || overridePacketsInBuffer;  //return true if bytes in Queue.
}


void COMMRadio::runTask(){
    //check for enable flag:
    if(doEnableFlag){
        this->enableTransmit();
        this->doEnableFlag = false;
    }

    // Process busOverride command Handler:
    if(busOverride->checkPending()){
        //Handle Receive and Pop Override Buffer Frame
        if(busOverride->cmdReceivedFlag)
        {
            uint8_t overrideReply[256];
            overrideReply[0] = overridePacketBuffer[mod(overridePacketBufferIndex - overridePacketsInBuffer, OVERRIDE_MAX_FRAMES)].getBytes()[0];
            uint8_t *repptr = this->busOverride->rxFrame.getFrame();
            for(int j = 0; j < repptr[1]+3; j++){
                overrideReply[1+j] = repptr[j];
            }
//            Console::log("rep: %d %d %d %d %d %d %d",repptr[0],repptr[1],repptr[2],repptr[3],repptr[4],repptr[5],repptr[6]);
            this->quePacketAX25(overrideReply, busOverride->rxFrame.getFrameSize()+3+1);
        }
        else
        {
            Console::log("Failure!");
            uint8_t overrideReply[256];
            overrideReply[0] = overridePacketBuffer[mod(overridePacketBufferIndex - overridePacketsInBuffer, OVERRIDE_MAX_FRAMES)].getBytes()[0];
            uint8_t *repptr = this->busOverride->rxFrame.getFrame();
            overrideReply[2] = 0; //destination
            overrideReply[3] = 2; //size
            overrideReply[4] = 0; //Source
            overrideReply[5] = 0; //service
            overrideReply[6] = 2; //Reply
            overrideReply[7] = 1; //NO_RESPONSE

//            Console::log("rep: %d %d %d %d %d %d %d",repptr[0],repptr[1],repptr[2],repptr[3],repptr[4],repptr[5],repptr[6]);
            this->quePacketAX25(overrideReply, 8);
        }

        overridePacketsInBuffer--;
//        Console::log("in Queue left: %d", overridePacketsInBuffer);

    }else if(overridePacketsInBuffer && !busOverride->waitingForReply && AX25Sync.bytesInQue() < BYTE_QUE_SIZE/2){
        //More frames in Buffer, so execute!

        //Oldest Frame to take:
        uint8_t* targetPacket = overridePacketBuffer[mod(overridePacketBufferIndex - overridePacketsInBuffer, OVERRIDE_MAX_FRAMES)].getBytes();
//        Console::log("override Buffer IDNUMBER: %d | in Queue: %d", targetPacket[0], overridePacketsInBuffer);
//        Console::log("req: %d %d %d %d",targetPacket[0],targetPacket[1],targetPacket[2],targetPacket[3]);

        //detect wether COMMS is destination in order to issue an internal command:
        if(targetPacket[1] == Address::COMMS)
        {
            //internal command
            PQ9Frame internalCommandOverride;
            internalCommandOverride.setDestination(Address::COMMS);
            internalCommandOverride.setSource(Address::COMMS);
            internalCommandOverride.setPayloadSize(targetPacket[2]);
            for (int i = 0; i < internalCommandOverride.getPayloadSize(); i++)
            {
                internalCommandOverride.getPayload()[i] = targetPacket[4+i];
            }
//            Console::log("Internal command Detected!");
            cmdHandler->received(internalCommandOverride);
            cmdHandler->run();
            PQ9Frame* internalResponse = cmdHandler->getTxBuffer();

            //hijack the busOverride to handle the reponse for us
            internalResponse->copy(busOverride->rxFrame);
            busOverride->waitingForReply = true;
            busOverride->cmdReceivedFlag = true;
        }
        else
        {
            //override bus
            busOverride->RequestReplyNoBlock(targetPacket[1], targetPacket[2]-2, &targetPacket[3], targetPacket[4], targetPacket[5], 100);
        }
    }


    // Process bits out of buffer:
    if(AX25Sync.rxBit())
    {
//        Console::log("PACKET!!");
        if(AX25Sync.rcvdFrame.checkValidCRC())
        {
            lastRSSI = getRXRSSI();
    //            if(lastRSSI < 0)
    //            {
    //                Console::log("Received Command! Current RSSI: -%d dBm", -lastRSSI);
    //            }
    //            else
    //            {
    //                Console::log("Received Command! Current RSSI: %d dBm", lastRSSI);
    //            }

            lastFreqError = rxRadio->getFrequencyError();
    //            if(lastFreqError < 0)
    //            {
    //                Console::log("Received Command! Freq Error: -%d Hz", -lastFreqError);
    //            }
    //            else
    //            {
    //                Console::log("Received Command! Freq Error: %d Hz", lastFreqError);
    //            }
    //            Console::log("%d", this->rxPacketBuffer[mod(rxPacketBufferIndex - 1, RX_MAX_FRAMES)].getBytes()[16]);
            switch(AX25Sync.rcvdFrame.getBytes()[16])
            {
            case 0xAA:  //RESET COMMAND
                //process and put pointer one back
                Console::log("RESET COMMAND! (Size: %d)", AX25Sync.rcvdFrame.getSize()-18);
                MAP_GPIO_setOutputHighOnPin(COMMS_RESET_PORT, COMMS_RESET_PIN);
                break;
            case 0x01:  //Internal Command
                //process command in internal commandHandler
                if(cmdHandler){
                    uint8_t internalCommandNumber = AX25Sync.rcvdFrame.getPQID();
                    PQ9Frame internalCommand;
                    internalCommand.setDestination(4);
                    internalCommand.setSource(8);
                    internalCommand.setPayloadSize(AX25Sync.rcvdFrame.getPQPayloadSize());
                    for (int i = 0; i < internalCommand.getPayloadSize(); i++)
                    {
                        internalCommand.getPayload()[i] = AX25Sync.rcvdFrame.getPQPayload()[i];
                    }
                    Console::log("INTERNAL COMMAND! (Size: %d, ID: %d) CMD = DEST:%d SRC:%d SIZE:%d", AX25Sync.rcvdFrame.getSize()-18,internalCommandNumber,internalCommand.getDestination(),internalCommand.getSource(),internalCommand.getPayloadSize());
                    cmdHandler->received(internalCommand);
                    cmdHandler->run();
                    PQ9Frame* internalResponse = cmdHandler->getTxBuffer();
                    int packetSize = internalResponse->getPayloadSize()+1;
    //                    Console::log("pSize: %d", packetSize);
                    uint8_t responsePacket[256];
                    responsePacket[0] = internalCommandNumber;
                    for(int j = 0; j < internalResponse->getPayloadSize(); j++){
                        responsePacket[1+j] = internalResponse->getPayload()[j];
                    }
                    this->quePacketAX25(responsePacket, packetSize);
                }
                break;
            case 0x02: //Bus override command
                int overridePacketSize = AX25Sync.rcvdFrame.getPQPayloadSize() + 1; //include ID

                //keep pointer one forward and increase packetsinBuffer
                overridePacketsInBuffer++;
                if(overridePacketsInBuffer > OVERRIDE_MAX_FRAMES){
                    overridePacketsInBuffer = OVERRIDE_MAX_FRAMES;
                }

                Console::log("BUSOVERRIDE COMMAND! (Size: %d)", AX25Sync.rcvdFrame.getSize());

                overridePacketBuffer[overridePacketBufferIndex].packetSize = overridePacketSize;
                for(int j = 0; j < overridePacketSize; j++){
                    overridePacketBuffer[overridePacketBufferIndex].getBytes()[j] = AX25Sync.rcvdFrame.getPQPayload()[j-1];
    //                    Console::log("%d", rxBuffer[j]);
                }
                overridePacketBufferIndex = (overridePacketBufferIndex + 1) % OVERRIDE_MAX_FRAMES;
                break;
            case 0x03: //OBC buffered command
                //Buffer command (dont move pointer back)
                int packetSize = AX25Sync.rcvdFrame.getPQPayloadSize() + 1; //include ID

                //keep pointer one forward and increase packetsinBuffer
                rxPacketsInBuffer++;
                if(rxPacketsInBuffer > RX_MAX_FRAMES){
                    rxPacketsInBuffer = RX_MAX_FRAMES;
                }

                Console::log("BUFFER COMMAND! (Size: %d) - Packets In Buffer: %d - cmdSize: %d",AX25Sync.rcvdFrame.getSize(),  rxPacketsInBuffer, packetSize);

                rxPacketBuffer[rxPacketBufferIndex].packetSize = packetSize;
                for(int j = 0; j < packetSize; j++){
                    rxPacketBuffer[rxPacketBufferIndex].getBytes()[j] = AX25Sync.rcvdFrame.getPQPayload()[j-1];
    //                    Console::log("%d", rxBuffer[j]);
                }
                rxPacketBufferIndex = (rxPacketBufferIndex + 1) % RX_MAX_FRAMES;
                break;
            }
        }
    }
}

uint8_t COMMRadio::onTransmit(){
    //NOTE, SPI Bus is configured to MSB_first, meaning that the bit transmitted first should be the MSB.
//    Console::log("onTransmit!");
    uint8_t outputByte = 0;

    if(txEnabled)
    {
       for(int i = 0; i < 8; i++) // Send 8bits per call
       {
           if(!txFlagQue && !txPacketsInBuffer()) //if no idle flags are qued, and no data is ready.
           {
               if(txIdleMode)
               {
                   txFlagQue++; //stay Idle by transmitting a flag
//                   Console::log("TX: stuffing");
               }
               else
               {
                   //disable Transmit
                   disableTransmit();
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
                   txIndex = 0;
                   txFlagQue--;
//                   Console::log("TX: FLAG!");
               }

           }
           else if(txPacketsInBuffer() > 0) //no flags, but tx packet buffered
           {

               if(encoder.StuffBitsInBuffer)
               { // send stuffing bit first
                   outputByte = outputByte | (encoder.txBit(0, true) << (7-i));
               }
               else
               { //send next bit of packet
                   uint8_t inBit = (txPacketBuffer[txPacketBufferReadIndex].getBytes()[txIndex] >> txBitIndex) & 0x01;
                   outputByte = outputByte | (encoder.txBit( inBit , true) << (7-i));
                   txBitIndex++;
               }

               //check if bitindex is high enough to roll over next byte
               if(txBitIndex >= 8)
               {
                   txIndex++;
                   txBitIndex = 0;
//                   Console::log("TX: Byte: %d, Packet: %d", txIndex, txPacketBufferIndex);
                   //check if we are out of bytes in packet
                   if(txIndex >= (txPacketBuffer[txPacketBufferReadIndex]).getSize())
                   {
                       //roll over to next packet
                      txIndex = 0;
                      txPacketBufferReadIndex = (txPacketBufferReadIndex + 1) % TX_MAX_FRAMES;
//                      Console::log("R%d", txPacketBufferReadIndex);
                      if(txPacketsInBuffer() > 0)
                      {
                          txFlagQue += 2; //if next packet available add, 2 flags to stack
                      }
                      else
                      {
                          txFlagQue += DOWNRAMP_BYTES;
                      }
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
        disableTransmit();
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
    Console::log("Drive Reset Pin Low (9.1)");

    TXDestination[0] = 0x8E;//('G' & 0x0F) << 1;
    TXDestination[1] = 0xA4;//('R' & 0x0F) << 1;
    TXDestination[2] = 0x9E;//('O' & 0x0F) << 1;
    TXDestination[3] = 0xAA;//('U' & 0x0F) << 1;
    TXDestination[4] = 0x9C;//('N' & 0x0F) << 1;
    TXDestination[5] = 0x88;//('D' & 0x0F) << 1;
    TXDestination[6] = 0xE0;//(('A' & 0x0F) << 1) | 0xE0;

    TXSource[0]      = 0x88;//('D' & 0x0F) << 1;
    TXSource[1]      = 0x98;//('L' & 0x0F) << 1;
    TXSource[2]      = 0x8C;//('F' & 0x0F) << 1;
    TXSource[3]      = 0x92;//('I' & 0x0F) << 1;
    TXSource[4]      = 0xa0;//('P' & 0x0F) << 1;
    TXSource[5]      = 0xa2;//('Q' & 0x0F) << 1;
    TXSource[6]      = 0x61;//(('B' & 0x0F) << 1) | 0x61;

    MAP_GPIO_setOutputLowOnPin(COMMS_RESET_PORT, COMMS_RESET_PIN);
    MAP_GPIO_setAsOutputPin(COMMS_RESET_PORT, COMMS_RESET_PIN);

    Console::log("Initialize TX and RX");
    this->initTX();
    this->initRX();

    Console::log("Configure Timer module..");
    MAP_Timer32_initModule(TIMER32_1_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT,
                    TIMER32_PERIODIC_MODE);

    Console::log("Configure PA Pins");
    MAP_GPIO_setOutputLowOnPin(PA_PORT, PA_ENABLE_PIN);
    MAP_GPIO_setAsOutputPin(PA_PORT, PA_ENABLE_PIN);
    MAP_GPIO_setOutputLowOnPin(PA_PORT, PA_MED_PIN);
    MAP_GPIO_setAsOutputPin(PA_PORT, PA_MED_PIN);
    MAP_GPIO_setOutputLowOnPin(PA_PORT, PA_HIGH_PIN);
    MAP_GPIO_setAsOutputPin(PA_PORT, PA_HIGH_PIN);
};

void COMMRadio::initTX(){
    // Initialise TX values
    // Modem set to FSK, deviation set to 1/2 datarate, gaussian filter enabled
    txRadio->init();

    if(txRadio->ping()){

        txConfig.modem = MODEM_FSK;
        txConfig.filtertype = BT_0_5;
        txConfig.bandwidth = 0; //not necessary for tx
        txConfig.fdev = txBitrate/2;
        txConfig.datarate = txBitrate;
        txConfig.power = powerByte;

        txRadio->setFrequency(PQPACKET_DOWNLINK_FREQ);
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
        rxConfig.bandwidth = 10000;
        rxConfig.bandwidthAfc = 83333;
        rxConfig.fdev = rxBitrate/2;
        rxConfig.datarate = rxBitrate;

        rxRadio->setFrequency(PQPACKET_UPLINK_FREQ);

        rxRadio->RxChainCalibration();
        rxRadio->enableBitMode(*bitSPI_rx, onReceiveWrapper, 0);

        rxRadio->setRxConfig(&rxConfig);
        rxRadio->startReceiver();

        Console::log("RX Radio Settings Set");
    }else{
        Console::log("RX Radio not Found");
    }
};


bool COMMRadio::quePacketAX25(uint8_t data[], uint8_t size){
    //return false is unsuccesful
    if(size < MAX_PACKET_SIZE && txPacketsInBuffer() < TX_MAX_FRAMES){

        AX25Frame::setAdress(txPacketBuffer[txPacketBufferWriteIndex], TXDestination, TXSource);
        AX25Frame::setControl(txPacketBuffer[txPacketBufferWriteIndex], false);
        AX25Frame::setPID(txPacketBuffer[txPacketBufferWriteIndex], 0xF0);
        AX25Frame::setPacket(txPacketBuffer[txPacketBufferWriteIndex], data, size);
        AX25Frame::calculateFCS(txPacketBuffer[txPacketBufferWriteIndex]);

        txPacketBufferWriteIndex = (txPacketBufferWriteIndex + 1) % TX_MAX_FRAMES;

        Console::log("TX - packetsInBuffer : %d, W:%d", txPacketsInBuffer(), txPacketBufferWriteIndex);

        if(!txTimeout && !txEnabled && !doEnableFlag){
            Console::log("Setting Timer");

            MAP_Timer32_registerInterrupt(TIMER32_1_INTERRUPT, &sendPacketWrapper);
            MAP_Timer32_setCount(TIMER32_1_BASE, 48000000 * TX_TIMEOUT);
            MAP_Timer32_startTimer(TIMER32_1_BASE, true);
            txTimeout = true;
        }

        return true;
    }else{
        return false;
    }
}

int COMMRadio::txPacketsInBuffer(){
    return mod(txPacketBufferWriteIndex - txPacketBufferReadIndex, TX_MAX_FRAMES);
}

void COMMRadio::enableTransmit(){
    Console::log("Enabling Transmitter!");
    txFlagQue += UPRAMP_BYTES;

    this->enablePA(this->targetPAPower);
    txEnabled = true;
    txRadio->setIdleMode(true);
    // else the radio is already on.

    //rxPrint = true;
    //rxReady = false;
}

void COMMRadio::disableTransmit(){
    Console::log("TRANSMIT DISABLED");
    this->disablePA();
    txRadio->setIdleMode(false);
    txEnabled = false;
    txIdleMode = false;
}


void COMMRadio::setIdleMode(bool idle){
    Console::log("Toggle TX Idle Mode: %s ", idle ? "ON":"OFF");
    this->txIdleMode = idle;
    if(txIdleMode){
        //device should be idling, so turn on.
        enableTransmit();
    }
};


uint8_t COMMRadio::getNumberOfRXFrames(){
    return rxPacketsInBuffer;
};

uint8_t COMMRadio::getSizeOfRXFrame(){
    if(rxPacketsInBuffer)
    {
        return rxPacketBuffer[mod(rxPacketBufferIndex - rxPacketsInBuffer, RX_MAX_FRAMES)].getSize();
    }
    else
    {
        return 0;
    }
}

uint8_t* COMMRadio::getRXFrame(){
    if(rxPacketsInBuffer)
    {
        return rxPacketBuffer[mod(rxPacketBufferIndex - rxPacketsInBuffer, RX_MAX_FRAMES)].getBytes();
    }
    else
    {
        return 0;
    }
}

void COMMRadio::popFrame(){
    if(rxPacketsInBuffer)
    {
        rxPacketsInBuffer--;
    }
}

signed short COMMRadio::getRXRSSI(){
    return rxRadio->GetRssi(ModemType::MODEM_FSK);
}

void COMMRadio::disablePA(){
    MAP_GPIO_setOutputLowOnPin(PA_PORT, PA_ENABLE_PIN);
    MAP_GPIO_setOutputLowOnPin(PA_PORT, PA_MED_PIN);
    MAP_GPIO_setOutputLowOnPin(PA_PORT, PA_HIGH_PIN);
//    __delay_cycles(48000000/(1000L/30)); // @suppress("Function cannot be resolved")
}

void COMMRadio::enablePA(uint8_t targetPower){
    disablePA();
    switch(targetPower){
    case 0:
        Console::log("PA ON LOW POWER");
        powerByte = 1;
        initTX();
        MAP_GPIO_setOutputHighOnPin(PA_PORT, PA_ENABLE_PIN);
        break;
    case 1:
        Console::log("PA ON MED POWER");
        powerByte = 3;
        initTX();
        MAP_GPIO_setOutputHighOnPin(PA_PORT, PA_MED_PIN);
        MAP_GPIO_setOutputHighOnPin(PA_PORT, PA_ENABLE_PIN);
        break;
    case 2:
        Console::log("PA ON HIGH POWER;");
        powerByte = 5;
        initTX();
        MAP_GPIO_setOutputHighOnPin(PA_PORT, PA_HIGH_PIN);
        MAP_GPIO_setOutputHighOnPin(PA_PORT, PA_ENABLE_PIN);
        break;
    default:
        Console::log("PA ON UNKNOWN POWER?! -> set to LOW");
        enablePA(0);
        break;
    }
}
