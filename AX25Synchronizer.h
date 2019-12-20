#include "DSerial.h"
#include "AX25Frame.h"
#include "AX25Encoder.h"
#include "CLTUPacket.h"

#ifndef AX25SYNC_H_
#define AX25SYNC_H_

#define BYTE_BUFFER_SIZE    1024
#define BYTE_QUE_SIZE       512

class AX25Synchronizer
{
protected:
    uint8_t bitBuffer[BYTE_BUFFER_SIZE];
    uint8_t byteQue[BYTE_BUFFER_SIZE];
    int byteQueIndex = 0;

    int byteBufferIndex = 0;
    int bitBufferIndex = 0;
    int bitCounter = 0;

    bool compareBitArrays(uint8_t array1[], uint8_t array[2], uint8_t size);
    CLTUPacket* receivedFrameBuffer;
    int* AX25RXbufferIndex;

    int mod(int a, int b);
    int clip(int a, int b);

    AX25Encoder encoder;

public:
    AX25Synchronizer(CLTUPacket AX25FrameBuffer[], int &AX25RXbufferIndex);

    //AX25Frame receivedFrame;
    bool queByte(uint8_t byte);
    bool rxBit();
    volatile bool hasReceivedFrame = false;
    int bytesInQue = 0;
};

#endif
