#include <PQPacket.h>
#include "Console.h"
#include <stdint.h>

#ifndef AX25FRAME_H_
#define AX25FRAME_H_

#define MAX_PACKET_SIZE 256-18
#define AX25_CRC        0x8408




class AX25Frame
{
protected:

public:
     static unsigned char reverseByteOrder(unsigned char x);
     static void setAdress(PQPacket &inPacket, uint8_t Destination[], uint8_t Source[]);
     static void setControl(PQPacket &inPacket, bool PF);
     static void setControl(PQPacket &inPacket, uint8_t byte);
     static void setPID(PQPacket &inPacket, uint8_t byte);
     static void setFCS(PQPacket &inPacket, uint8_t FCS[]);

     static void setPacket(PQPacket &inPacket, uint8_t packet[], uint8_t size);
     static void setData(PQPacket &inPacket, uint8_t data[], uint8_t size);
     static void calculateFCS(PQPacket &inPacket);
     static bool checkFCS(PQPacket &inPacket);
     static bool checkFCS(PQPacket &inPacket, int size);

     static uint8_t getPacketSize(PQPacket &inPacket);
};

#endif
