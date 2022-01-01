# STM32 Arduino IS25LQ010B SPI Flash Interface
Arduino Library for SPI Flash ([IS25LQ010B](https://www.issi.com/WW/pdf/25LQ025B-512B-010B-020B-040B.pdf))

Written for use on a Weact Blackpill STM32 Board. 
 
# Read Before Using
This is not a universal SPI flash library and many instructions are unimplemented.

This library will only work properly as is with the IS25LQ010B flash chip or equivalent SPI Flash chips.

It should be simple to edit the code to work with other capacity/models by following datasheet.

# Install Process
Drag and drop "SPIFlash" folder into your Arduino Sketchbook Library Folder.

For portable IDE : %IDEPATH%/portable/sketchbook/libraries

# Usage 
Start by creating a SPIFlash variable by calling the empty constructor.

`SPIFlash flash = SPIFlash();`

Here is a list of implemented functions:

## Required Setup

- `void Init(int ce, int clk, int dout, int din, char mode, char bitorder);`

Sets up the Chip Enable, Clock, Data Out, Data In pins as well as SPI mode and Bit Order

- `void SetOutputBuffer(char * buf, int length);`

Set the output buffer for bytes read out of Flash.

## Reading

- `char Read(long address, int length);`

Read out Flash memory starting at address for length bytes into specified output buffer

- `void ReadStatusRegister();`

Used to read (update) the status register byte to get new flags

- `char ReadStatusFlag(char flag);`

Used to check the state of a single flag. Check datasheet or header file for flag map.

Function will return 0 or 1 based on state of flag being checked.

- `void ReadDeviceIdentifierValues(long address);`

Used to read out chip ID values, could be used to detect capacity and make library universal.

## Writing 

NOTE: Chip cannot turn a 0 into a 1, must erase entire sector at minimum to do so

- `void WriteEnable();`

This function is called before any write or erase operation.

Made available but not necessary as the respective program and erase functions call it.

- `void FillWriteBuffer(char * input, int start, int length, int insize);`

Fill write buffer with data from input pointer, specify correct input buffer size to avoid overflow.

- `void PageProgram(long address, short length);`

Main Write Operation, write up to 256 bytes to memory starting at set address.

## Erasing

- `void SectorErase(long address);`

Erases a 4 KByte sector.

- `void BlockErase32K(long address);`

Erases a 32 KByte block.

- `void BlockErase64K(long address);`

Erases a 64 KByte block

- `void ChipErase();`

NOTE : All bits are set to 1 during erase operation

NOTE : All chip erase operation work according to the Sector and Block map set in the datasheet.

## Misc

- `void ClearBuffer(char * buf, int length);`

Clears buffer, memset to 0 for length

- `long GetSectorStartAddress(int id);`

Get sector (4 KB) start address

- `long GetBlock32KStartAddress(int id);`

Get block (32 KB) start address

- `long GetBlock64KStartAddress(int id);`

Get block (64 KB) start address

# More Info

Refer to the chip datasheet or code comments for help with unimplemented operations or adapting the library for other chips.




