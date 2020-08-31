#include "COMMS.h"

// I2C busses
DWire I2Cinternal(0);
INA226 powerBus(I2Cinternal, 0x40);
TMP100 TCXOtemperature(I2Cinternal, 0x4F);

// control SPI bus
DSPI controlSPI(3);      // used EUSCI_B3
DSPI lineTX(1);
DSPI lineRX(2);

// FRAM
MB85RS fram(controlSPI, GPIO_PORT_P10, GPIO_PIN0, MB85RS::MB85RS1MT );

SX1276Pins TXpins, RXpins;
SX1276 tx(controlSPI, &TXpins);
SX1276 rx(controlSPI, &RXpins);

COMMRadio commRadio(lineTX, lineRX, controlSPI, tx, rx);

// HardwareMonitor
HWMonitor hwMonitor(&fram);

// Bootloader
Bootloader bootLoader = Bootloader(fram);

// CDHS bus handler
PQ9Bus pq9bus(3, GPIO_PORT_P9, GPIO_PIN0);

// services running in the system
HousekeepingService<COMMSTelemetryContainer> hk;
TestService test;
PingService ping;
ResetService reset(GPIO_PORT_P8, GPIO_PIN0, GPIO_PORT_P8, GPIO_PIN1 );
#ifndef SW_VERSION
SoftwareUpdateService SWupdate(fram);
#else
SoftwareUpdateService SWupdate(fram, (uint8_t*)xtr(SW_VERSION));
#endif

RadioService radioService(commRadio);

ResetService reset( GPIO_PORT_P5, GPIO_PIN0 );


//SoftwareUpdateService SWUpdate;
Service* services[] = {&radioService, &hk, &ping, &reset, &tst };//{&radioService, &hk, &ping, &reset, &SWUpdate, &tst };

// COMMS board tasks
CommandHandler<PQ9Frame,PQ9Message> cmdHandler(pq9bus, services, 5);
PeriodicTask timerTask(1000, periodicTask);
PeriodicTask* periodicTasks[] = {&timerTask};
PeriodicTaskNotifier taskNotifier = PeriodicTaskNotifier(periodicTasks, 1);

CommandHandler<PQ9Frame> cmdHandler(pq9bus, services, 5);
Task* tasks[] = { &cmdHandler, &timerTask, &commRadio};

// system uptime
unsigned long uptime = 0;

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
    unsigned short v;
    signed short i, t;

    // set uptime in telemetry
    tc->setUpTime(uptime);

    powerBus.getVoltage(v);
    powerBus.getCurrent(i);
    TCXOtemperature.getTemperature(t);
    // measure the power bus
    //tc->setBusStatus((!powerBus.getVoltage(v)) & (!powerBus.getCurrent(i)));
    //tc->setBusVoltage(v);
    //tc->setBusCurrent(i);
    //Console::log("Bus Voltage: %d mV", v);
    //Console::log("Bus current: %d mA", i);

    // measure the MCU temperature
    //tc->setMCUTemperature(hwMonitor.getMCUTemp());
    //Console::log("TCXO Temperature: %d", t);
    //Console::log("MCU Temperature: %d", hwMonitor.getMCUTemp());
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

    // initialize the ADC
    // - ADC14 and FPU Module
    // - MEM0 for internal temperature measurements
    ADCManager::initADC();

    // Initialize SPI master
    controlSPI.initMaster(DSPI::MODE0, DSPI::MSBFirst, 1000000);
    fram.init();

    Console::init( 115200 );                // baud rate: 115200 bps
    pq9bus.begin(115200, COMMS_ADDRESS);    // baud rate: 115200 bps
                                            // address COMMS (4)

    // InitBootLoader!
    bootLoader.JumpSlot();

    // initialize the reset handler:
    // - prepare the watch-dog
    // - initialize the pins for the hardware watch-dog
    // - prepare the pin for power cycling the system
    reset.init();

    // initialize Task Notifier
    taskNotifier.init();

    // initialize HWMonitor readings
    hwMonitor.readResetStatus();
    hwMonitor.readCSStatus();

    // Initialize I2C masters
    I2Cinternal.setFastMode();
    I2Cinternal.begin();

    // Initialize SPI master
    controlSPI.initMaster(DSPI::MODE0, DSPI::MSBFirst, 1000000);

    Console::init( 115200 );                        // baud rate: 9600 bps
    pq9bus.begin(115200, COMMS_ADDRESS);    // baud rate: 115200 bps
                                            // address COMMS (4)

    // link the command handler to the PQ9 bus:
    // every time a new command is received, it will be forwarded to the command handler
    pq9bus.setReceiveHandler([](DataFrame &newFrame){ cmdHandler.received(newFrame); });

    // every time a command is correctly processed, call the watch-dog
    cmdHandler.onValidCommand([]{ reset.kickInternalWatchDog(); });

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

    Console::log("COMMS booting...SLOT: %d", (int) Bootloader::getCurrentSlot());

    if(HAS_SW_VERSION == 1)
    {
        Console::log("SW_VERSION: %s", (const char*)xtr(SW_VERSION));
    }

    TaskManager::start(tasks, 3);
}
