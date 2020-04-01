/*
 * TestService.cpp
 *
 *  Created on: 6 Aug 2019
 *      Author: stefanosperett
 */

#include "TestService.h"

extern DSPI controlSPI;
extern SX1276 tx, rx;

bool TestService::process(DataMessage &command, DataMessage &workingBuffer)
{
    if (command.getPayload()[0] == 0)
    {
        Console::log("TestService");

        if (command.getPayload()[1] == 1)
        {

        }
        else if (command.getPayload()[1] == 2)
        {
            unsigned long f = tx.getFrequency();
            Console::log("TX getFrequency %d", (int) f);
        }
        else if (command.getPayload()[1] == 3)
        {
            Console::log("setFrequency 435000000");
            tx.setFrequency(435000000);
        }
        else if (command.getPayload()[1] == 4)
        {
            Console::log("setFrequency 438000000");
            tx.setFrequency(438000000);
        }
        else if (command.getPayload()[1] == 5)
        {
            unsigned long f = rx.getFrequency();
            Console::log("RX getFrequency: %d", (int) f);
        }
        else if (command.getPayload()[1] == 6)
        {
            Console::log("setFrequency 435000000");
            rx.setFrequency(435000000);
        }
        else if (command.getPayload()[1] == 7)
        {
            Console::log("setFrequency 438000000");
            rx.setFrequency(438000000);
        }

        return true;
    }
    return false;
}


