#include<Arduino.h>

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