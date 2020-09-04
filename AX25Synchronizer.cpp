#include "AX25Synchronizer.h"
#include "COMMRadio.h"

AX25Synchronizer::AX25Synchronizer(){
}

//void AX25Synchronizer::queBit(uint8_t inBit){
//    byteBuffer[byteBufferIndex] = inBit;
//    byteBufferIndex = (byteBufferIndex + 1)%BYTE_BUFFER_SIZE;
//}
bool AX25Synchronizer::queByte(uint8_t inByte){
    byteQue[this->byteQueIndex] = inByte;
    byteQueIndex = BitArray::mod(byteQueIndex+1, BYTE_QUE_SIZE);
    bytesInQue = bytesInQue + 1;

    if(bytesInQue > BYTE_QUE_SIZE){
        Console::log("[!!]");
    }

    return true;
}

PQPacket* AX25Synchronizer::rxBit(){
    if(bytesInQue <= 0){
        return 0;
    }
    uint8_t inByte = byteQue[BitArray::mod(byteQueIndex - bytesInQue, BYTE_QUE_SIZE)];
    bytesInQue = bytesInQue - 1;

    for(int i = 0; i < 8; i++){
        uint8_t inBit = encoder.NRZIdecodeBit(BitArray::getBit(&inByte, i, 8));
        inBit = encoder.descrambleBit(inBit);
        switch(this->synchronizerState){
            case 0:
                //flagDetect
                BitArray::setBit(&flagBuffer, flagBufferIndex, inBit == 0x01);
                flagBufferIndex = BitArray::mod(flagBufferIndex + 1, 8);

                if(BitArray::getByte(&flagBuffer, flagBufferIndex - 8, 8) == 0x7E ){
                    //Flag detected, wait for Msg Start
                    flagBuffer = 0;
                    flagBufferIndex = 0;

                    bitBufferIndex = 0;
                    bitBuffer[0] = 0;
                    synchronizerState = 1;
                }
                break;
            case 1:
                //checkForStart
                BitArray::setBit(bitBuffer, bitBufferIndex, inBit == 0x01);
                bitBufferIndex++;
                if(bitBufferIndex == 8){ //check if there's another flag or data.
                    if(bitBuffer[0] == 0x7E ){
                        // Another Flag
                        bitBuffer[0] = 0;
                        bitBufferIndex = 0;
                    }else{
                        // Start of Msg?!
                        synchronizerState = 2;
                    }
                }
                break;
            case 2:
                //receivingMsg
                BitArray::setBit(bitBuffer, bitBufferIndex, inBit == 0x01);
                bitBufferIndex++;

                //Detect End Flag!
                if(BitArray::getByte(bitBuffer, bitBufferIndex - 8, 8*BYTE_BUFFER_SIZE) == 0x7E ){
//                    Console::log("EndFlag Received!");

                    //get Size of Msg (minus the end flag)
                    int rxBits_n = bitBufferIndex - 8;

                    //Wether this turns out a package or not, next iteration we're back at detecting flags.
                    bitBufferIndex = 0;
                    synchronizerState = 0;

                    //Destuff the received BitSequence
                    rxBits_n = this->encoder.destuffBits(bitBuffer, receivedFrameBuffer.getBytes(), rxBits_n);

                    if(rxBits_n % 8 == 0){ //packet always has whole bytes!

                        //reverseOrder on all bytes
                        for(int p = 0; p < rxBits_n/8; p++){
                            this->receivedFrameBuffer.data[p] = AX25Frame::reverseByteOrder(this->receivedFrameBuffer.data[p]);
                        }

                        //set Packet Length
                        this->receivedFrameBuffer.packetSize = rxBits_n/8;

                        if(AX25Frame::checkFCS(this->receivedFrameBuffer)){
                            //FCS Hit! Packet Received!
//                            Console::log("    RX! (size:%d)%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %d %d %d %d %d %d %d %d %d %d %d", rxBits_n/8,
//                                     receivedFrameBuffer.getBytes()[0],
//                                     receivedFrameBuffer.getBytes()[1],
//                                     receivedFrameBuffer.getBytes()[2],
//                                     receivedFrameBuffer.getBytes()[3],
//                                     receivedFrameBuffer.getBytes()[4],
//                                     receivedFrameBuffer.getBytes()[5],
//                                     receivedFrameBuffer.getBytes()[6],
//                                     receivedFrameBuffer.getBytes()[7],
//                                     receivedFrameBuffer.getBytes()[8],
//                                     receivedFrameBuffer.getBytes()[9],
//                                     receivedFrameBuffer.getBytes()[10],
//                                     receivedFrameBuffer.getBytes()[11],
//                                     receivedFrameBuffer.getBytes()[12],
//                                     receivedFrameBuffer.getBytes()[13],
//                                     receivedFrameBuffer.getBytes()[14],
//                                     receivedFrameBuffer.getBytes()[15],
//                                     receivedFrameBuffer.getBytes()[16],
//                                     receivedFrameBuffer.getBytes()[17],
//                                     receivedFrameBuffer.getBytes()[18],
//                                     receivedFrameBuffer.getBytes()[19],
//                                     receivedFrameBuffer.getBytes()[20],
//                                     receivedFrameBuffer.getBytes()[21],
//                                     receivedFrameBuffer.getBytes()[22],
//                                     receivedFrameBuffer.getBytes()[23],
//                                     receivedFrameBuffer.getBytes()[24],
//                                     receivedFrameBuffer.getBytes()[25]);

                            return &receivedFrameBuffer;

                    } //else { Console::log("FCS Failed!"); }
                } //else { Console::log("No Whole Bytes !"); }
                if(bitBufferIndex > 253*8){
                    Console::log("No Msg! Too Long!");
                    synchronizerState = 0;
                }
                break;
            }
        }
    }

    return 0;
}
