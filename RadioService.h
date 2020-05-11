
#include "Service.h"
#include "DSPI.h"
#include "SX1276.h"
#include "COMMRadio.h"
#include "Console.h"

#define RADIO_SERVICE            20
#define RADIO_CMD_INIT_TX        1
#define RADIO_CMD_INIT_RX        2
#define RADIO_CMD_SENDFRAME      3
#define RADIO_CMD_GETFRAME       4
#define RADIO_CMD_GETFREQ_TX     5
#define RADIO_CMD_GETFREQ_RX     6
#define RADIO_CMD_SETFREQ_TX     7
#define RADIO_CMD_SETFREQ_RX     8
#define RADIO_CMD_GETBITRATE_TX  9
#define RADIO_CMD_GETBITRATE_RX  10
#define RADIO_CMD_SETBITRATE_TX  11
#define RADIO_CMD_SETBITRATE_RX  12
#define RADIO_CMD_GETPROTOCOL    13
#define RADIO_CMD_SETPROTOCOL    14
#define RADIO_CMD_GETLASTRSSI    15
#define RADIO_CMD_GETRSSI        16
#define RADIO_CMD_SETTXIDLE      17

#define RADIO_CMD_PRINT_RX       20
#define RADIO_CMD_TOGGLE_MODE    21

#define RADIO_CMD_ACCEPT        2
#define RADIO_CMD_REJECT        3
#define RADIO_CMD_ERROR         0


class RadioService: public Service
{
protected:
    COMMRadio *radio;
public:
    RadioService(COMMRadio &radio_in);

    virtual bool process( DataMessage &command, DataMessage &workingBbuffer );
};

