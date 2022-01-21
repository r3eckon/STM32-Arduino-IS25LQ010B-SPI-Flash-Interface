#include <Arduino.h>
#include <SPI.h>
#include <SPIFlash.hpp>

const int CHIP_ENABLE = PA4;
const int CLOCK = PA5;
const int DATA_OUT = PA7;
const int DATA_IN = PA6;

const char SPIMODE = SPI_MODE0;

byte ManID, DevID;

SPIFlash flash = SPIFlash();

char * tok;
String cmdString;
String tempString;

bool waiting=false;

char cmd_full[1024];
char cmd_main[24];
char cmd_args[1000];

char FlashOutput[256];

int arg_add;
int arg_len;
char arg_val;

char WriteBuf[256];

void setup() {
  delay(100);
  Serial.begin(115200);
  delay(500);
  Serial.println("Serial Console Started...");
  delay(500);
  flash.Init(CHIP_ENABLE, CLOCK, DATA_OUT, DATA_IN, SPIMODE,0);
  flash.SetOutputBuffer(FlashOutput, 256);
  Serial.println("SPI Flash Library Init Done.");
  delay(500);
  Serial.println("Waiting for command (\"help\" for list)...");
  
}

void cmd_help()
{
  Serial.println("SPI Flash Console Help Page.\n\nCommand\t\tArgs\t\tInfo");
  Serial.println("help\t\tNA\t\tShow this page");
  Serial.println("getid\t\tAdd\t\tRead Manufacturer and Device ID\n\t\t\t\tAdd : 0 or 1 flips bit order");
  Serial.println("status\t\tNA\t\tShow status register values");
  Serial.println("read\t\tAdd,Len\t\tRead Flash Memory\n\t\t\t\tAdd : Start Address\n\t\t\t\tLen : Read length");
  Serial.println("set\t\tAdd,Val\t\tSet Write Buffer\n\t\t\t\tAdd : Address (0-255)\n\t\t\t\tVal : Byte Value");
  Serial.println("clear\t\tNA\t\tClear Write Buffer (not needed after write)");
  Serial.println("write\t\tAdd,Len\t\tExecute Page Program\n\t\t\t\tAdd : Start Address\n\t\t\t\tLen : Length (<=256)");
  Serial.println("erase\t\tAdd\t\tSector Erase\n\t\t\t\tAdd : Address");

  delay(1000);
  
}

void cmd_getid()
{
  arg_add = atoi(cmd_args);
  Serial.print("Reading Flash ID Values (Add:");
  Serial.print(String(arg_add, DEC));
  Serial.print(")...");
  
  flash.ReadDeviceIdentifierValues(arg_add);
  Serial.println("DONE.");

  if(arg_add == 0)
  {
    Serial.print("Man ID : ");
    Serial.println(String(FlashOutput[0], HEX));
    Serial.print("Dev ID : ");
    Serial.println(String(FlashOutput[1], HEX));
  }
  else
  {
    Serial.print("Dev ID : ");
    Serial.println(String(FlashOutput[0], HEX));
    Serial.print("Man ID : ");
    Serial.println(String(FlashOutput[1], HEX));
  }

  
}

void cmd_read()
{
  tok = strtok(cmd_args, ",");
  arg_add = atoi(tok);

  tok = strtok(NULL, ",");
  arg_len = atoi(tok);

  if(arg_len > 256)
  {
    Serial.println("Error - Read length should be below size of output buffer (256)");
    return;
  }

  Serial.print("Reading ");
  Serial.print(String(arg_len, DEC));
  Serial.print(" bytes of memory starting at address ");
  Serial.println(String(arg_add, HEX));

  flash.Read(arg_add, arg_len);

  Serial.println("(Address)\t=\tData");

  for(int i=0; i < arg_len; i++)
  {
    Serial.print("(");
    Serial.print(String(arg_add+i, HEX));
    Serial.print(")\t=\t");
    Serial.println(String(FlashOutput[i], HEX));
  }

  Serial.println("Read Command Finished.");
}



void cmd_status()
{
  Serial.print("Reading status register...");
  flash.ReadStatusRegister();
  Serial.println("Done.\nFlag Name\t\tFlag Status");

  char status = flash.ReadStatusFlag(STATUS_WRITE_IN_PROGRESS);
  Serial.print("WRITE IN PROGRESS\t\t");
  Serial.println(String(status, DEC));

  status = flash.ReadStatusFlag(STATUS_WRITE_ENABLE_LATCH);
  Serial.print("WRITE ENABLE LATCH\t\t");
  Serial.println(String(status, DEC));

  status = flash.ReadStatusFlag(STATUS_BLOCK_PROTECT_0);
  Serial.print("BLOCK PROTECT 0\t\t");
  Serial.println(String(status, DEC));

  status = flash.ReadStatusFlag(STATUS_BLOCK_PROTECT_1);
  Serial.print("BLOCK PROTECT 1\t\t");
  Serial.println(String(status, DEC));

  status = flash.ReadStatusFlag(STATUS_BLOCK_PROTECT_2);
  Serial.print("BLOCK PROTECT 2\t\t");
  Serial.println(String(status, DEC));

  status = flash.ReadStatusFlag(STATUS_BLOCK_PROTECT_3);
  Serial.print("BLOCK PROTECT 3\t\t");
  Serial.println(String(status, DEC));

  status = flash.ReadStatusFlag(STATUS_QUAD_ENABLE);
  Serial.print("QUAD ENABLE\t\t");
  Serial.println(String(status, DEC));

  status = flash.ReadStatusFlag(STATUS_REG_WRITE_DISABLE);
  Serial.print("REG WRITE DISABLE\t\t");
  Serial.println(String(status, DEC));

}

void cmd_set()
{
  tok = strtok(cmd_args, ",");
  arg_add = atoi(tok);

  tok = strtok(NULL, ",");
  arg_val = atoi(tok);

  Serial.print("Write buffer address ");
  Serial.print(String(arg_add, HEX));
  Serial.print(" set to ");
  Serial.print(String(arg_val, HEX));
  Serial.print(" (was ");
  Serial.print(String(WriteBuf[arg_add], HEX));
  Serial.println(")");

  WriteBuf[arg_add] = arg_val;

  
}

void cmd_clear()
{
  flash.ClearBuffer(WriteBuf, 256);
  Serial.println("Cleared write buffer, ready for data.");
}

void cmd_write()
{
  tok = strtok(cmd_args, ",");
  arg_add = atoi(tok);

  tok = strtok(NULL, ",");
  arg_len = atoi(tok);

  Serial.print("Programming ");
  Serial.print(String(arg_len, DEC));
  Serial.print(" bytes from address ");
  Serial.print(String(arg_add, HEX));
  Serial.print("...");

  flash.FillWriteBuffer(WriteBuf, 0, arg_len, 256);
  flash.PageProgram(arg_add, arg_len);

  Serial.println("Done.");

  //Clear buffer for next write
  flash.ClearBuffer(WriteBuf, 256);
}

void cmd_sectorerase()
{
  tok = strtok(cmd_args, ",");
  arg_add = atoi(tok);

  Serial.print("Erasing sector containing address ");
  Serial.print(String(arg_add, HEX));
  Serial.print("...");

  flash.SectorErase(arg_add);

  Serial.println("Done.");
}

void interpretCMD()
{
  //Read serial console
  cmdString = Serial.readString();

  //Return if timed out or empty
  if(cmdString.length() <= 0)
    return;

  //Print entered command to console
  Serial.print(cmdString);

  //Reset buffers
  memset(cmd_full, 0, 1024);
  memset(cmd_main, 0, 24);
  memset(cmd_args, 0, 1000);

  //Split into main cmd and args
  cmdString.toCharArray(cmd_full, cmdString.length()+1);
  tok = strtok(cmd_full, " ");
  memcpy(cmd_main, tok, strlen(tok));
  
  tok = strtok(NULL, " ");
  memcpy(cmd_args, tok, strlen(tok));

  //Interpret command
  tempString = String(cmd_main);
  
  if(tempString.indexOf("help") >= 0)
  {
    cmd_help();
    return;
  }

  if(tempString.indexOf("getid") >= 0)
  {
    cmd_getid();
    return;
  }

  if(tempString.indexOf("read") >= 0)
  {
    cmd_read();
    return;
  }

  if(tempString.indexOf("status") >= 0)
  {
    cmd_status();
    return;
  }

  if(tempString.indexOf("write") >= 0)
  {
    cmd_write();
    return;
  }

  if(tempString.indexOf("set") >= 0)
  {
    cmd_set();
    return;
  }

  if(tempString.indexOf("erase") >= 0)
  {
    cmd_sectorerase();
    return;
  }

  if(tempString.indexOf("clear") >= 0)
  {
    cmd_clear();
    return;
  }
  
}

void loop() {
  
   interpretCMD();
}