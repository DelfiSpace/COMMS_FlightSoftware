#define xstr(s) str(s)
#define str(s) #s

#include "COMMS.h"

// I2C busses
DWire I2Cinternal(0);

// SPI busses
DSPI controlSPI(3);      // used EUSCI_B3
DSPI lineTX(1);
DSPI lineRX(2);

SX1276Pins TXpins, RXpins;
SX1276 tx(controlSPI, &TXpins);
SX1276 rx(controlSPI, &RXpins);

COMMRadio commRadio(lineTX, lineRX, controlSPI, tx, rx);

// CDHS bus handler
PQ9Bus pq9bus(3, GPIO_PORT_P9, GPIO_PIN0);

// debug console handler
DSerial serial;
// services running in the system
HousekeepingService<COMMSTelemetryContainer> hk;
TestService tst;
PingService ping;

RadioService radioService(commRadio);

ResetService reset( GPIO_PORT_P5, GPIO_PIN0 );


SoftwareUpdateService SWUpdate;
Service* services[] = {&radioService, &hk, &ping, &reset, &SWUpdate, &tst };

// COMMS board tasks
PQ9CommandHandler cmdHandler(pq9bus, services, 5);
PeriodicTask timerTask(FCLOCK, periodicTask);
Task* tasks[] = { &cmdHandler, &timerTask, &commRadio };

// system uptime
unsigned long uptime = 0;

// TODO: remove when bug in CCS has been solved
void kickWatchdog(PQ9Frame &newFrame)
{
    cmdHandler.received(newFrame);
}

void validCmd(void)
{
    reset.kickInternalWatchDog();
}

void periodicTask()
{
    // increase the timer, this happens every second
    uptime++;

    // collect telemetry
    hk.acquireTelemetry(acquireTelemetry);

    // refresh the watch-dog configuration to make sure that, even in case of internal
    // registers corruption, the watch-dog is capable of recovering from an error
    reset.refreshConfiguration();

    // kick hardware watch-dog after every telemetry collection happens
    reset.kickExternalWatchDog();
}

void acquireTelemetry(COMMSTelemetryContainer *tc)
{
    // set uptime in telemetry
    tc->setUpTime(uptime);
}

void txcallback()
{
    tx.GPIO_IRQHandler();
}

void rxcallback()
{
    rx.GPIO_IRQHandler();
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
    // - initialize the pins for the hardware watch-dog
    // - prepare the pin for power cycling the system
    reset.init();

    // Initialize I2C masters
    I2Cinternal.setFastMode();
    I2Cinternal.begin();

    // Initialize SPI master
    controlSPI.initMaster(DSPI::MODE0, DSPI::MSBFirst, 1000000);


    serial.begin( );                        // baud rate: 9600 bps
    pq9bus.begin(115200, COMMS_ADDRESS);    // baud rate: 115200 bps
                                            // address COMMS (4)

    // link the command handler to the PQ9 bus:
    // every time a new command is received, it will be forwarded to the command handler
    // TODO: put back the lambda function after bug in CCS has been fixed
    //pq9bus.setReceiveHandler([](PQ9Frame &newFrame){ cmdHandler.received(newFrame); });
    pq9bus.setReceiveHandler(kickWatchdog);

    // every time a command is correctly processed, call the watch-dog
    // TODO: put back the lambda function after bug in CCS has been fixed
    //cmdHandler.onValidCommand([]{ reset.kickInternalWatchDog(); });
    cmdHandler.onValidCommand(validCmd);

    TXpins.CSPort = GPIO_PORT_P10;
    TXpins.CSPin = GPIO_PIN5;
    TXpins.DIO0Port = GPIO_PORT_P6;
    TXpins.DIO0Pin = GPIO_PIN1;
    TXpins.RESETPort = GPIO_PORT_P6;
    TXpins.RESETPin = GPIO_PIN0;
    TXpins.callback = txcallback;

    RXpins.CSPort = GPIO_PORT_P10;
    RXpins.CSPin = GPIO_PIN4;
    RXpins.DIO0Port = GPIO_PORT_P3;
    RXpins.DIO0Pin = GPIO_PIN1;
    RXpins.RESETPort = GPIO_PORT_P3;
    RXpins.RESETPin = GPIO_PIN0;
    RXpins.callback = rxcallback;

    tx.init();
    rx.init();

    serial.println("COMMS booting...");
    commRadio.init();
    serial.println("COMMS Booted.");
    serial.print("SOFTWARE VERSION:  ");
    serial.println(xstr(SW_VERSION));

    TaskManager::start(tasks, 3);
}
