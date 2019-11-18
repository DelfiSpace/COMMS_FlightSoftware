#include<COMMRadio.h>
#include "SX1276.h"
#include "DSerial.h"
#include <iostream>
#include <queue>

extern DSerial serial;

COMMRadio* radio;
uint8_t onTransmitWrapper(){
    return radio->onTransmit();
};
void onReceiveWrapper(uint8_t data){
    radio->onReceive(data);
};

COMMRadio::COMMRadio(DSPI &bitModeSPI_tx, DSPI &bitModeSPI_rx, DSPI &packetModeSPI, SX1276 &txRad, SX1276 &rxRad):
    bitSPI_tx(&bitModeSPI_tx), bitSPI_rx(&bitModeSPI_rx), packetSPI(&packetModeSPI), txRadio(&txRad), rxRadio(&rxRad)
{
    radio = this;
};



uint8_t COMMRadio::onTransmit()
{
    if(txReady && txCounter < txSize){
        return txBuffer[txCounter++];
    }else{
        txReady = false;
        return 0x00;
    }
};

void COMMRadio::onReceive(uint8_t data)
{
    //TODO: replace rxArray by a push-pop construction

    //TODO: when sequence detected. forward bytes to 'packet' receival
};

void COMMRadio::init(){

    serial.println("COMMS booting...");

    // Initialise TX values
    // GMSK:
    // Modem set to FSK, deviation set to 1/2 datarate, gaussian filter enabled
    if(txRadio->ping()){

        txConfig.modem = MODEM_FSK;
        txConfig.filtertype = BT_0_5;
        txConfig.power = 14;
        txConfig.fdev = 1200;
        txConfig.datarate = 2400;

        txRadio->setTxConfig(&txConfig);

        serial.println("TX Radio Settings Set");

        txRadio->enableBitMode(*bitSPI_tx, 0, onTransmitWrapper);
    }
    else{
        serial.println("TX Radio not Found");
    }

    // Initialise RX values
    // GMSK:
    // Modem set to FSK, deviation set to 1/2 datarate, gaussian filter enabled
    if(rxRadio->ping()){
        rxConfig.modem = MODEM_FSK;
        rxConfig.filtertype = BT_0_5;
        rxConfig.bandwidth = 15000;
        rxConfig.bandwidthAfc = 83333;
        rxConfig.fdev = 1200;
        rxConfig.datarate = 2400;

        rxRadio->setRxConfig(&rxConfig);

        serial.println("RX Radio Settings Set");

        rxRadio->enableBitMode(*bitSPI_rx, onReceiveWrapper, 0);
    }else{
        serial.println("RX Radio not Found");
    }
};

void COMMRadio::transmitData(uint8_t data[], uint8_t size){
    txSize = size;
    for(int i = 0; i < size; i++){
        txBuffer[i] = data[i];
    }
    txCounter = 0;
    txReady = true;
};

