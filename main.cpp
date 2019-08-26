
#include "COMMS.h"

// I2C busses
DWire I2Cinternal(0);

// CDHS bus handler
PQ9Bus pq9bus(3, GPIO_PORT_P10, GPIO_PIN0);

// debug console handler
DSerial serial;
// services running in the system
COMMSHousekeepingService hk;
PingService ping;
ResetService reset( GPIO_PORT_P5, GPIO_PIN0 );
SoftwareUpdateService SWUpdate;
Service* services[] = { &hk, &ping, &reset, &SWUpdate };

// command handler, dealing with all CDHS requests and responses
PQ9CommandHandler cmdHandler(pq9bus, services, 4);

// system uptime
unsigned long uptime = 0;
volatile int counter = 0;

void acquireTelemetry(COMMSTelemetryContainer *tc)
{

}

void timerHandler(void)
{
    MAP_Timer32_clearInterruptFlag(TIMER32_0_BASE);
    counter++;
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

    // init the reset handler:
    // - prepare the watchdog
    // - initialize the pins for the hardware watchdog
    // prepare the pin for power cycling the system
    reset.init();

    // Initialize I2C masters
    I2Cinternal.setFastMode();
    I2Cinternal.begin();

    serial.begin( );                        // baud rate: 9600 bps
    pq9bus.begin(115200, COMMS_ADDRESS);    // baud rate: 115200 bps
                                            // address COMMS (4)

    // initialize the command handler: from now on, commands can be processed
    cmdHandler.init();

    // Configuring Timer32 to FCLOCK (1s) of MCLK in periodic mode
    MAP_Timer32_initModule(TIMER32_0_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT,
            TIMER32_PERIODIC_MODE);
    MAP_Timer32_registerInterrupt(TIMER32_0_INTERRUPT, &timerHandler);
    MAP_Timer32_setCount(TIMER32_0_BASE, FCLOCK);
    MAP_Timer32_startTimer(TIMER32_0_BASE, false);

    serial.println("Hello World");

    while(true)
    {
        if (cmdHandler.commandLoop())
        {
            // if a correct command has been received, clear the watchdog
            reset.kickInternalWatchDog();
        }

        // hack to simulate timer to acquire telemetry approximately once per second
        if (counter != 0)
        {
            uptime ++;

            // collect telemetry
            COMMSTelemetryContainer *tc = static_cast<COMMSTelemetryContainer*>(hk.getContainerToWrite());
            acquireTelemetry(tc);

            // telemetry collected, store the values and prepare for next collection
            hk.stageTelemetry();

            // watchdog time window
            // t_win typ = 22.5s max = 28s min = 16.8s
            // tb_min 182ms tb max 542ms
            // kick hardware watchdog after every telemetry collection happens
            reset.kickExternalWatchDog();
        }

        //MAP_PCM_gotoLPM0();

    }
}
