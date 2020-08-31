#include "RadioService.h"
#include "AX25Frame.h"


RadioService::RadioService(COMMRadio &radio_in):
    radio(&radio_in){

};
bool RadioService::process(DataMessage &command, DataMessage &workingBuffer)
{
    if (command.getService() == RADIO_SERVICE)
    {
        // serial.println("RadioService: Service started.");
        // prepare response frame
        //workingBuffer.setDestination(command.getSource());
        //workingBuffer.setSource(interface.getAddress());
        workingBuffer.setService(RADIO_SERVICE);
        workingBuffer.setMessageType(SERVICE_RESPONSE_REPLY);


        if (command.getDataPayload()[0] == RADIO_CMD_INIT_TX)
        {
            Console::log("RadioService: Initialise TX Request");
            // respond to ping
            radio->initTX();
            workingBuffer.setSize(2);
            workingBuffer.getPayload()[0] = RADIO_SERVICE;
            workingBuffer.getPayload()[1] = RADIO_CMD_ACCEPT;
        }
        else if (command.getDataPayload()[0] == RADIO_CMD_INIT_TX)
        {
            Console::log("RadioService: Initialise TX Request");
            // respond to ping
            radio->initTX();
            workingBuffer.setSize(2);
            workingBuffer.getPayload()[0] = RADIO_SERVICE;
            workingBuffer.getPayload()[1] = RADIO_CMD_ACCEPT;
        }
        else if(command.getDataPayload()[0] == RADIO_CMD_SENDFRAME)
        {
            workingBuffer.setPayloadSize(1);

            uint8_t packetSize = command.getPayloadSize() - 1;
            if(radio->transmitData(&command.getDataPayload()[1], packetSize)){
                workingBuffer.getDataPayload()[0] = RADIO_CMD_ACCEPT;
            }else{
                workingBuffer.getDataPayload()[0] = RADIO_CMD_REJECT;
            }
        }
        else if(command.getPayload()[1] == RADIO_CMD_GETFRAME)
        {
            if (radio->getNumberOfRXFrames() > 0){
                //frames in buffer
                //int frameSize = radio->getSizeOfRXFrame() - 2;
                int frameSize = radio->getSizeOfRXFrame() - 18;
                workingBuffer.setSize(2 + frameSize);
                workingBuffer.getPayload()[0] = RADIO_SERVICE;
                workingBuffer.getPayload()[1] = SERVICE_RESPONSE_REPLY;
                uint8_t * frameData = radio->getRXFrame();
                for(int j = 0; j < frameSize; j++){
                    workingBuffer.getPayload()[2+j] = frameData[16+j];
                }
                radio->popFrame();
            }else{
                //no frames in buffer
                workingBuffer.setSize(2);
                workingBuffer.getPayload()[0] = RADIO_SERVICE;
                workingBuffer.getPayload()[1] = SERVICE_RESPONSE_ERROR;
            }
            //radio->toggleReceivePrint();
            //workingBuffer.getPayload()[1] = RADIO_CMD_ACCEPT;
        }
        else if(command.getPayload()[1] == RADIO_CMD_PRINT_RX)
        {
            //serial.println("TOGGLE RX PRINTING");
            //radio->toggleReceivePrint();
            radio->toggleReceivePrint();
            workingBuffer.setSize(2);
            workingBuffer.getPayload()[0] = RADIO_SERVICE;
            workingBuffer.getPayload()[1] = RADIO_CMD_ACCEPT;
        }
        else if(command.getPayload()[1] == RADIO_CMD_TOGGLE_MODE)
        {
            //serial.println("TOGGLE RX PRINTING");
            //radio->toggleReceivePrint();
            radio->toggleMode();
            workingBuffer.setSize(2);
            workingBuffer.getPayload()[0] = RADIO_SERVICE;
            workingBuffer.getPayload()[1] = RADIO_CMD_ACCEPT;
        }
        else
        {
            // unknown request
            workingBuffer.setSize(2);
            workingBuffer.getPayload()[1] = RADIO_CMD_ERROR;
            workingBuffer.getPayload()[0] = RADIO_SERVICE;
        }

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
