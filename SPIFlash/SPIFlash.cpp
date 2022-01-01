#include "SPIFlash.hpp"

//SPI settings
char SPI_MODE = 0;
BitOrder BIT_ORDER = BitOrder::MSBFIRST;

//Buffers
char * OUTBUFFER;
int OUTBUFFER_LENGTH;

char ADDBUFFER[SPIFLASH_ADDBUF_LENGTH];
char WRITEBUFFER[SPIFLASH_WRITEBUF_LENGTH];

//Status Register Output Byte
char STATUSREGISTER = 0;

//Pinout
int CHIP_ENABLE = 0;
int CLOCK = 0;
int DATA_OUT = 0;
int DATA_IN = 0;


SPIFlash::SPIFlash(){};

void SPIFlash::Init(int ce, int clk, int dout, int din, char mode, char bitorder)
{
    CHIP_ENABLE = ce;
    CLOCK = clk;
    DATA_OUT = dout;
    DATA_IN = din;
    SPI_MODE = mode;
    BIT_ORDER = (bitorder == 0) ? BitOrder::MSBFIRST : BitOrder::LSBFIRST;

    pinMode(CHIP_ENABLE, OUTPUT);
    digitalWrite(CHIP_ENABLE, HIGH);
    SPI.begin();
    SPI.setBitOrder(BIT_ORDER);
    SPI.setDataMode(SPI_MODE);

}

//Point to the output char array for data read from SPI flash
void SPIFlash::SetOutputBuffer(char * buf, int length)
{
    OUTBUFFER= buf;
    OUTBUFFER_LENGTH = length;
}

//Copies bytes from input array into write buffer
//Protected from buffer overflow
void SPIFlash::FillWriteBuffer(char * input,int start, int length, int insize)
{
    length = (length > SPIFLASH_WRITEBUF_LENGTH) ? SPIFLASH_WRITEBUF_LENGTH : length;
    length = (start+length > insize) ? length - ((start+length) - insize) : length;
    memcpy(WRITEBUFFER, input+start, length);
}

//Simple util function to reset arbitrary length buffer
void SPIFlash::ClearBuffer(char * buf, int length)
{
    memset(buf, 0, length);
}

//Sends long address to ADDBUFFER with most significant byte first
void SPIFlash::Int32AddressToBytes(long address)
{
    long temp = address << 24;
    temp = temp >> 24;
    char A0A7 = temp;

    temp = address << 16;
    temp = temp >> 24;
    char A8A15 = temp;

    temp = address << 8;
    temp = temp >> 24;
    char A16A23 = temp;

    ADDBUFFER[0] = A16A23;
    ADDBUFFER[1] = A8A15;
    ADDBUFFER[2] = A0A7;
}

char SPIFlash::Read(long start, int length)
{
    //Clear buffers
    ClearBuffer(OUTBUFFER, OUTBUFFER_LENGTH);
    ClearBuffer(ADDBUFFER, SPIFLASH_ADDBUF_LENGTH);

    //Decode long address into MSB fisrt byte array
    Int32AddressToBytes(start);

    //Select/Enable SPI slave
    digitalWrite(CHIP_ENABLE, LOW);

    //Send read command
    SPI.transfer(OP_RD);

    //Send address
    SPI.transfer(*ADDBUFFER);
    SPI.transfer(*(ADDBUFFER+1));
    SPI.transfer(*(ADDBUFFER+2));

    //Read out data
    for(int i=0; i < length; i++)
    {
        *(OUTBUFFER+i) = SPI.transfer(0x00);
    }

    //Deselect SPI slave
    digitalWrite(CHIP_ENABLE, HIGH);

    return 0;
}

//Read (update) status register byte value
void SPIFlash::ReadStatusRegister()
{
    //Select/Enable SPI slave
    digitalWrite(CHIP_ENABLE, LOW);

    //Send command
    SPI.transfer(OP_RDSR);

    //Read status register
    STATUSREGISTER = SPI.transfer(0x00);

    //Deselect SPI slave
    digitalWrite(CHIP_ENABLE, HIGH);
}

//Returns a specific status register flag
//If this function returns 0 the flag is OFF (0)
//Any other return value means the flag is ON (1)
char SPIFlash::ReadStatusFlag(char flag)
{
    return (flag & STATUSREGISTER) > 0 ? 1 : 0;
}

//Erase 4Kbyte sector
void SPIFlash::SectorErase(long address)
{
    //Clear address buffer
    ClearBuffer(ADDBUFFER, SPIFLASH_ADDBUF_LENGTH);

    //Decode address
    Int32AddressToBytes(address);

    //Write enable procedure before writing
    WriteEnable(); 

    //Select/Enable SPI slave
    digitalWrite(CHIP_ENABLE, LOW);

    //Send sector erase command
    SPI.transfer(OP_SER);

    //Send address of sector
    SPI.transfer(ADDBUFFER[0]);
    SPI.transfer(ADDBUFFER[1]);
    SPI.transfer(ADDBUFFER[2]);

    //Deselect SPI slave to start execution
    digitalWrite(CHIP_ENABLE, HIGH);
}

//Erase 32Kbyte block
void SPIFlash::BlockErase32K(long address)
{
    //Clear address buffer
    ClearBuffer(ADDBUFFER, SPIFLASH_ADDBUF_LENGTH);

    //Decode address
    Int32AddressToBytes(address);

    //Write enable procedure before writing
    WriteEnable(); 

    //Select/Enable SPI slave
    digitalWrite(CHIP_ENABLE, LOW);

    //Send block erase command
    SPI.transfer(OP_BER32);

    //Send address of block
    SPI.transfer(ADDBUFFER[0]);
    SPI.transfer(ADDBUFFER[1]);
    SPI.transfer(ADDBUFFER[2]);

    //Deselect SPI slave. Chip will begin erase operation.
    digitalWrite(CHIP_ENABLE, HIGH);
}

//Erase 64KByte block
void SPIFlash::BlockErase64K(long address)
{
    //Clear address buffer
    ClearBuffer(ADDBUFFER, SPIFLASH_ADDBUF_LENGTH);

    //Decode address
    Int32AddressToBytes(address);

    //Write enable procedure before writing
    WriteEnable(); 

    //Select/Enable SPI slave
    digitalWrite(CHIP_ENABLE, LOW);

    //Send block erase command
    SPI.transfer(OP_BER64);

    //Send address of block
    SPI.transfer(ADDBUFFER[0]);
    SPI.transfer(ADDBUFFER[1]);
    SPI.transfer(ADDBUFFER[2]);

    //Deselect SPI slave. Chip will begin erase operation.
    digitalWrite(CHIP_ENABLE, HIGH);
}

//Full chip erase
void SPIFlash::ChipErase()
{
    //Write enable procedure before writing
    WriteEnable(); 

    //Select/Enable SPI slave
    digitalWrite(CHIP_ENABLE, LOW);

    //Send chip erase command
    SPI.transfer(OP_CER);

    //Deselect SPI slave. Chip will begin erase operation.
    digitalWrite(CHIP_ENABLE, HIGH);

}

//Program up to 256 bytes starting at address
void SPIFlash::PageProgram(long address, short length)
{
    //Cap write length to 256 to avoid address rollover
    length = (length <= SPIFLASH_WRITEBUF_LENGTH) ? length : SPIFLASH_WRITEBUF_LENGTH;

    //Decode address
    Int32AddressToBytes(address);

    //Write enable procedure before writing
    WriteEnable();   

    //Select/Enable SPI slave again for write operation
    digitalWrite(CHIP_ENABLE, LOW);

    //Send Page Program command
    SPI.transfer(OP_PP);

    //Send three address bytes
    SPI.transfer(ADDBUFFER[0]);
    SPI.transfer(ADDBUFFER[1]);
    SPI.transfer(ADDBUFFER[2]);

    //Send data to write
    for(int i=0; i < length; i++)
    {
        SPI.transfer(WRITEBUFFER[i]);
    }

    //Deselect SPI slave. Chip will begin write operation. 
    //Check Status Register to know when write is finished.
    digitalWrite(CHIP_ENABLE, HIGH);

}

//Returns sector start address for particular sector ID
long SPIFlash::GetSectorStartAddress(int id)
{
    long add = id * ADD_SECTORSIZE;
    return add;
}

//Returns sector start address for particular sector ID
long SPIFlash::GetBlock32KStartAddress(int id)
{
    long add = id * ADD_BLOCKSIZE32K;
    return add;
}

//Returns sector start address for particular sector ID
long SPIFlash::GetBlock64KStartAddress(int id)
{
    long add = id * ADD_BLOCKSIZE64K;
    return add;
}

//Read ID values into output buffer for quickly checking if chip works properly
void SPIFlash::ReadDeviceIdentifierValues(long address)
{
    //Clear address and output buffers
    ClearBuffer(ADDBUFFER, SPIFLASH_ADDBUF_LENGTH);
    ClearBuffer(OUTBUFFER, OUTBUFFER_LENGTH);

    //Send address to buffer
    Int32AddressToBytes(address);

    //Select/Enable SPI slave
    digitalWrite(CHIP_ENABLE, LOW);

    //Send RMDID command
    SPI.transfer(OP_RDMDID);

    //3 bytes of address where only first byte matters (0 = ManID first, 1 = DevID first)
    SPI.transfer(ADDBUFFER[0]);
    SPI.transfer(ADDBUFFER[1]);
    SPI.transfer(ADDBUFFER[2]);

    //2 transfers to receive values
    *(OUTBUFFER) = SPI.transfer(0x00);
    *(OUTBUFFER+1) = SPI.transfer(0x00);

    //Deselect SPI slave
    digitalWrite(CHIP_ENABLE, HIGH);
    
}

//Turns on the WEL (Write Enable Latch) before writing/erase instructions
void SPIFlash::WriteEnable()
{
    //Select/Enable SPI slave for write enable instruction
    digitalWrite(CHIP_ENABLE, LOW);

    //Send write enable instruction
    SPI.transfer(OP_WREN);

    //Deselect SPI slave to send write enable instruction
    digitalWrite(CHIP_ENABLE, HIGH);
}