/*
 * CLTUpacket.h
 *
 *  Created on: 12 Dec 2019
 *      Author: Casper Broekhuizen
 */
#include <stdint.h>
#include "Console.h"

#ifndef PQPACKET_H_
#define PQPACKET_H_

#define PQPACKET_MAX_SIZE   256

#define PQPACKET_CRC16_POLY 0x1021

typedef enum PQCommand { ResetCommand = 0xAA, InternalCommand = 0x01, OverrideCommand = 0x02, OBCCommand = 0x03} PQCommand;

class PQPacket
{
protected:


public:
    uint8_t data[PQPACKET_MAX_SIZE] = {0};
    int packetSize = PQPACKET_MAX_SIZE;

    bool isReady = false;

    int getSize();
    uint8_t* getBytes();

    uint8_t getPQCommand();
    uint8_t getPQID();

    uint8_t* getPQPayload();
    int getPQPayloadSize();

    bool checkValidCRC();

    static uint16_t calculateCRC16(uint16_t seed, uint8_t* data, uint32_t length);

};




#endif /* PQPACKET_H_ */
