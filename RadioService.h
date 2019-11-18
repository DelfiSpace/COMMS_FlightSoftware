#ifndef TESTSERVICE_H_
#define TESTSERVICE_H_

#include "Service.h"
#include "PQ9Frame.h"
#include "DSerial.h"
#include "DSPI.h"
#include "SX1276.h"

class TestService: public Service
{
 public:
     virtual bool process( PQ9Frame &command, PQ9Bus &interface, PQ9Frame &workingBbuffer );
};
#endif /* TESTSERVICE_H_ */
