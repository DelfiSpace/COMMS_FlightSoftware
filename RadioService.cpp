#include "RadioService.h"

extern DSerial serial;

RadioService::RadioService(COMMRadio &radio_in):
    radio(&radio_in){

};
bool RadioService::process(PQ9Frame &command, PQ9Bus &interface, PQ9Frame &workingBuffer)
{
    if (command.getPayload()[0] == RADIO_SERVICE)
    {
        serial.println("RadioService: Service started.");
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
            radio->transmitData(0,0);
            //uint8_t dataPacket[packetSize]
            //TODO
            //radio->transmitData(dataPacket, packetSize);
            workingBuffer.getPayload()[1] = RADIO_CMD_ACCEPT;
        }
        else if(command.getPayload()[1] == RADIO_CMD_TOGGLE_RX)
        {
            serial.println("RadioService: Toggle RX print request");
            // respond to ping
            serial.println("RX-Mode:");
            serial.println("===");
            serial.println(radio->readRXReg(0x31));
            serial.println(radio->readRXReg(0x01));
            serial.println("===");
            serial.println("TX-Mode:");
            serial.println("===");
            serial.println(radio->readTXReg(0x31));
            serial.println(radio->readTXReg(0x01));
            serial.println("===");
            workingBuffer.getPayload()[1] = RADIO_CMD_ACCEPT;
        }
        else if(command.getPayload()[1] == RADIO_CMD_SET_REG1)
        {
            serial.println("RadioService: SET REG DEBUG 1");
            // respond to ping
            //radio->initRX();
            radio->writeTXReg(0x40, 0x10);
            radio->writeTXReg(0x31, 0x40);
            workingBuffer.getPayload()[1] = RADIO_CMD_ACCEPT;
        }
        else if(command.getPayload()[1] == RADIO_CMD_SET_REG2)
        {
            serial.println("RadioService: SET REG DEBUG 2");
            // respond to ping
            //radio->initRX();
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
