/*
 * APSynchronizer.cpp
 *
 *  Created on: 12 Dec 2019
 *      Author: Casper Broekhuizen
 */
#include "APSynchronizer.h"
#include "COMMRadio.h"

int APSynchronizer::mod(int a, int b)
{ return a>0 ? (a%b) : (a%b+b)%b; }

int APSynchronizer::clip(int a, int b)
{ return a>b ? a-mod(a,b) : a; }




APSynchronizer::APSynchronizer(PQPacket rxCLTU[], int &rxCLTUBufferIndex){
    this->rxCLTU = rxCLTU;
    this->rxCLTUBufferIndex = &rxCLTUBufferIndex;
}

bool APSynchronizer::queByte(uint8_t inByte){
    byteQue[this->byteQueIndex] = inByte;
    byteQueIndex = mod(byteQueIndex+1, AP_BYTE_QUE_SIZE);
    bytesInQue = bytesInQue + 1;

    if(bytesInQue > AP_BYTE_QUE_SIZE){
        //serial.println("[!! ByteQue Overflow !!]");
    }

    return true;
}

bool APSynchronizer::rxBit(){
    bool packetReceived = false;
    if(bytesInQue <= 0){
        return packetReceived;
    }
    uint8_t inByte = byteQue[mod(byteQueIndex - bytesInQue, AP_BYTE_QUE_SIZE)];
    bytesInQue = bytesInQue - 1;

    for(int i = 0; i < 8; i++){
        uint8_t inBit = encoder.NRZIdecodeBit(BitArray::getBit(&inByte, i,8));
        inBit = encoder.descrambleBit(inBit);
        switch(synchronizerState){
            case 0 : //Inactive
                break;
            case 1 : //Searching/Waiting for Start-Seq
                flagDetectBuffer[flagDetectBitIndex] = inBit;
                flagDetectBitIndex = (flagDetectBitIndex+1)%64;

                int matchErrors = 0;
                for(int p = 0; p < 64; p++){
                    //serial.print(BitArray::getBit(flagDetectBuffer, this->mod(flagDetectBitIndex-64+p,64)), DEC);
                    //serial.print(BitArray::getBit(this->startSeq, p), DEC);
                    matchErrors += (flagDetectBuffer[this->mod(flagDetectBitIndex-64+p,64)] == BitArray::getBit(this->startSeq, p, 64)) ? 0 : 1;
                }
                //serial.println();
                if(matchErrors <= this->allowedSeqError){
                    Console::log("%d  START SEQ DETECTED!", matchErrors);
                    flagDetectBitIndex = 0;
                    this->synchronizerState = 2;
                }
                break;
            case 2 : //Reading CLTUs
                BitArray::setBit(this->rxCLTU[*rxCLTUBufferIndex].data, CLTUbitCounter, inBit);
                CLTUbitCounter++;

                if(CLTUbitCounter <= 8*8){
                    BitArray::setBit(flagDetectBuffer, flagDetectBitIndex, inBit);
                    flagDetectBitIndex = (flagDetectBitIndex+1)%64;
                }

                if(CLTUbitCounter == 8*8){
                    //Check for Tail Marker
                    int matchErrors = 0;
                    for(int p = 0; p < 16; p++){
                        //serial.print(BitArray::getBit(flagDetectBuffer, this->mod(flagDetectBitIndex-64+p,64)), DEC);
                        //serial.print(BitArray::getBit(this->startSeq, p), DEC);
                        matchErrors += (BitArray::getBit(flagDetectBuffer, flagDetectBitIndex-16+p,64) == BitArray::getBit(this->tailSeq, p, 16)) ? 0 : 1;
                    }
                    if(matchErrors <= this->allowedSeqError){
                        Console::log("%d  TAIL SEQ DETECTED!", matchErrors);
                        CLTUbitCounter = 0;
                        this->synchronizerState = 1;
                    }
                    flagDetectBitIndex = 0;
                }
                else if(CLTUbitCounter == 8*64){
                    //Save CLTU
                    CLTUbitCounter = 0;
                    rxCLTU[*rxCLTUBufferIndex].packetSize = 64;

//                    rxCLTU[*rxCLTUBufferIndex].isLocked = false;
//                    rxCLTU[*rxCLTUBufferIndex].isCoded = true;
                    rxCLTU[*rxCLTUBufferIndex].isReady = true;

                    Console::log("%d - %d", *rxCLTUBufferIndex, this->rxCLTU[*rxCLTUBufferIndex].packetSize);

                    *rxCLTUBufferIndex = mod(*rxCLTUBufferIndex + 1, RX_MAX_FRAMES);
//
//                    rxCLTU[*rxCLTUBufferIndex].isLocked = true;
//                    rxCLTU[*rxCLTUBufferIndex].isCoded = false;
                    rxCLTU[*rxCLTUBufferIndex].isReady = false;

                    incomingCLTUs++;
                    if(incomingCLTUs >= 5){
                        incomingCLTUs = 0;
                        this->synchronizerState = 1;
                    }

                }else if(CLTUbitCounter > 8*64){
                    Console::log("Impossible state, should not happen!");
                    CLTUbitCounter = 0;
                    this->synchronizerState = 1;
                }
                break;
            default : //should not happen
                this->synchronizerState = 1;
                break;
        }
    }

    return packetReceived;
}
