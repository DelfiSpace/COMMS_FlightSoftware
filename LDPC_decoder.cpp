#include "LDPC_decoder.h"

extern DSerial serial;

bool LDPCDecoder::getParity(uint8_t input[]){
    for(int i = 0; i < 256; i++){
        this->sn[i] = 0;
    }

    for(int i = 0; i < 2048; i++){
        if( BitArray::getBit(input,this->H1[i]) == 0x01 ){
            this->sn[i/8] = this->sn[i/8] ^ 0x01;
        }
    }


    int sumSn = 0;
    for(int i = 0; i < 256; i++){
        sumSn = sumSn + sn[i];
    }
    if (sumSn == 0){
        return true;
    }
    else{
        return false;
    }
}

void LDPCDecoder::getScore(){
    for(int i = 0; i < 512; i++){
        this->en[i] = 0;
    }
    for(int i = 0; i < 2048; i++){
        en[H1[i]] += this->sn[i/8];
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
//    serial.println("Iteration!");
    if(this->getParity(input)){
        return true;
    }else{
        this->getScore();
        uint8_t maxVal = MaxValue(this->en, 512);
//        serial.println("Sn Array:");
//        for(int i = 0; i < 256; i++){
//            serial.print(sn[i], DEC);
//            serial.print(" ");
//        }
//        serial.println();
//
//
//        serial.println("En Array:");
//        for(int i = 0; i < 512; i++){
//            serial.print(en[i], DEC);
//            serial.print(" ");
//        }
//        serial.println();

        for(int i = 0; i < 512; i++){
            if(this->en[i] == maxVal){
//                serial.print("MaxVal: ");
//                serial.print(maxVal, DEC);
//                serial.print("  CHANGE: ");
//                serial.print(i,DEC);
//                serial.println();
                BitArray::xorBit(input,i);
                //break;
            }
        }
        return false;
    }
}

