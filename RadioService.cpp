#include "RadioService.h"

RadioService::RadioService(COMMRadio &radio_in):
    radio(&radio_in){

};
bool RadioService::process(DataMessage &command, DataMessage &workingBuffer)
{
    if (command.getService() == RADIO_SERVICE)
    {
        Console::log("RadioService: Service started");
        workingBuffer.setService(RADIO_SERVICE);
        workingBuffer.setMessageType(SERVICE_RESPONSE_REPLY);

        switch(command.getDataPayload()[0])
        {
        case RADIO_CMD_INIT_TX:
            Console::log("RadioService: Init TX");
            radio->initTX();
            workingBuffer.setPayloadSize(1);
            workingBuffer.getDataPayload()[0] = RADIO_CMD_NO_ERROR;
            break;
        case RADIO_CMD_INIT_RX:
            Console::log("RadioService: Init TX");
            radio->initRX();
            workingBuffer.setPayloadSize(1);
            workingBuffer.getDataPayload()[0] = RADIO_CMD_NO_ERROR;
            break;
        case RADIO_CMD_NR_OF_RX:
            Console::log("RadioService: Get nr of RX frames: %d", radio->getNumberOfRXFrames());
            workingBuffer.setPayloadSize(2);
            workingBuffer.getDataPayload()[0] = RADIO_CMD_NO_ERROR;
            workingBuffer.getDataPayload()[1] = radio->getNumberOfRXFrames();
            break;
        case RADIO_CMD_GETFRAME_RX:
            Console::log("RadioService: get RX Frame: ");
            workingBuffer.getDataPayload()[0] = RADIO_CMD_NO_ERROR;
            workingBuffer.setPayloadSize(radio->getSizeOfRXFrame()+1);
            unsigned char* rxFrame = radio->getRXFrame();
            if(radio->getNumberOfRXFrames()){
                for(int i = 0; i < radio->getSizeOfRXFrame(); i++){
                    workingBuffer.getDataPayload()[1+i] = rxFrame[i];
                }
            }
            break;
        case RADIO_CMD_REMOVEFRAME_RX:
            Console::log("RadioService: remove RX Frame: ");
            workingBuffer.setPayloadSize(1);
            workingBuffer.getDataPayload()[0] = RADIO_CMD_NO_ERROR;
            radio->popFrame();
            break;
        case RADIO_CMD_GET_RSSI_RX:
            //todo
            Console::log("#TODO# RadioService: get RX RSSI");
            workingBuffer.setPayloadSize(1);
            workingBuffer.getDataPayload()[0] = RADIO_CMD_NO_ERROR;
            break;
        case RADIO_CMD_SENDFRAME:
            Console::log("RadioService: Add frame to txbuffer (size:%d)", command.getPayloadSize()-1);
            workingBuffer.setPayloadSize(1);
            workingBuffer.getDataPayload()[0] = RADIO_CMD_NO_ERROR;
            radio->quePacketAX25(&command.getDataPayload()[1], command.getPayloadSize()-1);
            break;
        case RADIO_CMD_SENDFRAME_OC:
            //todo
            Console::log("#TODO# RadioService: Add frame to txbuffer + override callsign");
            workingBuffer.setPayloadSize(1);
            workingBuffer.getDataPayload()[0] = RADIO_CMD_NO_ERROR;
            break;
        case RADIO_CMD_SET_CS_TO:
            //todo
            Console::log("#TODO# RadioService: set TX -TO- callsign: ");
            workingBuffer.setPayloadSize(1);
            workingBuffer.getDataPayload()[0] = RADIO_CMD_NO_ERROR;
            break;
        case RADIO_CMD_SET_CS_FROM:
            //todo
            Console::log("#TODO# RadioService: set TX -FROM- callsign: ");
            workingBuffer.setPayloadSize(1);
            workingBuffer.getDataPayload()[0] = RADIO_CMD_NO_ERROR;
            break;
        case RADIO_CMD_SET_TX_IDLE_STATE:
            Console::log("RadioService: set TX idle mode:");
            if(command.getPayloadSize() == 2)
            {
                workingBuffer.setPayloadSize(1);
                workingBuffer.getDataPayload()[0] = RADIO_CMD_NO_ERROR;
                radio->setIdleMode(command.getDataPayload()[1] == 1);
            }
            else
            {
                workingBuffer.setPayloadSize(1);
                workingBuffer.getDataPayload()[0] = RADIO_CMD_INVALID_VALUE;
            }
            break;
        case RADIO_CMD_SET_TX_BITRATE:
            //todo
            Console::log("#TODO# RadioService: Set TX bitrate to: %d", 0);
            workingBuffer.setPayloadSize(1);
            workingBuffer.getDataPayload()[0] = RADIO_CMD_NO_ERROR;
            break;
        default:
            Console::log("RadioService: Unknown command!");
            workingBuffer.setPayloadSize(1);
            workingBuffer.getDataPayload()[0] = RADIO_CMD_UNKNOWN_COMMAND;
            break;
        }

        return true;
    }
    else
    {
        // this command is related to another service,
        // report the command was not processed
        return false;
    }
}
