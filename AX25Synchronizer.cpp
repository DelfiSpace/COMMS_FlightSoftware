#include "AX25Synchronizer.h"



int AX25Synchronizer::mod(int a, int b)
{ return a<0 ? (a%b+b)%b : a%b; }

AX25Synchronizer::AX25Synchronizer(CLTUPacket AX25FrameBuffer[], int &AX25RXbufferIndex){
    this->receivedFrameBuffer = AX25FrameBuffer;
    this->AX25RXbufferIndex = &AX25RXbufferIndex;
}

//void AX25Synchronizer::queBit(uint8_t inBit){
//    byteBuffer[byteBufferIndex] = inBit;
//    byteBufferIndex = (byteBufferIndex + 1)%BYTE_BUFFER_SIZE;
//}
bool AX25Synchronizer::queByte(uint8_t inByte){
    byteQue[this->byteQueIndex] = inByte;
    byteQueIndex = mod(byteQueIndex+1, BYTE_QUE_SIZE);
    bytesInQue = bytesInQue + 1;

    if(bytesInQue > BYTE_QUE_SIZE){
//        serial.println("[!!]");
    }

    return true;
}

bool AX25Synchronizer::rxBit(){
    bool packetReceived = false;
    if(bytesInQue <= 0){
        return packetReceived;
    }
    uint8_t inByte = byteQue[mod(byteQueIndex - bytesInQue, BYTE_QUE_SIZE)];
    bytesInQue = bytesInQue - 1;

    for(int i = 0; i < 8; i++){
        uint8_t inBit = encoder.NRZIdecodeBit(BitArray::getBit(&inByte, i, 8));
        inBit = encoder.descrambleBit(inBit);
        switch(this->synchronizerState){
            case 0:
                //flagDetect
                BitArray::setBit(&flagBuffer, flagBufferIndex, inBit == 0x01);

                if(BitArray::getByte(&flagBuffer, flagBufferIndex - 7, 8) == 0x7E ){
                    //serial.println("Flag!");
                    bitBuffer[0] = 0;
                    bitBufferIndex = 0;
                    synchronizerState = 1;
                }
                flagBufferIndex = mod(flagBufferIndex + 1, 8);
                break;
            case 1:
                //checkForStart
                BitArray::setBit(bitBuffer, bitBufferIndex, inBit == 0x01);
                if(bitBufferIndex == 7){
                    if(bitBuffer[0] == 0x7E ){
                        //serial.println("Another Flag!");
                        bitBufferIndex = 0;
                        bitBuffer[0] = 0;
                        synchronizerState = 1;
                        break; //early break due to bitBufferIndex reset
                    }else{
                        //serial.print("Msg start? ");
                        //serial.println(bitBuffer[0], HEX);
                        synchronizerState = 2;
                    }
                }
                bitBufferIndex = mod(bitBufferIndex + 1, BYTE_BUFFER_SIZE*8);
                break;
            case 2:
                //receivingMsg
                BitArray::setBit(bitBuffer, bitBufferIndex, inBit == 0x01);
                if(bitBufferIndex >= 7 && BitArray::getByte(bitBuffer, bitBufferIndex - 7, 8*BYTE_BUFFER_SIZE) == 0x7E ){
                    //serial.println("EndSeq Received!");
                    //destuff received BitSequence
                    int rxBits_n = this->encoder.destuffBits(bitBuffer, receivedFrameBuffer[*AX25RXbufferIndex].data, bitBufferIndex-7);
                    //This could be a packet?
                    if(rxBits_n % 8 == 0){ //packet has whole bytes!
                        //reverseOrder on all bytes

                        for(int p = 0; p < rxBits_n/8; p++){
                            this->receivedFrameBuffer[*AX25RXbufferIndex].data[p] = AX25Frame::reverseByteOrder(this->receivedFrameBuffer[*AX25RXbufferIndex].data[p]);
                        }
                        this->receivedFrameBuffer[*AX25RXbufferIndex].packetSize = rxBits_n/8;
//                        serial.println("#");
//                        serial.println(receivedFrameBuffer[*AX25RXbufferIndex].packetSize , DEC);
//                        for(int p = 0; p < (bitBufferIndex-7)/8; p++){
//                            serial.print(bitBuffer[p],HEX);
//                            serial.print(" ");
//                        }serial.println();
//                        for(int p = 0; p < rxBits_n/8; p++){
//                            serial.print(receivedFrameBuffer[*AX25RXbufferIndex].data[p],HEX);
//                            serial.print(" ");
//                        }serial.println();
                        if(AX25Frame::checkFCS(this->receivedFrameBuffer[*AX25RXbufferIndex])){

                            Console::log("!");
                            this->hasReceivedFrame = true;
                            packetReceived = true;
                            receivedFrameBuffer[*AX25RXbufferIndex].isLocked = false;
                            receivedFrameBuffer[*AX25RXbufferIndex].isReady = true;


//                            serial.print(*AX25RXbufferIndex, DEC);
//                            serial.print("  -  ");
//                            serial.print(this->receivedFrameBuffer[*AX25RXbufferIndex].packetSize, DEC);
//                            for(int p = 0; p < rxBits_n/8; p++){
//                                serial.print(this->receivedFrameBuffer[*AX25RXbufferIndex].data[p], HEX);
//                                serial.print(" ");
//                            }
//                            serial.println();
                            Console::log("%d",this->receivedFrameBuffer[*AX25RXbufferIndex].data[16]);

                            *AX25RXbufferIndex = (*AX25RXbufferIndex + 1)%RX_FRAME_BUFFER;

                            receivedFrameBuffer[*AX25RXbufferIndex].isLocked = true;
                            receivedFrameBuffer[*AX25RXbufferIndex].isReady = false;

                        }
                    }
                    flagBuffer = 0;
                    synchronizerState = 0;
                }
                if(bitBufferIndex > 255*8){
                    //serial.println("No Msg! Too Long!");
                    flagBuffer = 0;
                    synchronizerState = 0;
                }
                bitBufferIndex = mod(bitBufferIndex + 1, BYTE_BUFFER_SIZE*8);
                if(bitBufferIndex%8 == 0){
                    bitBuffer[bitBufferIndex/8] = 0;
                }
                break;
        }
    }

    return packetReceived;
}
