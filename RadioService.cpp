#include "RadioService.h"

extern DSerial serial;

RadioService::RadioService(COMMRadio &radio_in):
    radio(&radio_in){

};
bool RadioService::process(PQ9Frame &command, PQ9Bus &interface, PQ9Frame &workingBuffer)
{
    if (command.getPayload()[0] == RADIO_SERVICE)
    {
        // prepare response frame
        workingBuffer.setDestination(command.getSource());
        workingBuffer.setSource(interface.getAddress());
        workingBuffer.setPayloadSize(2);
        workingBuffer.getPayload()[0] = RADIO_SERVICE;

        if (command.getPayload()[1] == RADIO_CMD_INIT_TX)
        {
            serial.println("RadioService: Initialise TX Request");
            // respond to ping
            radio->initTX();
            workingBuffer.getPayload()[1] = RADIO_CMD_ACCEPT;
        }
        else if(command.getPayload()[1] == RADIO_CMD_INIT_RX)
        {
            serial.println("RadioService: Initialise RX Request");
            // respond to ping
            radio->initRX();
            workingBuffer.getPayload()[1] = RADIO_CMD_ACCEPT;
        }
        else if(command.getPayload()[1] == RADIO_CMD_TRANSMIT)
        {
            serial.println("RadioService: Transmit Request");
            // get packet size
            uint8_t packetSize = command.getPayload()[2];
            uint8_t dataPacket[packetSize] = new uint8_t [packetSize];
            for(int i = 0; i < packetSize; i++){
                dataPacket[i] = command.getPayload()[3+i];
            }
            radio->transmitData(dataPacket, packetSize);
            workingBuffer.getPayload()[1] = RADIO_CMD_ACCEPT;
        }
        else if(command.getPayload()[1] == RADIO_CMD_TOGGLE_RX)
        {
            serial.println("RadioService: Toggle RX print request");
            // respond to ping
            radio->toggleReceivePrint();
            workingBuffer.getPayload()[1] = RADIO_CMD_ACCEPT;
        }
        else
        {
            // unknown request
            workingBuffer.getPayload()[1] = RADIO_CMD_ERROR;
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
