#include "DSerial.h"
#include "CLTUPacket.h"

#ifndef AX25FRAME_H_
#define AX25FRAME_H_

#define MAX_PACKET_SIZE 256-18
#define AX25_CRC        0x8408




class AX25Frame
{
protected:
    //uint8_t controlField;
    //uint8_t PIDField           =   0xF0;
    //uint8_t addressField[14];

    //uint8_t packetField[MAX_PACKET_SIZE]; //'info' field

public:
     static unsigned char reverseByteOrder(unsigned char x);
     static void setAdress(CLTUPacket packet, uint8_t Destination[], uint8_t Source[]);
     static void setControl(CLTUPacket packet, bool PF);
     static void setControl(CLTUPacket packet, uint8_t byte);
     static void setPID(CLTUPacket packet, uint8_t byte);
     static void setFCS(CLTUPacket packet, uint8_t FCS[]);

     static void setInfoPacket(CLTUPacket packet, uint8_t data[], uint8_t size);
     static void setPacket(CLTUPacket packet, uint8_t data[], uint8_t size);
     static void calculateFCS(CLTUPacket packet);
     static bool checkFCS(CLTUPacket packet);

     static uint8_t getPacketSize(CLTUPacket packet);
};

#endif
