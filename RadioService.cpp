#include "RadioService.h"

RadioService::RadioService(COMMRadio &radio_in):
    radio(&radio_in){

};
bool RadioService::process(DataMessage &command, DataMessage &workingBuffer)
{
    if (command.getService() == RADIO_SERVICE)
    {
//        Console::log("RadioService: Service started");
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
//            Console::log("RadioService: Get nr of RX frames: %d", radio->getNumberOfRXFrames());
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
            Console::log("RadioService: get Instantaneous RX RSSI");
            workingBuffer.setPayloadSize(3); //OK + short
            workingBuffer.getDataPayload()[0] = RADIO_CMD_NO_ERROR;
            signed short rssi = radio->getRXRSSI();
            workingBuffer.getDataPayload()[1] = ((uint8_t*) &rssi)[0];
            workingBuffer.getDataPayload()[2] = ((uint8_t*) &rssi)[0];
            break;
        case RADIO_CMD_GET_LAST_RSSI_RX:
            Console::log("RadioService: get RX RSSI of last packet");
            workingBuffer.setPayloadSize(3); //OK + short
            workingBuffer.getDataPayload()[0] = RADIO_CMD_NO_ERROR;
            signed short lastrssi = radio->lastRSSI;
            workingBuffer.getDataPayload()[1] = ((uint8_t*) &lastrssi)[0];
            workingBuffer.getDataPayload()[2] = ((uint8_t*) &lastrssi)[0];
            break;
        case RADIO_CMD_GET_LAST_FREQERROR_RX:
            Console::log("RadioService: get RX Frequency Error of last packet");
            workingBuffer.setPayloadSize(3); //OK + short
            workingBuffer.getDataPayload()[0] = RADIO_CMD_NO_ERROR;
            signed short fError = radio->lastFreqError;
            workingBuffer.getDataPayload()[1] = ((uint8_t*) &fError)[0];
            workingBuffer.getDataPayload()[2] = ((uint8_t*) &fError)[0];
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
            if(command.getPayloadSize() == 3){
                short targetBitrate = ((command.getDataPayload()[1] << 8) & 0xFF00) | ((command.getDataPayload()[2]) & 0x00FF);
                Console::log("RadioService: Set TX bitrate to: %d", targetBitrate);
                radio->txBitrate = targetBitrate;
                workingBuffer.setPayloadSize(1);
                workingBuffer.getDataPayload()[0] = RADIO_CMD_NO_ERROR;
            }else{
                Console::log("RadioService: Unknown command!");
                workingBuffer.setPayloadSize(1);
                workingBuffer.getDataPayload()[0] = RADIO_CMD_UNKNOWN_COMMAND;
            }
            break;
        case RADIO_CMD_SET_PA:
            Console::log("RadioService: Set PA Mode to: %d", command.getDataPayload()[1]);
            switch(command.getDataPayload()[1]){
            case 0:
                Console::log("PA TARGET - LOW");
                radio->targetPAPower = 0;
                break;
            case 1:
                Console::log("PA TARGET - MED");
                radio->targetPAPower = 1;
                break;
            case 2:
                Console::log("PA TARGET - HIGH");
                radio->targetPAPower = 2;
                break;
            default:
                Console::log("PA TARGET - UNKNOWN");
                radio->targetPAPower = 1;
                break;
            }
            break;
        case RADIO_CMD_SET_TX_POWER:
            Console::log("RadioService: Set TXPower to: 0x%x", command.getDataPayload()[1]);
            radio->targetPAPower = command.getDataPayload()[1];
            break;
        case 99:
            Console::log("RESET COMMAND!");
            MAP_GPIO_setOutputHighOnPin(COMMS_RESET_PORT, COMMS_RESET_PIN);
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
