/*
 * APSynchronizer.h
 *
 *  Created on: 12 Dec 2019
 *      Author: Casper Broekhuizen
 */
#include <CLTUPacket.h>
#include <LDPCDecoder.h>
#include "BitArray.h"
#include "Console.h"
#include "AX25Frame.h"
#include "AX25Encoder.h"

#ifndef APSYNC_H_
#define APSYNC_H_
#define AX25_RX_FRAME_BUFFER 10

#define AP_BYTE_BUFFER_SIZE    8
#define AP_BYTE_QUE_SIZE       1024

class APSynchronizer
{
protected:
    uint8_t byteQue[AP_BYTE_QUE_SIZE];
    int byteQueIndex = 0;

    uint8_t tailSeq[2] = {0xEB, 0x90};
    uint8_t startSeq[8] = {0x00, 0x00, 0xEB, 0x90, 0x00, 0x00, 0xEB, 0x90};

    uint8_t flagDetectBuffer[64];
    int flagDetectBitIndex = 0;

    int synchronizerState = 1;
    int allowedSeqError = 2;
    int incomingCLTUs = 0;

    int CLTUbitCounter = 0;

    CLTUPacket* rxCLTU;
    int* rxCLTUBufferIndex;

    int mod(int a, int b);
    int clip(int a, int b);

    AX25Encoder encoder;


public:
    APSynchronizer(CLTUPacket AX25FrameBuffer[], int &AX25RXbufferIndex);

    //AX25Frame receivedFrame;
    bool queByte(uint8_t byte);
    bool rxBit();
    volatile bool hasReceivedFrame = false;
    int bytesInQue = 0;
};

#endif
