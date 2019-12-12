#include "APSynchronizer.h"

extern DSerial serial;


int APSynchronizer::mod(int a, int b)
{ return (a%b+b)%b; }



APSynchronizer::APSynchronizer(AX25Frame AX25FrameBuffer[], int &AX25RXframesInBuffer,  int &AX25RXbufferIndex){
    this->receivedFrameBuffer = AX25FrameBuffer;
    this->AX25RXframesInBuffer = &AX25RXframesInBuffer;
    this->AX25RXbufferIndex = &AX25RXbufferIndex;
}

bool APSynchronizer::compareBitArrays(uint8_t array1[], uint8_t array2[], uint8_t size){
    bool out = true;
    for(int i = 0; i < size; i++){
        if(array1[i] != array2[i]){
            out = false;
        }
    }
    return out;
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
        //serial.println("[!!]");
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
        bitBuffer[byteBufferIndex] = inBit;
        bitCounter++;
        //for(int i = 7; i >= 0; i--){
        //    serial.print(byteBuffer[(byteBufferIndex - i)%BYTE_BUFFER_SIZE], DEC);
        //}
        //serial.println();
        if(     bitBuffer[mod(byteBufferIndex - 7, AP_BYTE_BUFFER_SIZE)] == 0 &&
                bitBuffer[mod(byteBufferIndex - 6, AP_BYTE_BUFFER_SIZE)] == 1 &&
                bitBuffer[mod(byteBufferIndex - 5, AP_BYTE_BUFFER_SIZE)] == 1 &&
                bitBuffer[mod(byteBufferIndex - 4, AP_BYTE_BUFFER_SIZE)] == 1 &&
                bitBuffer[mod(byteBufferIndex - 3, AP_BYTE_BUFFER_SIZE)] == 1 &&
                bitBuffer[mod(byteBufferIndex - 2, AP_BYTE_BUFFER_SIZE)] == 1 &&
                bitBuffer[mod(byteBufferIndex - 1, AP_BYTE_BUFFER_SIZE)] == 1 &&
                bitBuffer[mod(byteBufferIndex - 0, AP_BYTE_BUFFER_SIZE)] == 0
            ){
            //last received bit completed a flag, the tail of a transfer exists of flags, hence check byteBuffer for packet;
            //minimum frame length is 4 bytes, maximum bits is decided by Buffer.

            if(bitCounter > 8*(14+2+2) && bitCounter < 8*(256)){

                //start destuffing bits:
                int destuffIndex = 0;
                int destuffBitIndex = 0;
                int destuffCount = 0;
                int destuffs = 0;

                //destuff Bits and Fix Ordering (every octet is received LSB first).
                this->destuffedBitBuffer[0] = 0;
                for(int k = 0; k < bitCounter - 8; k++){
                    this->destuffedBitBuffer[destuffIndex] |= ((bitBuffer[mod(byteBufferIndex - bitCounter + 1 + k, AP_BYTE_BUFFER_SIZE)] & 0x01) << destuffBitIndex);
                    //serial.print(destuffBuffer[destuffIndex], HEX);
                    //serial.print(destuffIndex + destuffBitIndex, DEC);
                    //serial.println();
                    if(bitBuffer[mod(byteBufferIndex - bitCounter + 1 + k, AP_BYTE_BUFFER_SIZE)] == 0x01){
                        destuffCount++;
                        //serial.print("1");
                    }
                    if(bitBuffer[mod(byteBufferIndex - bitCounter + 1 + k, AP_BYTE_BUFFER_SIZE)] == 0x00){
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
                        for(int iter = 0; iter < packetBits/8; iter++){
                            serial.print(this->destuffedBitBuffer[iter], HEX);
                            serial.print("|");
                        }
                        serial.println();
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
        if(this->byteBitBufferIndex > 7){
            this->byteBitBufferIndex = 0;
            byteBufferIndex = mod(byteBufferIndex + 1, AP_BYTE_BUFFER_SIZE);
        }
    }

    return packetReceived;
}
