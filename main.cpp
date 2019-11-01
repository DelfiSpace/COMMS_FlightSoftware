
#include "COMMS.h"

// I2C busses
DWire I2Cinternal(0);

// CDHS bus handler
PQ9Bus pq9bus(3, GPIO_PORT_P9, GPIO_PIN0);

// debug console handler
DSerial serial;
// services running in the system
COMMSHousekeepingService hk;
PingService ping;
ResetService reset( GPIO_PORT_P5, GPIO_PIN0 );
SoftwareUpdateService SWUpdate;
Service* services[] = { &hk, &ping, &reset, &SWUpdate };

// COMMS board tasks
PQ9CommandHandler cmdHandler(pq9bus, services, 4);
PeriodicTask timerTask(FCLOCK, periodicTask);
Task* tasks[] = { &cmdHandler, &timerTask };

// system uptime
unsigned long uptime = 0;

void periodicTask()
{
    // increase the timer, this happens every second
    uptime++;

    // collect telemetry
    COMMSTelemetryContainer *tc = static_cast<COMMSTelemetryContainer*>(hk.getContainerToWrite());

    // acquire the telemetry

    // telemetry collected, store the values and prepare for next collection
    hk.stageTelemetry();

    // refresh the watch-dog configuration to make sure that, even in case of internal
    // registers corruption, the watch-dog is capable of recovering from an error
    reset.refreshConfiguration();

    // kick hardware watch-dog after every telemetry collection happens
    reset.kickExternalWatchDog();
}

/**
 * main.c
 */
void main(void)
{
    // initialize the MCU:
    // - clock source
    // - clock tree
    DelfiPQcore::initMCU();

    // initialize the reset handler:
    // - prepare the watch-dog
    // - initialize the pins for the hardware watchdog
    // prepare the pin for power cycling the system
    reset.init();

    // Initialize I2C masters
    I2Cinternal.setFastMode();
    I2Cinternal.begin();

    serial.begin( );                        // baud rate: 9600 bps
    pq9bus.begin(115200, COMMS_ADDRESS);    // baud rate: 115200 bps
                                            // address COMMS (4)

    // link the command handler to the PQ9 bus:
    // every time a new command is received, it will be forwarded to the command handler
    pq9bus.setReceiveHandler([](PQ9Frame &newFrame){ cmdHandler.received(newFrame); });

    // every time a command is correctly processed, call the watch-dog
    cmdHandler.onValidCommand([]{ reset.kickInternalWatchDog(); });

    serial.println("COMMS booting...");

    DelfiPQcore::startTaskManager(tasks, 2);
}
