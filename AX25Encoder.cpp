#include "AX25Encoder.h"

extern DSerial serial;

uint8_t AX25Encoder::scrambleBit(uint8_t bit){
    int isHigh = (bit >= 0x01) ? 0x01:0x00;
    G3RUHscramble_bit = (((G3RUHscramble >> 11) & 0x01) ^ ((G3RUHscramble >> 16) & 0x01)) ^ (isHigh & 0x01);
    G3RUHscramble <<= 1;
    G3RUHscramble = G3RUHscramble | (G3RUHscramble_bit & 0x01);
    G3RUHscramble &= 0x01FFFF;

    return G3RUHscramble_bit;
}

uint8_t AX25Encoder::descrambleBit(uint8_t bit){
    G3RUHdescramble_bit = ((G3RUHdescramble >> 11) & 0x01) ^ ((G3RUHdescramble >> 16) & 0x01) ^ (bit & 0x01);
    G3RUHdescramble <<= 1;
    G3RUHdescramble |= bit & 0x01;
    G3RUHdescramble &= 0x01FFFF;
    return G3RUHdescramble_bit;
}

uint8_t AX25Encoder::NRZIencodeBit(uint8_t bit){

    //(bit & 0x01) == 0x01 -> bit is 1
    //(bit & 0x01) != 0x01 -> bit is 0
    bool isHigh = ((bit & 0x01) == 0x01);
    NRZI_ENCODER = NRZI_ENCODER != !isHigh;
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

uint8_t AX25Encoder::txBit(uint8_t inBit, bool bitStuffing){
    uint8_t outBit;
    if(bitsInBuffer == 0){
        outBit = inBit;
    }else{
        //take bit from buffer
        outBit = 0x00;
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
            bitsInBuffer++;
            bitCounter = 0;
            //serial.print("[stuffing!]");
        }
    }
    outBit = this->NRZIencodeBit(scrambleBit(outBit));
    //serial.print(outBit);

    return outBit;
}
