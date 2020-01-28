#include "AX25Synchronizer.h"

extern DSerial serial;


int AX25Synchronizer::mod(int a, int b)
{ return a<0 ? (a%b+b)%b : a%b; }

int AX25Synchronizer::clip(int a, int b)
{ return a>b ? a-mod(a,b) : a; }


AX25Synchronizer::AX25Synchronizer(CLTUPacket AX25FrameBuffer[], int &AX25RXbufferIndex){
    this->receivedFrameBuffer = AX25FrameBuffer;
    this->AX25RXbufferIndex = &AX25RXbufferIndex;
}

bool AX25Synchronizer::compareBitArrays(uint8_t array1[], uint8_t array2[], uint8_t size){
    bool out = true;
    for(int i = 0; i < size; i++){
        if(array1[i] != array2[i]){
            out = false;
        }
    }
    return out;
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
        //serial.println("[!!]");
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
        //serial.print(byteBufferIndex);
        uint8_t inBit = encoder.NRZIdecodeBit(BitArray::getBit(&inByte, i, 8));
        inBit = encoder.descrambleBit(inBit);
        BitArray::setBit(bitBuffer, bitBufferIndex, inBit == 0x01);
        bitCounter++;

        if(BitArray::getByte(bitBuffer, bitBufferIndex - 7, 8*BYTE_BUFFER_SIZE) == 0x7E ){
            //last received bit completed a flag, the tail of a transfer exists of flags, hence check byteBuffer for packet;
            //minimum frame length is 4 bytes, maximum bits is decided by Buffer.

            if(bitCounter > 8*(14+2+2) && bitCounter < 8*(256)){
                //start destuffing bits:
                int destuffIndex = 0;
                int destuffBitIndex = 0;
                int destuffCount = 0;

                //destuff Bits and Fix Ordering (every octet is received LSB first).
                this->receivedFrameBuffer[*AX25RXbufferIndex].data[0] = 0;
                for(int k = 0; k < bitCounter - 8; k++){
                    if(BitArray::getBit(bitBuffer, bitBufferIndex - bitCounter + 1 + k, 8*BYTE_BUFFER_SIZE) == 0x01){
                        BitArray::setBit(&receivedFrameBuffer[*AX25RXbufferIndex].data[destuffIndex], 7-destuffBitIndex, true);
                        destuffCount++;
                        //serial.print("1");
                    }else{
                        BitArray::setBit(&receivedFrameBuffer[*AX25RXbufferIndex].data[destuffIndex], 7-destuffBitIndex, false);
                        destuffCount = 0;
                        //serial.print("0");
                    }
                    if(destuffCount == 5){
                        destuffCount = 0;
                        k = k + 1; //skip next bit.
                    }
                    destuffBitIndex++;
                    if(destuffBitIndex >= 8){
                        destuffBitIndex = 0;
                        destuffIndex++;
                    }
                }

                if(destuffBitIndex == 0 ){//&& receivedFrameBuffer[*AX25RXbufferIndex].FrameBytes[0] == 0x82){ // 'correct' packets are always whole bytes
                    int packetBits = destuffIndex*8;
                    int oldSize = receivedFrameBuffer[*AX25RXbufferIndex].packetSize;
                    receivedFrameBuffer[*AX25RXbufferIndex].packetSize = packetBits/8;

                    if(AX25Frame::checkFCS(this->receivedFrameBuffer[*AX25RXbufferIndex])){
                        this->hasReceivedFrame = true;
                        packetReceived = true;
                        receivedFrameBuffer[*AX25RXbufferIndex].isLocked = false;
                        receivedFrameBuffer[*AX25RXbufferIndex].isReady = true;

                        serial.print(*AX25RXbufferIndex, DEC);
                        serial.print("  -  ");
                        serial.print(this->receivedFrameBuffer[*AX25RXbufferIndex].packetSize, DEC);
                        serial.println();

                        *AX25RXbufferIndex = (*AX25RXbufferIndex + 1)%RX_FRAME_BUFFER;

                        receivedFrameBuffer[*AX25RXbufferIndex].isLocked = true;
                        receivedFrameBuffer[*AX25RXbufferIndex].isReady = false;


                    }else{
                        receivedFrameBuffer[*AX25RXbufferIndex].packetSize = oldSize;
                    }
                }
                bitCounter = 0;
            }else{
                bitCounter = 0;
            }
        }
        bitBufferIndex = mod(bitBufferIndex + 1, BYTE_BUFFER_SIZE*8);
    }

    return packetReceived;
}
