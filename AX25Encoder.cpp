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
    bool isHigh = ((bit & 0x01) == 0x01);
    NRZI_ENCODER = NRZI_ENCODER != isHigh;
    return (NRZI_ENCODER ? 0x01 : 0x00);
}

uint8_t AX25Encoder::NRZIdecodeBit(uint8_t bit){

    //(bit & 0x01) == 0x01 -> bit is 1
    //(bit & 0x01) != 0x01 -> bit is 0
    bool isHigh = ((bit & 0x01) == 0x01);
    bool tmp = NRZI_DECODER;
    NRZI_DECODER = isHigh;
    return (isHigh != tmp ? 0x01 : 0x00);
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

