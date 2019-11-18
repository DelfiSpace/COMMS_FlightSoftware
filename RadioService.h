#ifndef TESTSERVICE_H_
#define TESTSERVICE_H_

#include "Service.h"
#include "PQ9Frame.h"
#include "DSerial.h"
#include "DSPI.h"
#include "SX1276.h"
#include "COMMRadio.h"

#define RADIO_SERVICE            20
#define RADIO_CMD_INIT_TX        1
#define RADIO_CMD_INIT_RX        2
#define RADIO_CMD_TRANSMIT       3

#define RADIO_CMD_ACCEPT        1
#define RADIO_CMD_ERROR         2


class RadioService : public Service
{
protected:
    COMMRadio *radio;
public:
    RadioService(COMMRadio &radio_in);
    virtual bool process( PQ9Frame &command, PQ9Bus &interface, PQ9Frame &workingBbuffer );
};
#endif /* TESTSERVICE_H_ */
