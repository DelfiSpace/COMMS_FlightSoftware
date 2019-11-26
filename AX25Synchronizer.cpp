#include "AX25Synchronizer.h"

extern DSerial serial;

bool AX25Synchronizer::compareBitArrays(uint8_t array1[], uint8_t array2[], uint8_t size){
    bool out = true;
    for(int i = 0; i < size; i++){
        if(array1[i] != array2[i]){
            out = false;
        }
    }
    return out;
}

bool AX25Synchronizer::rxBit(uint8_t inBit){
    if(inBit != 0x01 && inBit != 0x00){
        serial.println("NOT A BIT?!");
    }
    bool packetReceived = false;
    byteBuffer[byteBufferIndex] = inBit;
    bitCounter++;
    //for(int i = 7; i >= 0; i--){
    //    serial.print(byteBuffer[(byteBufferIndex - i)%BYTE_BUFFER_SIZE], DEC);
    //}
    //serial.println();
    if(     byteBuffer[(byteBufferIndex - 7)%BYTE_BUFFER_SIZE] == 0 &&
            byteBuffer[(byteBufferIndex - 6)%BYTE_BUFFER_SIZE] == 1 &&
            byteBuffer[(byteBufferIndex - 5)%BYTE_BUFFER_SIZE] == 1 &&
            byteBuffer[(byteBufferIndex - 4)%BYTE_BUFFER_SIZE] == 1 &&
            byteBuffer[(byteBufferIndex - 3)%BYTE_BUFFER_SIZE] == 1 &&
            byteBuffer[(byteBufferIndex - 2)%BYTE_BUFFER_SIZE] == 1 &&
            byteBuffer[(byteBufferIndex - 1)%BYTE_BUFFER_SIZE] == 1 &&
            byteBuffer[(byteBufferIndex - 0)%BYTE_BUFFER_SIZE] == 0
        ){
        //last received bit completed a flag, the tail of a transfer exists of flags, hence check byteBuffer for packet;
        //minimum frame length is 4 bytes, maximum bits is decided by Buffer.

        if(bitCounter > 8*(14+2+2) && bitCounter < 1000){

            //start destuffing bits:
            int destuffIndex = 0;
            int destuffBitIndex = 0;
            int destuffCount = 0;
            int destuffs = 0;

            //destuff Bits and Fix Ordering (every octet is received LSB first).
            this->destuffBuffer[0] = 0;
            for(int k = 0; k < bitCounter - 8; k++){
                this->destuffBuffer[destuffIndex] |= ((byteBuffer[(byteBufferIndex - bitCounter + 1 + k)%BYTE_BUFFER_SIZE] & 0x01) << destuffBitIndex);
                //serial.print(destuffBuffer[destuffIndex], HEX);
                //serial.print(destuffIndex + destuffBitIndex, DEC);
                //serial.println();
                if(byteBuffer[(byteBufferIndex - bitCounter + 1 + k)%BYTE_BUFFER_SIZE] == 0x01){
                    destuffCount++;
                    //serial.print("1");
                }
                if(byteBuffer[(byteBufferIndex - bitCounter + 1 + k)%BYTE_BUFFER_SIZE] == 0x00){
                    destuffCount = 0;
                    //serial.print("0");
                }
                if(destuffCount == 5){
                    destuffCount = 0;
                    destuffs++;;
                    k = k + 1; //skip next bit.
                }
                destuffBitIndex++;
                if(destuffBitIndex >= 8){
                    destuffBitIndex = 0;
                    destuffIndex++;
                    this->destuffBuffer[destuffIndex] = 0;
                }
            }
            int packetBits = bitCounter - destuffs - 8 - 16;

            if(packetBits % 8 == 0 && destuffBuffer[0] == 0x82){ // 'correct' packets are always whole bytes
                this->receivedFrame.setData(destuffBuffer, packetBits/8);
                this->receivedFrame.calculateFCS();
                if( ((uint8_t) (this->receivedFrame.FCSField >> 8)) == destuffBuffer[packetBits/8] &&
                        ((uint8_t) (this->receivedFrame.FCSField & 0xFF)) == destuffBuffer[packetBits/8 + 1]    ){
                    this->hasReceivedFrame = true;
                    packetReceived = true;
                    serial.println("!");
                }
            }

            bitCounter = 0;
        }else{
            bitCounter = 0;
        }
    }

    byteBufferIndex = (byteBufferIndex + 1)%BYTE_BUFFER_SIZE;

    return packetReceived;
}
