#include "SX1276.h"
#include "DSPI.h"
#include "sx1276Enums.h"
#include "DSerial.h"

#ifndef COMMRADIO_H_
#define COMMRADIO_H_
#define PACKET_SIZE    256
#define RF_MSG_SIZE    276

class AX25Frame
{
protected:
    uint8_t AX25_FLAG = 0x7E;

public:

};

#endif
