#include "DSerial.h"
#include "AX25Frame.h"

#ifndef AX25SYNC_H_
#define AX25SYNC_H_

#define BYTE_BUFFER_SIZE   2048

class AX25Synchronizer
{
protected:
    uint8_t FlagByte[8] = {0,1,1,1,1,1,1,0};
    uint8_t crc16[17] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1};
    uint8_t bitBuffer[BYTE_BUFFER_SIZE];
    uint8_t destuffBuffer[BYTE_BUFFER_SIZE];
    int byteBufferIndex = 0;
    int bitCounter = 0;

    bool compareBitArrays(uint8_t array1[], uint8_t array[2], uint8_t size);

public:
    AX25Frame receivedFrame;
    bool rxBit(uint8_t inBit);
    volatile bool hasReceivedFrame = false;
};

#endif
