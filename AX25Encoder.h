#include "SX1276.h"
#include "DSPI.h"
#include "sx1276Enums.h"
#include "DSerial.h"

#ifndef AX25ENCODER_H_
#define AX25ENCODER_H_

class AX25Encoder
{
protected:
    uint16_t SCRAMBLE_BYTES = 0;
    uint8_t SCRAMBLE_BIT = 0;

    uint16_t DESCRAMBLE_BYTES = 0;
    uint8_t DESCRAMBLE_BIT = 0;

    uint8_t bitStuffingBuffer[100];

    bool NRZI_ENCODER = false;
    bool NRZI_DECODER = true;

    uint8_t bufferEmpty = true;
    uint8_t bitsInBuffer = 0;
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

    uint8_t txBit(uint8_t inBit, bool bitStuff, bool NRZI, bool scrambling);
};

#endif
