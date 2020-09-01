/*
 * CLTUpacket.cpp
 *
 *  Created on: 12 Dec 2019
 *      Author: Casper Broekhuizen
 */
#include <PQPacket.h>

int PQPacket::getSize(){
    return packetSize;
}

uint8_t* PQPacket::getBytes(){
    return data;
}


