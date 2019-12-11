#include "LDPC_decoder.h"

extern DSerial serial;

uint8_t getBit(uint8_t byteArray[], int bitIndex){
    //serial.print((byteArray[bitIndex/8] >> (7-(bitIndex%8)) ) & 0x01, HEX);
    return  (byteArray[bitIndex/8] >> (7-(bitIndex%8)) ) & 0x01;
}

void xorBit(uint8_t byteArray[], int bitIndex){
    byteArray[bitIndex/8] ^= 0x01 << (7-(bitIndex%8));
}

void setBit(uint8_t byteArray[], int bitIndex, bool state){
    if( (getBit(byteArray, bitIndex) == 0x01 && state == false) || (getBit(byteArray, bitIndex) == 0x00 && state == true)  ){
        xorBit(byteArray,bitIndex);
    }
}

bool LDPCDecoder::getParity(uint8_t input[]){
    int sumSn = 0;
    for(int i = 0; i < 2048; i++){

        if(i % 8 == 0){
            setBit(this->sn, i/8, false);
        }

        if( getBit(input,this->H1[i]) == 0x01 ){
            xorBit(this->sn, i/8);
        }


        if(i % 8 == 7){
            sumSn = sumSn + getBit(sn, i/8);
        }
    }

    if (sumSn == 0){
        return true;
    }
    else{
        return false;
    }
}

void LDPCDecoder::getScore(){
    for(int i = 0; i < 256; i++){
        this->en[i] = 0;
    }
    for(int i = 0; i < 2048; i++){
        this->en[this->H1[i]] += getBit(this->sn, i/8);
    }
}

uint8_t MaxValue(uint8_t inputArray[], int size){
    uint8_t maxValue = 0;
    for( int i = 0; i < size; i++){
        if(inputArray[i] > maxValue){
            maxValue = inputArray[i];
        }
    }
    return maxValue;
}

bool LDPCDecoder::iterateBitflip(uint8_t input[]){
    serial.println("Iteration!");
    if(this->getParity(input)){
        return true;
    }else{
        this->getScore();
        uint8_t maxVal = MaxValue(this->en, 512);

        for(int i = 0; i < 512; i++){
            if(this->en[i] == maxVal){
                xorBit(input,i);
            }
        }
        return false;
    }
}

