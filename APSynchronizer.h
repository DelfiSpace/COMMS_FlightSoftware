#include "DSerial.h"
#include "AX25Frame.h"
#include "AX25Encoder.h"

#ifndef APSYNC_H_
#define APSYNC_H_
#define AX25_RX_FRAME_BUFFER 10

#define AP_BYTE_BUFFER_SIZE    512
#define AP_BYTE_QUE_SIZE       1024

class APSynchronizer
{
protected:
    uint8_t byteQue[AP_BYTE_QUE_SIZE];
    int byteQueIndex = 0;

    uint8_t bitBuffer[AP_BYTE_BUFFER_SIZE];
    uint8_t destuffedBitBuffer[AP_BYTE_BUFFER_SIZE];

    int byteBufferIndex = 0;
    int bitBufferIndex = 0;
    int bitCounter = 0;

    uint8_t APBitBuffer[AP_BYTE_BUFFER_SIZE];
    int APBufferIndex;
    int APBufferBitIndex;



    bool compareBitArrays(uint8_t array1[], uint8_t array[2], uint8_t size);
    AX25Frame* receivedFrameBuffer;
    int* AX25RXframesInBuffer;
    int* AX25RXbufferIndex;

    int mod(int a, int b);

    AX25Encoder encoder;

public:
    APSynchronizer(AX25Frame AX25FrameBuffer[], int &AX25RXframesInBuffer, int &AX25RXbufferIndex);

    //AX25Frame receivedFrame;
    bool queByte(uint8_t byte);
    bool rxBit();
    volatile bool hasReceivedFrame = false;
    int bytesInQue = 0;
};

#endif
