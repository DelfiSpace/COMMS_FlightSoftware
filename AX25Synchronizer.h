#include "PQPacket.h"
#include "AX25Frame.h"
#include "AX25Encoder.h"
#include "BitArray.h"
#include "Console.h"

#ifndef AX25SYNC_H_
#define AX25SYNC_H_

#define BYTE_BUFFER_SIZE    PQPACKET_MAX_SIZE*2
#define BYTE_QUE_SIZE       2048

class AX25Synchronizer
{
protected:
    uint8_t flagBuffer = 0;
    int flagBufferIndex = 0;

    uint8_t bitBuffer[BYTE_BUFFER_SIZE];
    int bitBufferIndex = 0;

    uint8_t byteQue[BYTE_QUE_SIZE];
    int byteQueWriteIndex = 1;
    int byteQueReadIndex = 0;


    int synchronizerState = 0;

    int mod(int a, int b);

    AX25Encoder encoder;

public:
    AX25Synchronizer();

    bool queByte(uint8_t byte);
    bool rxBit();
    int bytesInQue();

    PQPacket rcvdFrame;
};

#endif
