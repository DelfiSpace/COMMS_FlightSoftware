#include "DSerial.h"
#include "CLTUPacket.h"

#ifndef AX25FRAME_H_
#define AX25FRAME_H_

#define MAX_PACKET_SIZE 256-18
#define AX25_CRC        0x8408




class AX25Frame
{
protected:

public:
     static unsigned char reverseByteOrder(unsigned char x);
     static void setAdress(CLTUPacket &inPacket, uint8_t Destination[], uint8_t Source[]);
     static void setControl(CLTUPacket &inPacket, bool PF);
     static void setControl(CLTUPacket &inPacket, uint8_t byte);
     static void setPID(CLTUPacket &inPacket, uint8_t byte);
     static void setFCS(CLTUPacket &inPacket, uint8_t FCS[]);

     static void setPacket(CLTUPacket &inPacket, uint8_t packet[], uint8_t size);
     static void setData(CLTUPacket &inPacket, uint8_t data[], uint8_t size);
     static void calculateFCS(CLTUPacket &inPacket);
     static bool checkFCS(CLTUPacket &inPacket);
     static bool checkFCS(CLTUPacket &inPacket, int size);

     static uint8_t getPacketSize(CLTUPacket &inPacket);
};

#endif
