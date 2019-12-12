/*
 * APSynchronizer.cpp
 *
 *  Created on: 12 Dec 2019
 *      Author: Casper Broekhuizen
 */
#include "APSynchronizer.h"

extern DSerial serial;

int APSynchronizer::mod(int a, int b)
{ return (a%b+b)%b; }



APSynchronizer::APSynchronizer(CLTUPacket rxCLTU[], int &rxCLTUInBuffer,  int &rxCLTUBufferIndex){
    this->rxCLTU = rxCLTU;
    this->rxCLTUInBuffer = &rxCLTUInBuffer;
    this->rxCLTUBufferIndex = &rxCLTUBufferIndex;
}


//void APSynchronizer::queBit(uint8_t inBit){
//    bitBuffer[byteBufferIndex] = inBit;
//    byteBufferIndex = (byteBufferIndex + 1)%AP_BYTE_BUFFER_SIZE;
//}
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
        //serial.print(byteBufferIndex);
        uint8_t inBit = encoder.NRZIdecodeBit((inByte >> (7-i))& 0x01);
        inBit = encoder.descrambleBit(inBit);
        BitArray::setBit(bitBuffer, bitBufferIndex, inBit == 0x01);
        bitCounter++;

        if(     BitArray::getBit(bitBuffer, mod(bitBufferIndex - 7, 8*AP_BYTE_BUFFER_SIZE)) == 0 &&
                BitArray::getBit(bitBuffer, mod(bitBufferIndex - 6, 8*AP_BYTE_BUFFER_SIZE)) == 1 &&
                BitArray::getBit(bitBuffer, mod(bitBufferIndex - 5, 8*AP_BYTE_BUFFER_SIZE)) == 1 &&
                BitArray::getBit(bitBuffer, mod(bitBufferIndex - 4, 8*AP_BYTE_BUFFER_SIZE)) == 1 &&
                BitArray::getBit(bitBuffer, mod(bitBufferIndex - 3, 8*AP_BYTE_BUFFER_SIZE)) == 1 &&
                BitArray::getBit(bitBuffer, mod(bitBufferIndex - 2, 8*AP_BYTE_BUFFER_SIZE)) == 1 &&
                BitArray::getBit(bitBuffer, mod(bitBufferIndex - 1, 8*AP_BYTE_BUFFER_SIZE)) == 1 &&
                BitArray::getBit(bitBuffer, mod(bitBufferIndex - 0, 8*AP_BYTE_BUFFER_SIZE)) == 0
            ){
            //last received bit completed a flag, the tail of a transfer exists of flags, hence check byteBuffer for packet;
            //minimum frame length is 4 bytes, maximum bits is decided by Buffer.

            if(bitCounter > 8*(18) && bitCounter < 8*(256)){

                //start destuffing bits:
                int destuffIndex = 0;
                int destuffBitIndex = 0;
                int destuffCount = 0;
                int destuffs = 0;

                //destuff Bits and Fix Ordering (every octet is received LSB first).
                this->destuffedBitBuffer[0] = 0;
                for(int k = 0; k < bitCounter - 8; k++){
                    uint8_t curBit = BitArray::getBit(bitBuffer, mod(bitBufferIndex - bitCounter + 1 + k, 8*AP_BYTE_BUFFER_SIZE));
                    this->destuffedBitBuffer[destuffIndex] |= curBit << destuffBitIndex;
                    //serial.print(destuffBuffer[destuffIndex], HEX);
                    //serial.print(destuffIndex + destuffBitIndex, DEC);
                    //serial.println();
                    if(curBit == 0x01){
                        destuffCount++;
                        //serial.print("1");
                    }
                    if(curBit == 0x00){
                        destuffCount = 0;
                        //serial.print("0");
                    }
                    if(destuffCount == 5){
                        destuffCount = 0;
                        destuffs++;;
                        k = k + 1; //skip next bit.
                    }
                    destuffBitIndex++;
                    if(destuffBitIndex >= 8){
                        destuffBitIndex = 0;
                        destuffIndex++;
                        this->destuffedBitBuffer[destuffIndex] = 0;
                    }
                }
                int packetBits = bitCounter - destuffs - 8;

                if(mod(packetBits, 8) == 0 ){//&& destuffedBitBuffer[0] == 0x82){ // 'correct' packets are always whole bytes
                    //
                    //
                    //
                    //
                    //
                    if(this->destuffedBitBuffer[0] == 0xAA && this->destuffedBitBuffer[1] == 0xAA){
                        for(int iter = 0; iter < packetBits-16; iter++){
                            uint8_t inBit = BitArray::getBit(destuffedBitBuffer, iter);
                            BitArray::setBit(APBitBuffer, APBitBufferIndex, inBit == 0x01);
                            switch(synchronizerState){
                                case 0 : //Inactive
                                    break;
                                case 1 : //Searching/Waiting for Start-Seq
                                    if(pilotReceived){
                                        pilotReceived = false;
                                    }
                                    //check for start seq: 1110 1011 1001 0000
                                    int matchCoeff = 0;
                                    matchCoeff +=  (BitArray::getBit(APBitBuffer, mod(APBitBufferIndex - 15, 8*AP_BYTE_BUFFER_SIZE)) == 1) ? 1 : 0;
                                    matchCoeff +=  (BitArray::getBit(APBitBuffer, mod(APBitBufferIndex - 14, 8*AP_BYTE_BUFFER_SIZE)) == 1) ? 1 : 0;
                                    matchCoeff +=  (BitArray::getBit(APBitBuffer, mod(APBitBufferIndex - 13, 8*AP_BYTE_BUFFER_SIZE)) == 1) ? 1 : 0;
                                    matchCoeff +=  (BitArray::getBit(APBitBuffer, mod(APBitBufferIndex - 12, 8*AP_BYTE_BUFFER_SIZE)) == 0) ? 1 : 0;
                                    matchCoeff +=  (BitArray::getBit(APBitBuffer, mod(APBitBufferIndex - 11, 8*AP_BYTE_BUFFER_SIZE)) == 1) ? 1 : 0;
                                    matchCoeff +=  (BitArray::getBit(APBitBuffer, mod(APBitBufferIndex - 10, 8*AP_BYTE_BUFFER_SIZE)) == 0) ? 1 : 0;
                                    matchCoeff +=  (BitArray::getBit(APBitBuffer, mod(APBitBufferIndex - 9, 8*AP_BYTE_BUFFER_SIZE)) == 1) ? 1 : 0;
                                    matchCoeff +=  (BitArray::getBit(APBitBuffer, mod(APBitBufferIndex - 8, 8*AP_BYTE_BUFFER_SIZE)) == 1) ? 1 : 0;
                                    matchCoeff +=  (BitArray::getBit(APBitBuffer, mod(APBitBufferIndex - 7, 8*AP_BYTE_BUFFER_SIZE)) == 1) ? 1 : 0;
                                    matchCoeff +=  (BitArray::getBit(APBitBuffer, mod(APBitBufferIndex - 6, 8*AP_BYTE_BUFFER_SIZE)) == 0) ? 1 : 0;
                                    matchCoeff +=  (BitArray::getBit(APBitBuffer, mod(APBitBufferIndex - 5, 8*AP_BYTE_BUFFER_SIZE)) == 0) ? 1 : 0;
                                    matchCoeff +=  (BitArray::getBit(APBitBuffer, mod(APBitBufferIndex - 4, 8*AP_BYTE_BUFFER_SIZE)) == 1) ? 1 : 0;
                                    matchCoeff +=  (BitArray::getBit(APBitBuffer, mod(APBitBufferIndex - 3, 8*AP_BYTE_BUFFER_SIZE)) == 0) ? 1 : 0;
                                    matchCoeff +=  (BitArray::getBit(APBitBuffer, mod(APBitBufferIndex - 2, 8*AP_BYTE_BUFFER_SIZE)) == 0) ? 1 : 0;
                                    matchCoeff +=  (BitArray::getBit(APBitBuffer, mod(APBitBufferIndex - 1, 8*AP_BYTE_BUFFER_SIZE)) == 0) ? 1 : 0;
                                    matchCoeff +=  (BitArray::getBit(APBitBuffer, mod(APBitBufferIndex - 0, 8*AP_BYTE_BUFFER_SIZE)) == 0) ? 1 : 0;
                                    if(matchCoeff >= 16 - allowedSeqError){
                                        serial.println("START SEQ DETECTED!");
                                        this->synchronizerState = 2;
                                    }
                                    break;
                                case 2 : //Reading CLTUs
                                    //test case: read 1 pilot CLTU
//                                    serial.print(APBitBufferIndex, DEC);
//                                    serial.print(" - ");
//                                    serial.print(CLTUIndex, DEC);
//                                    serial.println();
                                    if(!pilotReceived){
                                        BitArray::setBit(pilotCLTU, CLTUbitCounter, BitArray::getBit(APBitBuffer, APBitBufferIndex) == 0x01);
                                        CLTUbitCounter += 1;
                                        if(CLTUbitCounter >= 8*64){
                                            CLTUbitCounter = 0;
                                            serial.println("PILOT SEQUENCE RECEIVED!");
                                            pilotReceived = true;
                                            bool decoded = false;
                                            for(int decoder_iter = 0; decoder_iter<10; decoder_iter++){
                                                if(LDPCDecoder::iterateBitflip(pilotCLTU)){
                                                    serial.print("LDPC Iterations:  ");
                                                    serial.print(decoder_iter, DEC);
                                                    serial.println();
                                                    decoded = true;
                                                    break;
                                                }
                                            }
                                            if(decoded){
                                                incomingCLTUs = pilotCLTU[0];
                                                for(int w = 0; w < 32; w++){
                                                    serial.print(pilotCLTU[w], HEX);
                                                    serial.print("|");
                                                }
                                            }else{
                                                serial.print("!! PILOT DECODING FAILED !!");
                                                this->synchronizerState = 1;
                                            }
                                            serial.println();
                                        }
                                    }else if(incomingCLTUs > 0){
                                        BitArray::setBit(this->rxCLTU[*rxCLTUBufferIndex].data, CLTUbitCounter, BitArray::getBit(APBitBuffer, APBitBufferIndex) == 0x01);
                                        CLTUbitCounter++;
                                        if(CLTUbitCounter >= 8*64){
                                            serial.print("receivedCLTU: ");
                                            serial.print(incomingCLTUs, DEC);
                                            serial.println();
                                            CLTUbitCounter = 0;
                                            *rxCLTUBufferIndex = mod(*rxCLTUBufferIndex + 1, 20);
                                            *rxCLTUInBuffer = *rxCLTUInBuffer+1;
                                            incomingCLTUs--;
                                        }
                                    }else{
                                        this->synchronizerState = 1;
                                    }
                                    break;
                                default : //should not happen
                                    this->synchronizerState = 0;
                                    break;
                            }
                            //Detect Start Sequence:
//                            serial.print("APBitBufferIndex: ");
//                            serial.print(APBitBufferIndex);
//                            serial.print("  :  ");
//                            serial.print(BitArray::getBit(APBitBuffer, APBitBufferIndex), HEX);
                            APBitBufferIndex = mod(APBitBufferIndex + 1, 8 * AP_BYTE_BUFFER_SIZE);

                        }

                    }








                    //
                    //
                    //
                    //
                    //
                    //
                    //
                    //

                }
                bitCounter = 0;
            }else{
                bitCounter = 0;
            }
        }
        bitBufferIndex = mod(bitBufferIndex + 1, 8* AP_BYTE_BUFFER_SIZE);

    }

    return packetReceived;
}
