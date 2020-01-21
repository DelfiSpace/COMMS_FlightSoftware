/*
 * TestService.cpp
 *
 *  Created on: 6 Aug 2019
 *      Author: stefanosperett
 */

#include "TestService.h"

extern DSerial serial;
extern DSPI controlSPI;
extern SX1276 tx, rx;

bool TestService::process(DataMessage &command, DataMessage &workingBuffer)
{
    if (command.getPayload()[0] == 0)
    {
        serial.println("TestService");

        if (command.getPayload()[1] == 1)
        {

        }
        else if (command.getPayload()[1] == 2)
        {
            serial.println("TX getFrequency ");
            unsigned long f = tx.getFrequency();
            serial.print(f, DEC);
            serial.println();
        }
        else if (command.getPayload()[1] == 3)
        {
            serial.println("setFrequency 435000000");
            tx.setFrequency(435000000);
        }
        else if (command.getPayload()[1] == 4)
        {
            serial.println("setFrequency 438000000");
            tx.setFrequency(438000000);
        }
        else if (command.getPayload()[1] == 5)
        {
            serial.println("RX getFrequency ");
            unsigned long f = rx.getFrequency();
            serial.print(f, DEC);
            serial.println();
        }
        else if (command.getPayload()[1] == 6)
        {
            serial.println("setFrequency 435000000");
            rx.setFrequency(435000000);
        }
        else if (command.getPayload()[1] == 7)
        {
            serial.println("setFrequency 438000000");
            rx.setFrequency(438000000);
        }

        return true;
    }
    return false;
}


