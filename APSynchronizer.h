/*
 * APSynchronizer.h
 *
 *  Created on: 12 Dec 2019
 *      Author: Casper Broekhuizen
 */
#include <CLTUPacket.h>
#include "DSerial.h"
#include "BitArray.h"
#include "AX25Frame.h"
#include "AX25Encoder.h"
#include "LDPC_decoder.h"

#ifndef APSYNC_H_
#define APSYNC_H_
#define AX25_RX_FRAME_BUFFER 10

#define AP_BYTE_BUFFER_SIZE    256
#define AP_BYTE_QUE_SIZE       1024

class APSynchronizer
{
protected:
    uint8_t byteQue[AP_BYTE_QUE_SIZE];
    int byteQueIndex = 0;

    uint8_t bitBuffer[AP_BYTE_BUFFER_SIZE];

    uint8_t destuffedBitBuffer[AP_BYTE_BUFFER_SIZE];

    int bitBufferIndex = 0;
    int bitCounter = 0;

    uint8_t APBitBuffer[AP_BYTE_BUFFER_SIZE];
    int APBitBufferIndex = 0;
    int synchronizerState = 1;
    int allowedSeqError = 3;
    //0 = Inactive, 1 = Searching, 2 = Reading Pilot CLTU, 3 = obtaining CLTUs
    uint8_t pilotCLTU[64];
    int CLTUbitCounter = 0;
    int incomingCLTUs = 0;
    bool pilotReceived = false;

    CLTUPacket* rxCLTU;
    int* rxCLTUInBuffer;
    int* rxCLTUBufferIndex;

    int mod(int a, int b);

    AX25Encoder encoder;


public:
    APSynchronizer(CLTUPacket AX25FrameBuffer[], int &AX25RXframesInBuffer, int &AX25RXbufferIndex);

    //AX25Frame receivedFrame;
    bool queByte(uint8_t byte);
    bool rxBit();
    volatile bool hasReceivedFrame = false;
    int bytesInQue = 0;
};

#endif
