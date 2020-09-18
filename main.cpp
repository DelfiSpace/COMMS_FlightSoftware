#include "COMMS.h"

// I2C busses
DWire I2Cinternal(0);
INA226 powerBus(I2Cinternal, 0x40);
INA226 transmitPower(I2Cinternal, 0x41);
INA226 amplifierPower(I2Cinternal, 0x42);
TMP100 CommsTemperature(I2Cinternal, 0x4F);
TMP100 phasingTemperature(I2Cinternal, 0x4C);
TMP100 amplifierTemperature(I2Cinternal, 0x4B);


// control SPI bus
DSPI controlSPI(3);      // used EUSCI_B3
DSPI lineTX(1);
DSPI lineRX(2);

// FRAM
MB85RS fram(controlSPI, GPIO_PORT_P10, GPIO_PIN0, MB85RS::MB85RS1MT );

SX1276Pins TXpins, RXpins;
SX1276 tx(controlSPI, &TXpins);
SX1276 rx(controlSPI, &RXpins);

// HardwareMonitor
HWMonitor hwMonitor(&fram);

// Bootloader
Bootloader bootLoader = Bootloader(fram);

// CDHS bus handler
PQ9Bus pq9bus(3, GPIO_PORT_P9, GPIO_PIN0);
BusMaster<PQ9Frame, PQ9Message> busHandler(pq9bus);

// Radio Object
COMMRadio commRadio(lineTX, lineRX, controlSPI, tx, rx);

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


//SoftwareUpdateService SWUpdate;
Service* services[] = {&radioService, &hk, &ping, &reset, &test, &SWupdate };//{&radioService, &hk, &ping, &reset, &SWUpdate, &tst };

// COMMS board tasks
CommandHandler<PQ9Frame,PQ9Message> cmdHandler(pq9bus, services, 6);
InternalCommandHandler<PQ9Frame,PQ9Message> internalCmdHandler(services, 6);
PeriodicTask timerTask(1000, periodicTask);
PeriodicTask* periodicTasks[] = {&timerTask};
PeriodicTaskNotifier taskNotifier = PeriodicTaskNotifier(periodicTasks, 1);

Task* tasks[] = { &cmdHandler, &timerTask, &commRadio};

// system uptime
unsigned long uptime = 0;
FRAMVar<unsigned long> totalUptime;


void receivedCommand(DataFrame &newFrame)
{
    if(!busHandler.received(newFrame)){
        cmdHandler.received(newFrame);
    }
}

void periodicTask()
{
    // increase the timer, this happens every second
    totalUptime += 1;
    uptime += 1;

    // collect telemetry
    hk.acquireTelemetry(acquireTelemetry);

    // refresh the watch-dog configuration to make sure that, even in case of internal
    // registers corruption, the watch-dog is capable of recovering from an error
    reset.refreshConfiguration();

    // kick hardware watch-dog after every telemetry collection happens
    reset.kickExternalWatchDog();

//    signed short rssi = rx.GetRssi(ModemType::MODEM_FSK);
//    if(rssi < 0)
//    {
//        Console::log("Received Command! Current RSSI: -%d dBm", -rssi);
//    }
//    else
//    {
//        Console::log("Received Command! Current RSSI: %d dBm", rssi);
//    }
}

void acquireTelemetry(COMMSTelemetryContainer *tc)
{
    unsigned short v, c;
    signed short i, t;
    unsigned char uc;
    unsigned long ul;
    //Set Telemetry:

    //HouseKeeping Header:
    tc->setStatus(Bootloader::getCurrentSlot());
    fram.read(FRAM_RESET_COUNTER + Bootloader::getCurrentSlot(), &uc, 1);
    tc->setBootCounter(uc);
    tc->setResetCause(hwMonitor.getResetStatus());
    tc->setUptime(uptime);
    tc->setTotalUptime((unsigned long) totalUptime);
    tc->setVersionNumber(2);
    tc->setMCUTemp(hwMonitor.getMCUTemp());

    //main board sensors
    tc->setINAStatus(!(powerBus.getVoltage(v)) & !(powerBus.getCurrent(i)));
    tc->setVoltage(v);
    tc->setCurrent(i);

    tc->setTMPStatus(!(CommsTemperature.getTemperature(t)));
    tc->setTemperature(t);
    tc->setTransmitINAStatus(!(transmitPower.getVoltage(v)) & !(transmitPower.getCurrent(i)));
    tc->setTransmitVoltage(v);
    tc->setTransmitCurrent(i);

    //Phasingboard
    tc->setPhasingTMPStatus(!phasingTemperature.getTemperature(t));
    tc->setPhasingTemperature(t);

    //PABoard
    tc->setAmplifierINAStatus(!(amplifierPower.getVoltage(v)) & !(amplifierPower.getCurrent(i)));
    tc->setAmplifierVoltage(v);
    tc->setAmplifierCurrent(i);
    tc->setAmplifierTMPStatus(!amplifierTemperature.getTemperature(t));
    tc->setAmplifierTemperature(t);
    if(!tc->getAmplifierTMPStatus() && t > 400){
        Console::log("PA TOO HOT!!! TURN OFF");
        commRadio.disablePA();
    }


    i = rx.GetRssi(ModemType::MODEM_FSK);
//    Console::log("-%d dBm", -i);
    tc->setReceiverRSSI(i);
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
 *
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

    // Initialize fram and fram-variables
    fram.init();
    totalUptime.init(fram, FRAM_TOTAL_UPTIME);


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

    // initialize the shunt resistors
    powerBus.setShuntResistor(33);
    transmitPower.setShuntResistor(33);
    amplifierPower.setShuntResistor(33);



    // Initialize SPI master
    controlSPI.initMaster(DSPI::MODE0, DSPI::MSBFirst, 1000000);

    Console::init( 115200 );                        // baud rate: 9600 bps
    pq9bus.begin(115200, COMMS_ADDRESS);    // baud rate: 115200 bps
                                            // address COMMS (4)

    // link the command handler to the PQ9 bus:
    // every time a new command is received, it will be forwarded to the command handler
    pq9bus.setReceiveHandler( &receivedCommand );

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

    commRadio.init();
    commRadio.setcmdHandler(internalCmdHandler);
    commRadio.setbusMaster(busHandler);

    Console::log("COMMS booting...SLOT: %d", (int) Bootloader::getCurrentSlot());

    if(HAS_SW_VERSION == 1)
    {
        Console::log("SW_VERSION: %s", (const char*)xtr(SW_VERSION));
    }

    TaskManager::start(tasks, 3);
}
