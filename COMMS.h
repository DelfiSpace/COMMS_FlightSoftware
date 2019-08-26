/*
 * COMMS.h
 *
 *  Created on: 23 Jul 2019
 *      Author: stefanosperett
 */

#ifndef EPS_H_
#define EPS_H_

#include <driverlib.h>
#include "msp.h"
#include "PQ9Bus.h"
#include "PQ9Frame.h"
#include "DWire.h"
#include "DSerial.h"
#include "PQ9CommandHandler.h"
#include "Service.h"
#include "HousekeepingService.h"
#include "COMMSHousekeepingService.h"
#include "COMMSTelemetryContainer.h"
#include "PingService.h"
#include "ResetService.h"
#include "SoftwareUpdateService.h"

#define FCLOCK 48000000

#define COMMS_ADDRESS     4

#endif /* COMMS_H_ */
