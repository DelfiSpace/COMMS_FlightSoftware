/*
 * CLTUpacket.h
 *
 *  Created on: 12 Dec 2019
 *      Author: Casper Broekhuizen
 */

#ifndef CLTUPACKET_H_
#define CLTUPACKET_H_

#include "Dserial.h"

#define CLTU_MAX_SIZE   64

class CLTUPacket
{
protected:


public:
    uint8_t data[CLTU_MAX_SIZE] = {0};
    int packetSize = 0;
};




#endif /* CLTUPACKET_H_ */
