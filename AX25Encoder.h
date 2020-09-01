#include "SX1276.h"
#include "DSPI.h"
#include "sx1276Enums.h"
#include "BitArray.h"

#ifndef AX25ENCODER_H_
#define AX25ENCODER_H_

class AX25Encoder
{
protected:
    //uint8_t bitStuffingBuffer[10] = {0};

    bool NRZI_ENCODER = true;
    bool NRZI_DECODER = true;

    int G3RUHscramble = 0;
    int G3RUHscramble_bit = 0;
    int G3RUHdescramble = 0;
    int G3RUHdescramble_bit = 0;

    uint8_t bitCounter = 0;

public:
    uint8_t AX25_FLAG = 0x7E;

    uint8_t scrambleBit(uint8_t bit);
    uint8_t descrambleBit(uint8_t bit);

    uint8_t scrambleByte(uint8_t inbyte);
    uint8_t descrambleByte(uint8_t inbyte);

    uint8_t NRZIencodeBit(uint8_t bit);
    uint8_t NRZIdecodeBit(uint8_t bit);

    uint8_t NRZIencodeByte(uint8_t inbyte);
    uint8_t NRZIdecodeByte(uint8_t inbyte);

    uint8_t txBit(uint8_t inBit, bool bitStuff);
    uint8_t StuffBitsInBuffer = 0;

    int destuffBits(uint8_t inBuffer[], uint8_t outBuffer[], int bitCount);
};

#endif
