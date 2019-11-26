#include "DSerial.h"

#ifndef AX25FRAME_H_
#define AX25FRAME_H_

#define MAX_PACKET_SIZE 100
#define AX25_CRC        0x8408




class AX25Frame
{
protected:
    uint8_t controlField;
    uint8_t PIDField           =   0xF0;
    uint8_t addressField[14];

    uint8_t packetField[MAX_PACKET_SIZE]; //'info' field
    uint8_t packetSize;

    uint8_t FCSBuffer[17];
    uint8_t FrameBytes[MAX_PACKET_SIZE+14+4];
    uint8_t FrameSize;

    uint8_t crc16[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0};

    void updateCRCByte(uint8_t inByte);

public:
     uint16_t FCSField;
     unsigned char reverseByteOrder(unsigned char x);
     void setAdress(uint8_t Destination[], uint8_t Source[]);
     void setControl(bool PF);
     void setControl(uint8_t byte);
     void setPID(uint8_t byte);

     void setPacket(uint8_t packet[], uint8_t size);
     void setData(uint8_t data[], uint8_t size);
     void calculateFCS();

     uint8_t* getBytes();
     uint8_t getSize();

};

#endif
