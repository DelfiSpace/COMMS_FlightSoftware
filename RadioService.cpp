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
            //todo
            Console::log("RadioService: get RX Frame: ");
            workingBuffer.setPayloadSize(1);
            workingBuffer.getDataPayload()[0] = RADIO_CMD_NO_ERROR;
            break;
        case RADIO_CMD_REMOVEFRAME_RX:
            //todo
            Console::log("RadioService: remove RX Frame: ");
            workingBuffer.setPayloadSize(1);
            workingBuffer.getDataPayload()[0] = RADIO_CMD_NO_ERROR;
            break;
        case RADIO_CMD_GET_RSSI_RX:
            //todo
            Console::log("RadioService: get RX RSSI");
            workingBuffer.setPayloadSize(1);
            workingBuffer.getDataPayload()[0] = RADIO_CMD_NO_ERROR;
            break;
        case RADIO_CMD_SENDFRAME:
            //todo
            Console::log("RadioService: Add frame to txbuffer");
            workingBuffer.setPayloadSize(1);
            workingBuffer.getDataPayload()[0] = RADIO_CMD_NO_ERROR;
            break;
        case RADIO_CMD_SENDFRAME_OC:
            //todo
            Console::log("RadioService: Add frame to txbuffer + override callsign");
            workingBuffer.setPayloadSize(1);
            workingBuffer.getDataPayload()[0] = RADIO_CMD_NO_ERROR;
            break;
        case RADIO_CMD_SET_CS_TO:
            //todo
            Console::log("RadioService: set TX -TO- callsign: ");
            workingBuffer.setPayloadSize(1);
            workingBuffer.getDataPayload()[0] = RADIO_CMD_NO_ERROR;
            break;
        case RADIO_CMD_SET_CS_FROM:
            //todo
            Console::log("RadioService: set TX -FROM- callsign: ");
            workingBuffer.setPayloadSize(1);
            workingBuffer.getDataPayload()[0] = RADIO_CMD_NO_ERROR;
            break;
        case RADIO_CMD_SET_TX_IDLE_STATE:
            //todo
            Console::log("RadioService: set TX idle mode:");
            workingBuffer.setPayloadSize(1);
            workingBuffer.getDataPayload()[0] = RADIO_CMD_NO_ERROR;
            break;
        case RADIO_CMD_SET_TX_BITRATE:
            //todo
            Console::log("RadioService: Set TX bitrate to: %d", 0);
            workingBuffer.setPayloadSize(1);
            workingBuffer.getDataPayload()[0] = RADIO_CMD_NO_ERROR;
            break;
        default:
            Console::log("RadioService: Unknown command!");
            workingBuffer.setPayloadSize(1);
            workingBuffer.getDataPayload()[0] = RADIO_CMD_UNKNOWN_COMMAND;
            break;
        }

//  if(command.getDataPayload()[0] == RADIO_CMD_SENDFRAME)
//        {
//            workingBuffer.setPayloadSize(1);
//
//            uint8_t packetSize = command.getPayloadSize() - 1;
//            if(radio->transmitData(&command.getDataPayload()[1], packetSize)){
//                workingBuffer.getDataPayload()[0] = RADIO_CMD_ACCEPT;
//            }else{
//                workingBuffer.getDataPayload()[0] = RADIO_CMD_REJECT;
//            }
//        }
//        else if(command.getPayload()[1] == RADIO_CMD_GETFRAME)
//        {
//            if (radio->getNumberOfRXFrames() > 0){
//                //frames in buffer
//                //int frameSize = radio->getSizeOfRXFrame() - 2;
//                int frameSize = radio->getSizeOfRXFrame() - 18;
//                workingBuffer.setSize(2 + frameSize);
//                workingBuffer.getPayload()[0] = RADIO_SERVICE;
//                workingBuffer.getPayload()[1] = SERVICE_RESPONSE_REPLY;
//                uint8_t * frameData = radio->getRXFrame();
//                for(int j = 0; j < frameSize; j++){
//                    workingBuffer.getPayload()[2+j] = frameData[16+j];
//                }
//                radio->popFrame();
//            }else{
//                //no frames in buffer
//                workingBuffer.setSize(2);
//                workingBuffer.getPayload()[0] = RADIO_SERVICE;
//                workingBuffer.getPayload()[1] = SERVICE_RESPONSE_ERROR;
//            }
//            //radio->toggleReceivePrint();
//            //workingBuffer.getPayload()[1] = RADIO_CMD_ACCEPT;
//        }
//        else if(command.getPayload()[1] == RADIO_CMD_PRINT_RX)
//        {
//            //serial.println("TOGGLE RX PRINTING");
//            //radio->toggleReceivePrint();
//            radio->toggleReceivePrint();
//            workingBuffer.setSize(2);
//            workingBuffer.getPayload()[0] = RADIO_SERVICE;
//            workingBuffer.getPayload()[1] = RADIO_CMD_ACCEPT;
//        }
//        else if(command.getPayload()[1] == RADIO_CMD_TOGGLE_MODE)
//        {
//            //serial.println("TOGGLE RX PRINTING");
//            //radio->toggleReceivePrint();
//            radio->toggleMode();
//            workingBuffer.setSize(2);
//            workingBuffer.getPayload()[0] = RADIO_SERVICE;
//            workingBuffer.getPayload()[1] = RADIO_CMD_ACCEPT;
//        }
//        else
//        {
//            // unknown request
//            workingBuffer.setSize(2);
//            workingBuffer.getPayload()[1] = RADIO_CMD_ERROR;
//            workingBuffer.getPayload()[0] = RADIO_SERVICE;
//        }

        // send response
        //interface.transmit(workingBuffer);
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
