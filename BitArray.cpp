#include "BitArray.h"

extern DSerial serial;

int BitArray::mod(int a, int b){
    return a<0 ? (a%b+b)%b : a%b;
}

uint8_t BitArray::getBit(uint8_t byteArray[], int bitIndex_in, int arraySize){
    int bitIndex = BitArray::mod(bitIndex_in, arraySize);
    //serial.print((byteArray[bitIndex/8] >> (7-(bitIndex%8)) ) & 0x01, HEX);
    return  (byteArray[bitIndex/8] >> (7-(bitIndex%8)) ) & 0x01;
}

uint8_t BitArray::getBit(uint8_t byteArray[], int bitIndex){
    //serial.print((byteArray[bitIndex/8] >> (7-(bitIndex%8)) ) & 0x01, HEX);
    return  (byteArray[bitIndex/8] >> (7-(bitIndex%8)) ) & 0x01;
}

uint8_t BitArray::getByte(uint8_t byteArray[], int bitIndex, int arraySize){
    //create 2byte short from msbbyte
    unsigned short intermediateValue = (byteArray[BitArray::mod(bitIndex, arraySize)/8] << 8) +  (byteArray[BitArray::mod(bitIndex+8, arraySize)/8]);
    //get desired msb to actual msb
    intermediateValue = intermediateValue << (BitArray::mod(bitIndex, arraySize)%8);
    //return msbByte
    return  (uint8_t)((intermediateValue >> 8) & 0xFF);
}


void BitArray::xorBit(uint8_t byteArray[], int bitIndex){
    byteArray[bitIndex/8] ^= 0x01 << (7-(bitIndex%8));
}

void BitArray::setBit(uint8_t byteArray[], int bitIndex, bool state){
    if( (getBit(byteArray, bitIndex) == 0x01 && state == false) || (getBit(byteArray, bitIndex) == 0x00 && state == true)  ){
        xorBit(byteArray,bitIndex);
    }
}

int BitArray::compareArrays(uint8_t byteArray1[], uint8_t byteArray2[], int bitSize){
    int score = 0;
    for( int k = 0; k < bitSize; k++){
        if(getBit(byteArray1, k) != getBit(byteArray1, k)){
            score += 1;
        }
    }
    return score;
}
