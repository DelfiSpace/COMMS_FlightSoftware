#include "AX25Synchronizer.h"

extern DSerial serial;


int AX25Synchronizer::mod(int a, int b)
{ return (a%b+b)%b; }



AX25Synchronizer::AX25Synchronizer(AX25Frame AX25FrameBuffer[], int &AX25RXframesInBuffer,  int &AX25RXbufferIndex){
    this->receivedFrameBuffer = AX25FrameBuffer;
    this->AX25RXframesInBuffer = &AX25RXframesInBuffer;
    this->AX25RXbufferIndex = &AX25RXbufferIndex;
}

bool AX25Synchronizer::compareBitArrays(uint8_t array1[], uint8_t array2[], uint8_t size){
    bool out = true;
    for(int i = 0; i < size; i++){
        if(array1[i] != array2[i]){
            out = false;
        }
    }
    return out;
}

//void AX25Synchronizer::queBit(uint8_t inBit){
//    bitBuffer[byteBufferIndex] = inBit;
//    byteBufferIndex = (byteBufferIndex + 1)%BYTE_BUFFER_SIZE;
//}
bool AX25Synchronizer::queByte(uint8_t inByte){
    byteQue[this->byteQueIndex] = inByte;
    byteQueIndex = mod(byteQueIndex+1, BYTE_QUE_SIZE);
    bytesInQue = bytesInQue + 1;

    if(bytesInQue > BYTE_QUE_SIZE){
        serial.println("[!!]");
    }

    return true;
}

bool AX25Synchronizer::rxBit(){
    bool packetReceived = false;
    if(bytesInQue <= 0){
        return packetReceived;
    }
    uint8_t inByte = byteQue[mod(byteQueIndex - bytesInQue, BYTE_QUE_SIZE)];
    bytesInQue = bytesInQue - 1;

    for(int i = 0; i < 8; i++){
        uint8_t inBit = encoder.NRZIdecodeBit((inByte >> (7-i))& 0x01);
        inBit = encoder.descrambleBit(inBit);
        bitBuffer[bitBufferIndex/8] |= inBit << (7-(bitBufferIndex%8));
        bitCounter++;
        //for(int i = 7; i >= 0; i--){
        //    serial.print(byteBuffer[(byteBufferIndex - i)%BYTE_BUFFER_SIZE], DEC);
        //}
        //serial.println();
        if(     ((bitBuffer[mod((bitBufferIndex-7)/8,BYTE_BUFFER_SIZE)] >> (7-mod(bitBufferIndex-7,8))) & 0x01) == 0 &&
                ((bitBuffer[mod((bitBufferIndex-6)/8,BYTE_BUFFER_SIZE)] >> (7-mod(bitBufferIndex-6,8))) & 0x01) == 1 &&
                ((bitBuffer[mod((bitBufferIndex-5)/8,BYTE_BUFFER_SIZE)] >> (7-mod(bitBufferIndex-5,8))) & 0x01) == 1 &&
                ((bitBuffer[mod((bitBufferIndex-4)/8,BYTE_BUFFER_SIZE)] >> (7-mod(bitBufferIndex-4,8))) & 0x01) == 1 &&
                ((bitBuffer[mod((bitBufferIndex-3)/8,BYTE_BUFFER_SIZE)] >> (7-mod(bitBufferIndex-3,8))) & 0x01) == 1 &&
                ((bitBuffer[mod((bitBufferIndex-2)/8,BYTE_BUFFER_SIZE)] >> (7-mod(bitBufferIndex-2,8))) & 0x01) == 1 &&
                ((bitBuffer[mod((bitBufferIndex-1)/8,BYTE_BUFFER_SIZE)] >> (7-mod(bitBufferIndex-1,8))) & 0x01) == 1 &&
                ((bitBuffer[mod((bitBufferIndex-0)/8,BYTE_BUFFER_SIZE)] >> (7-mod(bitBufferIndex-0,8))) & 0x01) == 0
            ){
            //last received bit completed a flag, the tail of a transfer exists of flags, hence check byteBuffer for packet;
            //minimum frame length is 4 bytes, maximum bits is decided by Buffer.

            if(bitCounter > 8*(14+2+2) && bitCounter < 8*(50)){

                //start destuffing bits:
                int destuffIndex = 0;
                int destuffBitIndex = 0;
                int destuffCount = 0;
                int destuffs = 0;

                //destuff Bits and Fix Ordering (every octet is received LSB first).
                this->receivedFrameBuffer[*AX25RXbufferIndex].FrameBytes[0] = 0;
                for(int k = 0; k < bitCounter - 8; k++){
                    this->receivedFrameBuffer[*AX25RXbufferIndex].FrameBytes[destuffIndex] |= (((bitBuffer[mod((bitBufferIndex- bitCounter + 1 + k)/8,BYTE_BUFFER_SIZE)] >> (7-mod(bitBufferIndex- bitCounter + 1 + k,8))) & 0x01) << destuffBitIndex);
                    //serial.print(destuffBuffer[destuffIndex], HEX);
                    //serial.print(destuffIndex + destuffBitIndex, DEC);
                    //serial.println();
                    if(((bitBuffer[mod((bitBufferIndex- bitCounter + 1 + k)/8,BYTE_BUFFER_SIZE)] >> (7-mod(bitBufferIndex- bitCounter + 1 + k,8))) & 0x01) == 0x01){
                        destuffCount++;
                        //serial.print("1");
                    }
                    if(((bitBuffer[mod((bitBufferIndex- bitCounter + 1 + k)/8,BYTE_BUFFER_SIZE)] >> (7-mod(bitBufferIndex- bitCounter + 1 + k,8))) & 0x01) == 0x00){
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
                        this->receivedFrameBuffer[*AX25RXbufferIndex].FrameBytes[destuffIndex] = 0;
                    }
                }
                int packetBits = bitCounter - destuffs - 8;

                if(packetBits % 8 == 0 && receivedFrameBuffer[*AX25RXbufferIndex].FrameBytes[0] == 0x82){ // 'correct' packets are always whole bytes
                    receivedFrameBuffer[*AX25RXbufferIndex].FrameSize = packetBits/8;
                    if(this->receivedFrameBuffer[*AX25RXbufferIndex].checkFCS()){
                        this->hasReceivedFrame = true;
                        packetReceived = true;
                        //serial.println("!");
                        *AX25RXbufferIndex = ( *AX25RXbufferIndex + 1 ) % AX25_RX_FRAME_BUFFER;
                        if(*AX25RXframesInBuffer < AX25_RX_FRAME_BUFFER){
                            *AX25RXframesInBuffer = *AX25RXframesInBuffer + 1;
                        }
                        serial.print(*AX25RXframesInBuffer, DEC);
                        //serial.print("  -  ");
                        //serial.print(*AX25RXbufferIndex, DEC);
                        serial.println();
                    }
                }
                bitCounter = 0;
            }else{
                bitCounter = 0;
            }
        }

        bitBufferIndex = mod((bitBufferIndex + 1),(8*BYTE_BUFFER_SIZE));
    }

    return packetReceived;
}
