#include "AX25Frame.h"

extern DSerial serial;

uint8_t AX25Frame::scrambleBit(uint8_t bit){
    SCRAMBLE_BIT = ((SCRAMBLE_BYTES >> 11) & 0x01) ^ ((SCRAMBLE_BYTES >> 16) & 0x01) ^ (bit & 0x01);
    SCRAMBLE_BYTES <<= 1;
    SCRAMBLE_BYTES |= SCRAMBLE_BIT & 0x01;
    SCRAMBLE_BYTES &= 0x01FFFF;
    return SCRAMBLE_BIT;
}

uint8_t AX25Frame::scrambleByte(uint8_t inByte){
    uint8_t outputByte = 0;
    for(int i = 0; i < 8; i++){
        outputByte |= scrambleBit(inByte >> i) << i;
    }
    return outputByte;
}

uint8_t AX25Frame::descrambleBit(uint8_t bit){
    DESCRAMBLE_BIT = ((DESCRAMBLE_BYTES >> 11) & 0x01) ^ ((DESCRAMBLE_BYTES >> 16) & 0x01) ^ (bit & 0x01);
    DESCRAMBLE_BYTES <<= 1;
    DESCRAMBLE_BYTES |= bit & 0x01;
    DESCRAMBLE_BYTES &= 0x01FFFF;
    return DESCRAMBLE_BIT;
}
