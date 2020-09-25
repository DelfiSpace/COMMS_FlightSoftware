
#include "Service.h"
#include "COMMRadio.h"
#include "Console.h"

#ifndef RADIOSERVICE_H_
#define RADIOSERVICE_H_

#define RADIO_SERVICE               25
#define RADIO_CMD_INIT_TX           1
#define RADIO_CMD_INIT_RX           2

#define RADIO_CMD_NR_OF_RX          3
#define RADIO_CMD_GETFRAME_RX       4
#define RADIO_CMD_REMOVEFRAME_RX    5
#define RADIO_CMD_GET_RSSI_RX       6
#define RADIO_CMD_GET_LAST_RSSI_RX      7
#define RADIO_CMD_GET_LAST_FREQERROR_RX 8

#define RADIO_CMD_SENDFRAME         9
#define RADIO_CMD_SENDFRAME_OC      10
#define RADIO_CMD_SET_CS_TO         11
#define RADIO_CMD_SET_CS_FROM       12
#define RADIO_CMD_SET_TX_IDLE_STATE 13
#define RADIO_CMD_SET_TX_BITRATE    14
#define RADIO_CMD_SET_PA            15
#define RADIO_CMD_SET_TX_POWER  16

#define RADIO_CMD_NO_ERROR          0
#define RADIO_CMD_UNKNOWN_COMMAND   1
#define RADIO_CMD_INVALID_VALUE     2


class RadioService: public Service
{
protected:
    COMMRadio *radio;
public:
    RadioService(COMMRadio &radio_in);

    virtual bool process( DataMessage &command, DataMessage &workingBbuffer );
};

#endif
