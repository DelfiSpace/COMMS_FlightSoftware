/*
 * CLTUpacket.cpp
 *
 *  Created on: 12 Dec 2019
 *      Author: Casper Broekhuizen
 */
#include "CLTUPacket.h"

int CLTUPacket::getSize(){
    return packetSize;
}

uint8_t* CLTUPacket::getBytes(){
    return data;
}


