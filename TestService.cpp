/*
 * TestService.cpp
 *
 *  Created on: 6 Aug 2019
 *      Author: stefanosperett
 */

#include "TestService.h"

extern DSerial serial;

bool TestService::process(PQ9Frame &command, PQ9Bus &interface, PQ9Frame &workingBuffer)
{
    if (command.getPayload()[0] == 0)
    {
        serial.println("TestService");

        if (command.getPayload()[1] == 1)
        {


        }

        return true;
    }
    return false;
}


