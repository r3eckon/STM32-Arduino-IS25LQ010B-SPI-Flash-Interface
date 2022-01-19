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
  Serial.println("read\t\tAdd,Len\t\tRead Flash Memory\n\t\t\t\tAdd : Start Address\n\t\t\t\tLen : Read length");

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
    Serial.println("Error - Read length should be below size of output buffer");
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
  
}

void loop() {
  
   interpretCMD();
}