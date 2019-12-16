#include "AX25Frame.h"

extern DSerial serial;

unsigned char AX25Frame::reverseByteOrder(unsigned char x)
{
    static const unsigned char table[] = {
        0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
        0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
        0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
        0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
        0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
        0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
        0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
        0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
        0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
        0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
        0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
        0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
        0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
        0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
        0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
        0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
        0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
        0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
        0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
        0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
        0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
        0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
        0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
        0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
        0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
        0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
        0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
        0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
        0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
        0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
        0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
        0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
    };
    return table[x];
}

//static void AX25Frame::setData(CLTUPacket packet, uint8_t data[], uint8_t size){
//    AX25Frame::setAdress(packet, &data[0], &data[7]);
//    AX25Frame::setControl(packet, data[14]);
//    AX25Frame::setPID(packet, data[15]);
//    AX25Frame::setPacket(packet, &data[16], size - 18);
//    AX25Frame::setFCS(packet, &data[size-2]);
//};

void AX25Frame::setFCS(CLTUPacket packet, uint8_t FCS[]){
    packet.data[packet.packetSize-2] = FCS[0];
    packet.data[packet.packetSize-1] = FCS[1];
    //AX25Frame::FCSField = ( (uint16_t)  (FCS[1] << 8)) | FCS[0];
};

void AX25Frame::setAdress(CLTUPacket packet, uint8_t Destination[], uint8_t Source[]){
    for(int i = 0; i < 7; i++){
        //AX25Frame::addressField[i] = Destination[i];
        packet.data[i] = Destination[i];
    }
    for(int i = 0; i < 7; i++){
        //AX25Frame::addressField[7+i] = Source[i];
        packet.data[i+7] = Source[i];
    }

};

void AX25Frame::setControl(CLTUPacket packet, bool PF){
    if(PF){
        //AX25Frame::controlField = 0x13; //000 1 00 11
        packet.data[14] = 0x13;
    }else{
        //AX25Frame::controlField = 0x03; //000 0 00 11
        packet.data[14] = 0x03;
    }
    //AX25Frame::FrameBytes[14] = controlField;
};

void AX25Frame::setControl(CLTUPacket inpacket, uint8_t controlByte){
    //AX25Frame::controlField = controlByte; //000 1 00 11
    inpacket.data[14] = controlByte;//controlField;
};

void AX25Frame::setPID(CLTUPacket inpacket, uint8_t PIDByte){
    //AX25Frame::PIDField = PIDByte; //000 1 00 11
    inpacket.data[15] = PIDByte;
};


void AX25Frame::setInfoPacket(CLTUPacket framepacket, uint8_t packet[], uint8_t size){
    for(int i = 0; i < size; i++){
        //AX25Frame::packetField[i] = packet[i];
        framepacket.data[16+i] = packet[i];
    }

    //AX25Frame::packetSize = size;
    framepacket.packetSize = size + 18;
};

void AX25Frame::calculateFCS(CLTUPacket framepacket){
    //serial.println("calculating?");
    //fcs poly: 1 0001 0000 0010 0001  (17bits);

    //PKT -> FrameBytes
    bool guard;
    bool bit;

    uint16_t FCSBuff = 0xFFFF;

    for(int k = 0; k < (framepacket.packetSize-2); k++)
    {
        for (int i = 0; i < 8; i++)
        {
            guard = (FCSBuff & 0x01) != 0;
            FCSBuff = FCSBuff >> 1;     //Shift right
            FCSBuff = FCSBuff & 0x7FFF; //Set most left bit to zero;
            bit = (framepacket.data[k] & (1 << i)) != 0;
            if (bit != guard)
            {
                FCSBuff = FCSBuff ^ 0x8408;
            }
        }
    }
    FCSBuff = FCSBuff ^ 0xFFFF;
    uint8_t FCSByte1 = ((uint8_t) (FCSBuff & 0x00FF));
    uint8_t FCSByte2 = ((uint8_t) ((FCSBuff & 0xFF00) >> 8));
    //FCSField = 0;
    //FCSField |= FCSByte2;
    //FCSField |= FCSByte1 << 8;
    //Since FCS is send bit 15 first, and all other is send in octets with lsb first, the byte order is changed and the bytes are reversed
    framepacket.data[framepacket.packetSize-2] = FCSByte1;
    framepacket.data[framepacket.packetSize-1] = FCSByte2;
};


bool AX25Frame::checkFCS(CLTUPacket framepacket){
    //serial.println("calculating?");
    //fcs poly: 1 0001 0000 0010 0001  (17bits);

    //PKT -> FrameBytes
    bool guard;
    bool bit;

    uint16_t FCSBuff = 0xFFFF;

    for(int k = 0; k < (framepacket.packetSize-2); k++)
    {
        for (int i = 0; i < 8; i++)
        {
            guard = (FCSBuff & 0x01) != 0;
            FCSBuff = FCSBuff >> 1;     //Shift right
            FCSBuff = FCSBuff & 0x7FFF; //Set most left bit to zero;
            bit = (framepacket.data[k] & (1 << i)) != 0;
            if (bit != guard)
            {
                FCSBuff = FCSBuff ^ 0x8408;
            }
        }
    }
    FCSBuff = FCSBuff ^ 0xFFFF;
    uint8_t FCSByte1 = ((uint8_t) (FCSBuff & 0x00FF));
    uint8_t FCSByte2 = ((uint8_t) ((FCSBuff & 0xFF00) >> 8));
    //FCSField = 0;
    //FCSField |= FCSByte2;
    //FCSField |= FCSByte1 << 8;
//    serial.print("Calculated CRC: ");
//    serial.print(FCSByte1, HEX);
//    serial.print(FCSByte2, HEX);
//    serial.println();
//    serial.print("Received CRC: ");
//    serial.print(FrameBytes[FrameSize-2], HEX);
//    serial.print(FrameBytes[FrameSize-1], HEX);
//    serial.println();
    if( FCSByte1 == framepacket.data[framepacket.packetSize-2] && FCSByte2 == framepacket.data[framepacket.packetSize-1]){
        return true;
    }else{
        return false;
    }
}

uint8_t AX25Frame::getPacketSize(CLTUPacket packet){
    return packet.packetSize - 18;
}
