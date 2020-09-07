#include "AX25Synchronizer.h"
#include "COMMRadio.h"

int AX25Synchronizer::mod(int a, int b)
{ return a<0 ? (a%b+b)%b : a%b; }

AX25Synchronizer::AX25Synchronizer(PQPacket AX25FrameBuffer[], int &AX25RXbufferIndex){
    this->receivedFrameBuffer = AX25FrameBuffer;
    this->AX25RXbufferIndex = &AX25RXbufferIndex;
}

bool AX25Synchronizer::queByte(uint8_t inByte){
    if(bytesInQue() < BYTE_QUE_SIZE - 1)
    {
        byteQue[byteQueWriteIndex] = inByte;
        byteQueWriteIndex = mod(byteQueWriteIndex+1, BYTE_QUE_SIZE);
    }
    else{
        Console::log("[!!]");
    }
    return true;
}

int AX25Synchronizer::bytesInQue(){
    return mod(byteQueWriteIndex - byteQueReadIndex, BYTE_QUE_SIZE);
}

bool AX25Synchronizer::rxBit(){
    bool packetReceived = false;
    if(bytesInQue() <= 1){
        return 0;
    }
    uint8_t inByte = byteQue[byteQueReadIndex];
    byteQueReadIndex = mod(byteQueReadIndex+1, BYTE_QUE_SIZE);

    for(int i = 0; i < 8; i++){
        uint8_t inBit = encoder.NRZIdecodeBit(BitArray::getBit(&inByte, i, 8));
        inBit = encoder.descrambleBit(inBit);
        switch(this->synchronizerState){
            case 0:
                //flagDetect
                BitArray::setBit(&flagBuffer, flagBufferIndex, inBit == 0x01);
                flagBufferIndex = mod(flagBufferIndex + 1, 8);

                if(BitArray::getByte(&flagBuffer, flagBufferIndex - 8, 8) == 0x7E ){
                    //Flag Detected!
                    flagBuffer = 0;
                    flagBufferIndex = 0;
                    bitBuffer[0] = 0;
                    bitBufferIndex = 0;
                    synchronizerState = 1;
                }

                break;
            case 1:
                //checkForStart
                BitArray::setBit(bitBuffer, bitBufferIndex, inBit == 0x01);
                bitBufferIndex++;
                if(bitBufferIndex == 7){
                    if(bitBuffer[0] == 0x7E ){
                        //Another Flag, Reset bitBuffer
                        bitBufferIndex = 0;
                        bitBuffer[0] = 0;
                        synchronizerState = 1;
                    }else{
                        //First Byte in! continue to the wait for end state
                        synchronizerState = 2;
                    }
                }
                break;
            case 2:
                //Wait for ending State

                //Set next bit in Buffer
                BitArray::setBit(bitBuffer, bitBufferIndex, inBit == 0x01);
                bitBufferIndex++;
                if(bitBufferIndex >= 8 && BitArray::getByte(bitBuffer, bitBufferIndex - 8, 8*BYTE_BUFFER_SIZE) == 0x7E ){
                    // End Sequence Byte Detected;
                    // Whether this is an actual packet or not, it doesnt matter, go back to flagdetect afterwards
                    synchronizerState = 0;

                    // Get length of Packet, destuff and update length
                    int rxBits_n = bitBufferIndex - 8;
                    rxBits_n = this->encoder.destuffBits(bitBuffer, receivedFrameBuffer[*AX25RXbufferIndex].data, rxBits_n);

                    //This could be a packet?
                    if(rxBits_n % 8 == 0 && rxBits_n/8 >= 18){ //packet has whole bytes and minimum length of 18 bytes!

                        //reverseOrder on all bytes
                        for(int p = 0; p < rxBits_n/8; p++){
                            this->receivedFrameBuffer[*AX25RXbufferIndex].data[p] = AX25Frame::reverseByteOrder(this->receivedFrameBuffer[*AX25RXbufferIndex].data[p]);
                        }
                        this->receivedFrameBuffer[*AX25RXbufferIndex].packetSize = rxBits_n/8;

                        if(AX25Frame::checkFCS(this->receivedFrameBuffer[*AX25RXbufferIndex])){
                            *AX25RXbufferIndex = (*AX25RXbufferIndex + 1) % RX_MAX_FRAMES;
                            packetReceived = true;
                        }
                    }
                }
                if(bitBufferIndex > 255*8){
                    //This Message is too long, just return to flagdetect
                    synchronizerState = 0;
                }
                break;
        }
    }
    return packetReceived;
}
