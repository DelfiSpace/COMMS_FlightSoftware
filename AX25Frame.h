#include "DSerial.h"

#ifndef AX25FRAME_H_
#define AX25FRAME_H_

#define MAX_PACKET_SIZE 100
#define AX25_CRC        0x8408




class AX25Frame
{
protected:
    //uint8_t controlField;
    //uint8_t PIDField           =   0xF0;
    //uint8_t addressField[14];

    //uint8_t packetField[MAX_PACKET_SIZE]; //'info' field

public:
     //uint16_t FCSField;
     uint8_t FrameBytes[MAX_PACKET_SIZE+18];
     uint8_t FrameSize;
     //uint8_t packetSize;

     unsigned char reverseByteOrder(unsigned char x);
     void setAdress(uint8_t Destination[], uint8_t Source[]);
     void setControl(bool PF);
     void setControl(uint8_t byte);
     void setPID(uint8_t byte);
     void setFCS(uint8_t FCS[]);

     void setPacket(uint8_t packet[], uint8_t size);
     void setData(uint8_t data[], uint8_t size);
     void calculateFCS();
     void calculateFCS(uint8_t data[], uint8_t size);
     bool checkFCS();

     uint8_t* getBytes();
     uint8_t getSize();

};

#endif
