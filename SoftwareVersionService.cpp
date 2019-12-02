/*
 * SoftwareVersionService.cpp
 *
 *  Created on: 02 Dec 2019
 *      Author: Casper Broekhuizen
 */
#include <SoftwareVersionService.h>

#define xstr(s) str(s)
#define str(s) #s
#define HEXTONIBBLE(s) (*(s) >= 'a' ?  *(s) - 87 : *(s) - 48)
#define NIBBLESTOBYTE(msb,lsb) ((msb << 4) | lsb)


#ifndef SW_VERSION
#define HAS_SW_VERSION 0
#else
#define HAS_SW_VERSION 1
#endif

uint8_t HexStringToNibble(char c){
    return ((c) >= 'a' ? (c) - 87 : (c) - 48);
}

uint8_t NibblesToByte(uint8_t msb, uint8_t lsb){
    return ((msb << 4) | lsb);
}

extern DSerial serial;

/**
 *
 *   Process the Service (Called by CommandHandler)
 *
 *   Parameters:
 *   PQ9Frame &command          Frame received over the bus
 *   PQ9Bus &interface          Bus object
 *   PQ9Frame &workingBuffer    Reference to buffer to store the response.
 *
 *   Returns:
 *   bool true      :           Frame is directed to this Service
 *        false     :           Frame is not directed to this Service
 *
 */
bool SoftwareVersionService::process(PQ9Frame &command, PQ9Bus &interface, PQ9Frame &workingBuffer)
{
    if (command.getPayload()[0] == SOFTWAREVERSION_SERVICE) //Check if this frame is directed to this service
    {
        // prepare response frame
        workingBuffer.setDestination(command.getSource());
        workingBuffer.setSource(interface.getAddress());
        workingBuffer.getPayload()[0] = SOFTWAREVERSION_SERVICE;

        if (command.getPayload()[1] == SOFTWAREVERSION_GETVERSION)
        {
            workingBuffer.setPayloadSize(2);
            serial.println("SoftwareVersionService: Version Request");
            // respond to ping
            workingBuffer.getPayload()[1] = SOFTWAREVERSION_ACCEPT;
            if(HAS_SW_VERSION == 1){
                workingBuffer.setPayloadSize(6);
                workingBuffer.getPayload()[1] = SOFTWAREVERSION_ACCEPT;
                serial.println("has SW Version!");
                serial.print("SW VERSION:  ");
                serial.println(xstr(SW_VERSION));
                uint8_t versionString[] = {xstr(SW_VERSION)};
                workingBuffer.getPayload()[2] = NibblesToByte(0,HexStringToNibble(versionString[0]));
                workingBuffer.getPayload()[3] = NibblesToByte(HexStringToNibble(versionString[1]),HexStringToNibble(versionString[2]));
                workingBuffer.getPayload()[4] = NibblesToByte(HexStringToNibble(versionString[3]),HexStringToNibble(versionString[4]));
                workingBuffer.getPayload()[5] = NibblesToByte(HexStringToNibble(versionString[5]),HexStringToNibble(versionString[6]));
            }else{
                serial.println("has no SW Version!");
                workingBuffer.setPayloadSize(2);
                workingBuffer.getPayload()[1] = SOFTWAREVERSION_ERROR;
            }
        }
        else
        {
            // unknown request
            workingBuffer.setPayloadSize(2);
            workingBuffer.getPayload()[1] = SOFTWAREVERSION_ERROR;
        }

        // send response
        interface.transmit(workingBuffer);
        // command processed
        return true;
    }
    else
    {
        // this command is related to another service,
        // report the command was not processed
        return false;
    }
}
