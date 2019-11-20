#include "SX1276.h"
#include "DSPI.h"
#include "sx1276Enums.h"
#include "DSerial.h"

#ifndef AX25FRAME_H_
#define AX25FRAME_H_

class AX25Frame
{
protected:
    uint16_t SCRAMBLE_BYTES = 0;
    uint8_t SCRAMBLE_BIT = 0;

    uint16_t DESCRAMBLE_BYTES = 0;
    uint8_t DESCRAMBLE_BIT = 0;


public:
    uint8_t AX25_FLAG = 0x7E;

    uint8_t scrambleBit(uint8_t bit);
    uint8_t descrambleBit(uint8_t bit);

    uint8_t scrambleByte(uint8_t bit);
    uint8_t descrambleByte(uint8_t bit);
};

#endif
