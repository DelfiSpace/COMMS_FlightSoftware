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
    bool packetReceived = false;
    byteBuffer[byteBufferIndex] = inBit;
    bitCounter++;
    //for(int i = 7; i >= 0; i--){
    //    serial.print(byteBuffer[(byteBufferIndex - i)%BYTE_BUFFER_SIZE], DEC);
    //}
    //serial.println();
    if( byteBuffer[(byteBufferIndex - 15)%BYTE_BUFFER_SIZE] == 0 &&
            byteBuffer[(byteBufferIndex - 14)%BYTE_BUFFER_SIZE] == 1 &&
            byteBuffer[(byteBufferIndex - 13)%BYTE_BUFFER_SIZE] == 1 &&
            byteBuffer[(byteBufferIndex - 12)%BYTE_BUFFER_SIZE] == 1 &&
            byteBuffer[(byteBufferIndex - 11)%BYTE_BUFFER_SIZE] == 1 &&
            byteBuffer[(byteBufferIndex - 10)%BYTE_BUFFER_SIZE] == 1 &&
            byteBuffer[(byteBufferIndex - 9)%BYTE_BUFFER_SIZE] == 1 &&
            byteBuffer[(byteBufferIndex - 8)%BYTE_BUFFER_SIZE] == 0 &&
            byteBuffer[(byteBufferIndex - 7)%BYTE_BUFFER_SIZE] == 0 &&
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

        if(bitCounter > 39 && bitCounter < 3000){
            serial.print(bitCounter, DEC);
            serial.println("  == doubleFlag!");
            bitCounter = 0;
        }else{
            bitCounter = 0;
        }
    }

    byteBufferIndex = (byteBufferIndex + 1)%BYTE_BUFFER_SIZE;
    return packetReceived;
}
