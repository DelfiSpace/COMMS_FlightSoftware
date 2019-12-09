#include "LDPC_decoder.h"

extern DSerial serial;

void LDPCDecoder::setInput(uint8_t inputBuffer[]){
    this->inputReady = false;
    for(int i = 0; i < 512; i++){
        this->v[i] = inputBuffer[i];
    }
}


bool LDPCDecoder::getParity(){
    for(int i = 0; i < 256; i++){
        this->sn[i] = 0;
    }
    int sumSn = 0;
    for(int i = 0; i < 2048; i++){
        this->sn[i/8] = this->sn[i/8] ^ (this->v[this->H1[i]] & 0x01);
        if(i % 8 == 7){
            sumSn = sumSn + sn[i/8];
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
        this->en[this->H1[i]] += (this->sn[i/8] & 0x01);
    }
}

uint8_t MaxValue(uint8_t inputArray[], int size){
    uint8_t maxValue = 0;
    for( int i = 0; i < size; i++){
        if(inputArray[i] > maxValue){
            maxValue = inputArray[i];
        }
    }
}

bool LDPCDecoder::iterateBitflip(){
    serial.println("# parity bits:");
    if(this->getParity()){
        this->inputReady = true
        return true;
    }else{
        this->getScore();
        uint8_t maxVal = MaxValue(this->en, 512);

        for(int i = 0; i < 512; i++){
            if(this->en[i] == maxVal){
                this->v[i] ^= (0x01);
            }
        }

        return false;
    }
}
