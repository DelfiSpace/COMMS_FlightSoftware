#include <PQPacket.h>
#include "AX25Frame.h"
#include "AX25Encoder.h"
#include "BitArray.h"
#include "Console.h"

#ifndef AX25SYNC_H_
#define AX25SYNC_H_

#define BYTE_BUFFER_SIZE    1024
#define BYTE_QUE_SIZE       2048

class AX25Synchronizer
{
protected:
    uint8_t flagBuffer = 0;
    int flagBufferIndex = 0;

    uint8_t bitBuffer[BYTE_BUFFER_SIZE];
    int bitBufferIndex = 0;

    uint8_t byteQue[BYTE_QUE_SIZE];
    int byteQueIndex = 0;

    int synchronizerState = 0;

    PQPacket receivedFrameBuffer;

    AX25Encoder encoder;

public:
    AX25Synchronizer();

    //AX25Frame receivedFrame;
    bool queByte(uint8_t byte);
    PQPacket* rxBit();
    int bytesInQue = 0;
};

#endif
