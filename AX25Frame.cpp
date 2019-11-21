#include "AX25Frame.h"

extern DSerial serial;

void AX25Frame::setAdress(uint8_t Destination[], uint8_t Source[]){
    for(int i = 0; i < 7; i++){
        this->addressField[i] = Destination[i];
        this->FrameBytes[i] = Destination[i];
    }
    for(int i = 0; i < 7; i++){
        this->addressField[7+i] = Source[i];
        this->FrameBytes[7+i] = Source[i];
    }

};

void AX25Frame::setControl(bool PF){
    if(PF){
        this->controlField = 0x13; //000 1 00 11
    }else{
        this->controlField = 0x03; //000 0 00 11
    }
    this->FrameBytes[14] = controlField;
};

void AX25Frame::setPacket(uint8_t packet[], uint8_t size){
    for(int i = 0; i < size; i++){
        this->packetField[i] = packet[i];
    }

    this->packetSize = size;
    this->FrameSize = size + 18;
};

void AX25Frame::calculateFCS(){
    //fcs poly: 1 0001 0000 0010 0001  (17bits);
    this->FCSField=0xFFFF;
    //Fill FCSBuffer with packet
    for(int i = 0; i < 14; i++){
        this->FCSBuffer[i] = this->addressField[i];

    }
    this->FCSBuffer[14] = this->controlField;
    this->FCSBuffer[14+1] = this->PIDField;
    this->FrameBytes[14] = this->controlField;
    this->FrameBytes[14+1] = this->PIDField;

    for(int i = 0; i < this->packetSize; i++){
        this->FCSBuffer[16+i] = this->packetField[i];
        this->FrameBytes[16+i] = this->packetField[i];
    }

    for(int i = 0; i < 2; i++){
            this->FCSBuffer[16+this->packetSize+i] = 0x00;
            this->FrameBytes[16+this->packetSize+i] = 0x00;
    }

    this->FrameSize = this->packetSize + 18;

    //calculate CRC
    for(int i = 0; i < ((this->FrameSize - 2 )); i++){
        updateCRCByte(FCSBuffer[i]);
    }

    this->FCSBuffer[16+this->packetSize+1] = this->FCSField & 0xFF;
    this->FCSBuffer[16+this->packetSize] = (this->FCSField >> 8);
    this->FrameBytes[16+this->packetSize+1] = this->FCSField & 0xFF;
    this->FrameBytes[16+this->packetSize] = (this->FCSField >> 8);

};

void AX25Frame::updateCRCByte(uint8_t inByte){

    FCSField ^= ((uint16_t)inByte << 8);
    for(uint8_t i = 0; i < 8; i++){
        if (FCSField & 0x8000){
            FCSField = (FCSField << 1) ^ AX25_CRC;
        }else{
            FCSField <<= 1;
        }
    }
}

uint8_t * AX25Frame::getBytes(){
    return this->FrameBytes;
};

uint8_t AX25Frame::getSize(){
    return this->FrameSize;
}
