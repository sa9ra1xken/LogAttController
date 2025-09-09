#include <Arduino.h>
#include <SPI.h>
#include <Vrekrer_scpi_parser.h>

SPISettings mySPISettings = SPISettings(8000000, LSBFIRST, SPI_MODE0);

SCPI_Parser myParser;

byte reverseByte(byte b) {
  byte reversedB = 0;
  for (int i = 0; i < 8; i++) {
    if (bitRead(b, i)) { // Check if the i-th bit is set
      bitWrite(reversedB, 7 - i, 1); // Set the corresponding bit in the reversed byte
    }
  }
  return reversedB;
}

byte swapBits(byte b, int pos1, int pos2){
  byte mask1 = 0x01 << pos1; 
  byte mask2 = 0x01 << pos2;
  bool source1 = b & mask1;   
  bool source2 = b & mask2;
  byte dest1 = source2 ? mask1 : 0x00;
  byte dest2 = source1 ? mask2 : 0x00;
  byte unchanged = b & ~ ( mask1 | mask2 ) ;
  return unchanged | dest1 | dest2; 
}

void SendAttenuaion(int left, int right){

  byte data_right = swapBits(right, 5, 7); 
  byte data_left = swapBits(reverseByte(left), 0, 2);

  SPI.beginTransaction(mySPISettings);
  digitalWrite(SS, LOW);
  SPI.transfer(~data_right);
  SPI.transfer(~data_left);
    
  digitalWrite(SS, HIGH);
  SPI.endTransaction();

}

void SetAttImmidiate(SCPI_C commands, SCPI_P parameters, Stream& interface){
  int left;
  int right;
  if (parameters.Size() <= 0) return; 
  left = constrain(String(parameters[0]).toInt(), 0, 255);
  right = left;
  if (parameters.Size() > 1) {
    right = constrain(String(parameters[1]).toInt(), 0, 255);
  }
  SendAttenuaion(left, right);
}

void setup(){
  myParser.SetCommandTreeBase(F("ATT"));
  myParser.RegisterCommand(F(":IMMidiate"), &SetAttImmidiate);
  Serial.begin( 9600 );
  SPI.begin();
}

void loop() {
  myParser.ProcessInput(Serial, "\n");
}
