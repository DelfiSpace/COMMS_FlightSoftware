#include "SX1276.h"
#include "DSPI.h"
#include "sx1276Enums.h"
#include "DSerial.h"

#ifndef AX25FRAME_H_
#define AX25FRAME_H_

#define MAX_PACKET_SIZE 100
#define AX25_CRC        0x1021

class AX25Frame
{
protected:
    uint8_t controlField;
    uint8_t PIDField           =   0xF0;
    uint16_t FCSField;
    uint8_t addressField[14];

    uint8_t packetField[MAX_PACKET_SIZE]; //'info' field
    uint8_t packetSize;

    uint8_t FCSBuffer[MAX_PACKET_SIZE+14+4];
    uint8_t FrameBytes[MAX_PACKET_SIZE+14+4];
    uint8_t FrameSize;

    void AX25Frame::updateCRCByte(uint8_t inByte);

public:

     void setAdress(uint8_t Destination[], uint8_t Source[]);
     void setControl(bool PF);
     void setPacket(uint8_t packet[], uint8_t size);
     void calculateFCS();

     uint8_t* getBytes();
     uint8_t getSize();

};

#endif
