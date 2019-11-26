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

void AX25Frame::setData(uint8_t data[], uint8_t size){
    this->setAdress(&data[0], &data[7]);
    this->setControl(data[14]);
    this->setPID(data[15]);
    this->setPacket(&data[16], size - 18);
    this->setFCS(&data[size-2]);
};

void AX25Frame::setFCS(uint8_t FCS[]){
    this->FrameBytes[16+packetSize] = FCS[0];
    this->FrameBytes[16+packetSize + 1] = FCS[1];
    this->FCSField = ( (uint16_t)  (FCS[1] << 8)) | FCS[0];
};

void AX25Frame::setAdress(uint8_t Destination[], uint8_t Source[]){
    for(int i = 0; i < 7; i++){
        this->addressField[i] = Destination[i];
        this->FrameBytes[i] = Destination[i];
    }
    for(int i = 0; i < 7; i++){
        this->addressField[7+i] = Source[i];
        this->FrameBytes[7+i] = Source[i];
    }

};

void AX25Frame::setControl(bool PF){
    if(PF){
        this->controlField = 0x13; //000 1 00 11
    }else{
        this->controlField = 0x03; //000 0 00 11
    }
    this->FrameBytes[14] = controlField;
};

void AX25Frame::setControl(uint8_t controlByte){
    this->controlField = controlByte; //000 1 00 11
    this->FrameBytes[14] = controlField;
};

void AX25Frame::setPID(uint8_t PIDByte){
    this->PIDField = PIDByte; //000 1 00 11
    this->FrameBytes[15] = PIDByte;
};


void AX25Frame::setPacket(uint8_t packet[], uint8_t size){
    for(int i = 0; i < size; i++){
        this->packetField[i] = packet[i];
        this->FrameBytes[16+i] = packet[i];
    }

    this->packetSize = size;
    this->FrameSize = size + 18;
};

void AX25Frame::calculateFCS(){
    //serial.println("calculating?");
    //fcs poly: 1 0001 0000 0010 0001  (17bits);

    //PKT -> FrameBytes
    bool guard;
    //fcs -> FCSBuffer[17]
    int tg;
    bool bit;

    for(int i = 0; i < 17; i++){
        FCSBuffer[i] = 1;
    }

    for(int k = 0; k < (this->FrameSize - 2 ); k++)
    {
        for (int i = 0; i < 8; i++)
        {
            guard = (FCSBuffer[15] != 0);
            for (tg = 14; tg > -1; tg--)
            {
                FCSBuffer[tg + 1] = FCSBuffer[tg];      // shift right
            }

            FCSBuffer[0] = 0;

            bit = (FrameBytes[k] & (1 << i)) != 0;
            if (bit != guard)
            {
                for (tg = 0; tg < 16; tg++)
                {
                    FCSBuffer[tg] = FCSBuffer[tg] ^ crc16[tg];
                }
            }
        }
    }
    //serial.println("CalculatedCRC: ");
    for (tg = 0; tg < 16; tg++)
    {
        FCSBuffer[tg] = 1 - FCSBuffer[tg];              // 1 -> 0 and 0 -> 1
    //    serial.print(FCSBuffer[tg],DEC);
    }

    FCSField = 0;
    for (int y = 0; y < 16; y++)
    {
        FCSField |= (FCSBuffer[8 + y] << (7 - y)) << 8;
        FCSField |= FCSBuffer[y] << (7 - y);
    }

    //Since FCS is send bit 15 first, and all other is send in octets with lsb first, the byte order is changed and the bytes are reversed
    this->FrameBytes[16+this->packetSize+1] = (this->FCSField & 0xFF);
    this->FrameBytes[16+this->packetSize] = (this->FCSField >> 8);
};

void AX25Frame::calculateFCS(uint8_t data[], uint8_t size){
    //fcs poly: 1 0001 0000 0010 0001  (17bits);

    //PKT -> FrameBytes
    bool guard;
    //fcs -> FCSBuffer[17]
    int tg;
    bool bit;

    for(int i = 0; i < 17; i++){
        FCSBuffer[i] = 1;
    }

    for(int k = 0; k < (size); k++)
    {
        for (int i = 0; i < 8; i++)
        {
            guard = (FCSBuffer[15] != 0);
            for (tg = 14; tg > -1; tg--)
            {
                FCSBuffer[tg + 1] = FCSBuffer[tg];      // shift right
            }

            FCSBuffer[0] = 0;

            bit = (data[k] & (1 << i)) != 0;
            if (bit != guard)
            {
                for (tg = 0; tg < 16; tg++)
                {
                    FCSBuffer[tg] = FCSBuffer[tg] ^ crc16[tg];
                }
            }
        }
    }
    //serial.println("CalculatedCRC: ");
    for (tg = 0; tg < 16; tg++)
    {
        FCSBuffer[tg] = 1 - FCSBuffer[tg];              // 1 -> 0 and 0 -> 1
    //    serial.print(FCSBuffer[tg],DEC);
    }

    FCSField = 0;
    for (int y = 0; y < 16; y++)
    {
        FCSField |= (FCSBuffer[8 + y] << (7 - y)) << 8;
        FCSField |= FCSBuffer[y] << (7 - y);
    }
    //Since FCS is send bit 15 first, and all other is send in octets with lsb first, the byte order is changed and the bytes are reversed
    this->FrameBytes[16+this->packetSize+1] = (this->FCSField & 0xFF);
    this->FrameBytes[16+this->packetSize] = (this->FCSField >> 8);
};

uint8_t * AX25Frame::getBytes(){
    return this->FrameBytes;
};

uint8_t AX25Frame::getSize(){
    return this->FrameSize;
}