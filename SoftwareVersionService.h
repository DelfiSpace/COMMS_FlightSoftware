/*
 * SoftwareVersionService.h
 *
 *  Created on: 02 Dec 2019
 *      Author: Casper Broekhuizen
 */

#ifndef SOFTWAREVERSIONSERVICE_H_
#define SOFTWAREVERSIONSERVICE_H_

#include "PQ9Bus.h"
#include "PQ9Frame.h"
#include "Service.h"
#include "DSerial.h"

#define SOFTWAREVERSION_SERVICE         23

#define SOFTWAREVERSION_GETVERSION      1
#define SOFTWAREVERSION_ACCEPT          2
#define SOFTWAREVERSION_ERROR           0

class SoftwareVersionService: public Service
{
 public:
     virtual bool process( PQ9Frame &command, PQ9Sender &interface, PQ9Frame &workingBbuffer );
};

#endif /* SOFTWAREVERSIONSERVICE_H_ */
