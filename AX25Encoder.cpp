#include "AX25Encoder.h"

extern DSerial serial;

uint8_t AX25Encoder::scrambleBit(uint8_t bit){
    SCRAMBLE_BIT = ((SCRAMBLE_BYTES >> 11) & 0x01) ^ ((SCRAMBLE_BYTES >> 16) & 0x01) ^ (bit & 0x01);
    SCRAMBLE_BYTES <<= 1;
    SCRAMBLE_BYTES |= SCRAMBLE_BIT & 0x01;
    SCRAMBLE_BYTES &= 0x01FFFF;
    return SCRAMBLE_BIT;
}

uint8_t AX25Encoder::scrambleByte(uint8_t inByte){
    uint8_t outputByte = 0;
    for(int i = 0; i < 8; i++){
        outputByte |= (scrambleBit(inByte >> i) << i);
    }
    return outputByte;
}

uint8_t AX25Encoder::descrambleBit(uint8_t bit){
    DESCRAMBLE_BIT = ((DESCRAMBLE_BYTES >> 11) & 0x01) ^ ((DESCRAMBLE_BYTES >> 16) & 0x01) ^ (bit & 0x01);
    DESCRAMBLE_BYTES <<= 1;
    DESCRAMBLE_BYTES |= bit & 0x01;
    DESCRAMBLE_BYTES &= 0x01FFFF;
    return DESCRAMBLE_BIT;
}

uint8_t AX25Encoder::descrambleByte(uint8_t inByte){
    uint8_t outputByte = 0;
    for(int i = 0; i < 8; i++){
        outputByte |= (descrambleBit(inByte >> i) << i);
    }
    return outputByte;
}

uint8_t AX25Encoder::NRZIencodeBit(uint8_t bit){

    //(bit & 0x01) == 0x01 -> bit is 1
    //(bit & 0x01) != 0x01 -> bit is 0
    bool isHigh = !((bit & 0x01) == 0x01);
    NRZI_ENCODER = NRZI_ENCODER != isHigh;
    return (NRZI_ENCODER ? 0x01 : 0x00);
}

uint8_t AX25Encoder::NRZIdecodeBit(uint8_t bit){

    //(bit & 0x01) == 0x01 -> bit is 1
    //(bit & 0x01) != 0x01 -> bit is 0
    bool isHigh = ((bit & 0x01) == 0x01);
    bool tmp = NRZI_DECODER;
    NRZI_DECODER = isHigh;
    return (isHigh != tmp ? 0x00 : 0x01);
}

uint8_t AX25Encoder::NRZIencodeByte(uint8_t inByte){

    uint8_t outputByte = 0;
    for(int i = 0; i < 8; i++){
        outputByte |= (NRZIencodeBit(inByte >> i) << i);
    }
    return outputByte;
}

uint8_t AX25Encoder::NRZIdecodeByte(uint8_t inByte){

    uint8_t outputByte = 0;
    for(int i = 0; i < 8; i++){
        outputByte |= (NRZIdecodeBit(inByte >> i) << i);
    }
    return outputByte;
}

uint8_t AX25Encoder::txBit(uint8_t inBit, bool bitStuffing, bool scrambling, bool NRZIencoding){
    uint8_t outBit;
    if(bitsInBuffer == 0){
        outBit = inBit;
    }else{
        //take bit from buffer
        outBit = bitStuffingBuffer[bitsInBuffer-1];
        bitsInBuffer--;
        //serial.print("[s]");
    }

    if(bitStuffing){
        //serial.print(outBit, HEX);
        if(inBit > 0){
            bitCounter++;
            //serial.print("1");
        }else{
            bitCounter = 0;
            //serial.print("0");
        }
        if(bitCounter >= 5){
            bitStuffingBuffer[bitsInBuffer] = 0x00;
            bitsInBuffer++;
            bitCounter = 0;
            //serial.print("[stuffing!]");
        }
    }
    if(scrambling){
        outBit = this->scrambleBit(outBit);

    }
    if(NRZIencoding){
        outBit = this->NRZIencodeBit(outBit);
    }

    //serial.print((outBit > 0 ? 0x01 : 0x00), HEX);
    return (outBit > 0 ? 0x01 : 0x00);
}

uint8_t AX25Encoder::txByte(uint8_t inByte, bool bitStuffing, bool scrambling, bool NRZIencoding){

    uint8_t outByte = 0;

    for(int i = 0; i < 8; i++){
        outByte = outByte | (this->txBit((inByte >> i)&0x01, bitStuffing, scrambling, NRZIencoding) << i);
    }

    return outByte;
}
