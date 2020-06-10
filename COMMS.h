/*
 * COMMS.h
 *
 *  Created on: 23 Jul 2019
 *      Author: stefanosperett
 */

#ifndef EPS_H_
#define EPS_H_

#include <driverlib.h>
#include "SLOT_SELECT.h"
#include "MB85RS.h"
#include "msp.h"
#include "DelfiPQcore.h"
#include "PQ9Bus.h"
#include "PQ9Frame.h"
#include "PQ9Message.h"
#include "DWire.h"
#include "DSPI.h"
#include "CommandHandler.h"
#include "Task.h"
#include "PeriodicTask.h"
#include "TaskManager.h"
#include "HousekeepingService.h"
#include "COMMSTelemetryContainer.h"
#include "PingService.h"
#include "COMMSTelemetryContainer.h"
#include "TestService.h"
#include "ResetService.h"
#include "SoftwareUpdateService.h"
#include "SX1276.h"
#include "PeriodicTaskNotifier.h"
#include "HWMonitor.h"
#include "ADCManager.h"


#define FCLOCK 48000000

#define COMMS_ADDRESS     4

// callback functions
void acquireTelemetry(COMMSTelemetryContainer *tc);
void periodicTask();
void periodicTask2();

#endif /* COMMS_H_ */
