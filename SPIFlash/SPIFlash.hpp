//ISSI SPI Flash Library
#include <Arduino.h>
#include <SPI.h>

#define SPIFLASH_ADDBUF_LENGTH 3
#define SPIFLASH_WRITEBUF_LENGTH 256

//SPI flash hex opcodes
const char OP_RD =      0x03;//Normal read
const char OP_FR =      0xB;//Fast read
const char OP_FRDIO =   0xBB;//Fast read dual IO
const char OP_FRDO =    0x3B;//Fast read dual output
const char OP_FRQUIO =  0xEB;//Fast read quad IO
const char OP_FRQO =    0x6B;//Fast Read Quad Output
const char OP_PP =      0x02; //Page Program Data Bytes into Memory 
const char OP_PPQ =     0x32;//38h Page Program Data Bytes into Memory with Quad Interface 
const char OP_SER =     0xD7;//20h Sector Erase 4KB 
const char OP_BER32 =   0x52;// Block Erase 32KB 
const char OP_BER64 =   0xD8;// Block Erase 64KB 
const char OP_CER =     0xC7;//60h Chip Erase 
const char OP_WREN =    0x06;// Write Enable 
const char OP_WRDI =    0x04;// Write Disable 
const char OP_RDSR =    0x05;// Read Status Register
const char OP_WRSR =    0x01;// Write Status Register
const char OP_RDFR =    0x48;// Read Function Register
const char OP_WRFR =    0x42;// Write Function Register
const char OP_PERSUS =  0x75;///B0h Suspend during the Program/Erase
const char OP_PERRSM =  0x7A;///30h Resume Program/Erase
const char OP_DP =      0xB9;//Deep Power Down Mode
const char OP_RDID =    0xAB;// Read Manufacturer and Product ID
const char OP_RDPD =    0xAB;// Release Deep Power Down
const char OP_RDUID  =  0x4B;// Read Unique ID Number
const char OP_RDJDID  = 0x9F;// Read Manufacturer and Product ID by JEDEC ID Command
const char OP_RDMDID =  0x90;// Read Manufacturer and Device ID
const char OP_RDSFDP =  0x5A;// SFDP Read
const char OP_RSTEN =   0x66;// Software Reset Enable
const char OP_RST =     0x99;//Reset
const char OP_IRRD =    0x62;//Program Information Row
const char OP_IRPD =    0x68;//Read Information Row
const char OP_SECUNLOCK=0x26;//Sector Unlock
const char OP_SECLOCK = 0x24;//Sector Lock

//Address space specs
const long ADD_TOTALSIZE =      0x1FFFF+1;//128KBytes = 1 Mbit = 1,024,000 Bits
const long ADD_BLOCKSIZE64K =   0xFFFF+1;
const long ADD_BLOCKSIZE32K =   0x7FFF+1; 
const long ADD_SECTORSIZE =     0xFFF+1;

//Status Register Bit Map
const char STATUS_WRITE_IN_PROGRESS =         B00000001;
const char STATUS_WRITE_ENABLE_LATCH =        B00000010;
const char STATUS_BLOCK_PROTECT_0 =           B00000100;
const char STATUS_BLOCK_PROTECT_1 =           B00001000;
const char STATUS_BLOCK_PROTECT_2 =           B00010000;
const char STATUS_BLOCK_PROTECT_3 =           B00100000;
const char STATUS_QUAD_ENABLE =               B01000000;
const char STATUS_REG_WRITE_DISABLE =         B10000000;

//Bit Orders
const char ORDER_MSBFIRST = 0;
const char ORDER_LSBFIRST = 1;

class SPIFlash
{
    public:

        //Constructor
        SPIFlash();

        //Functions
        void Init(int ce, int clk, int dout, int din, char mode, char bitorder);
        void SetOutputBuffer(char * buf, int length);
        void FillWriteBuffer(char * input, int start, int length, int insize);
        void ClearBuffer(char * buf, int length);
        char Read(long address, int length);
        void SectorErase(long address);
        void BlockErase32K(long address);
        void BlockErase64K(long address);
        void ChipErase();
        void ReadStatusRegister();
        char ReadStatusFlag(char flag);
        void PageProgram(long address, short length);
        long GetSectorStartAddress(int id);
        long GetBlock32KStartAddress(int id);
        long GetBlock64KStartAddress(int id);
        void ReadDeviceIdentifierValues(long address);
        void WriteEnable();

        //Public Vars
        char WRITEBUFFER[SPIFLASH_WRITEBUF_LENGTH];

    private:

        //Pinout
        int CHIP_ENABLE;
        int CLOCK;
        int DATA_OUT;
        int DATA_IN;

        //SPI settings
        char SPI_MODE;
        BitOrder BIT_ORDER;

        //Output buffer
        char * OUTBUFFER;
        int OUTBUFFER_LENGTH;

        char ADDBUFFER[SPIFLASH_ADDBUF_LENGTH];

        //Private functions
        void Int32AddressToBytes(long address);
        
};



