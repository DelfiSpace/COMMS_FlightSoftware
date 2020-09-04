/*
 * CLTUpacket.h
 *
 *  Created on: 12 Dec 2019
 *      Author: Casper Broekhuizen
 */
#include <stdint.h>

#ifndef PQPACKET_H_
#define PQPACKET_H_

#define PQPACKET_MAX_SIZE   256

class PQPacket
{
protected:


public:
    uint8_t data[PQPACKET_MAX_SIZE] = {0};
    int packetSize = PQPACKET_MAX_SIZE;

    bool isReady = false;

    int getSize();
    uint8_t* getBytes();

    void clear(){
        for(int i = 0; i < PQPACKET_MAX_SIZE; i++){
            data[i] = 0;
        }
    }
};




#endif /* PQPACKET_H_ */
