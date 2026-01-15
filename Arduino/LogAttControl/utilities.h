#ifndef UTILITIES_H
#define UTILITIES_H

byte reverseByte(byte b);
byte swapBits(byte b, int pos1, int pos2);

  enum Sign{
    NEG,
    ZERO,
    POS
  }; 

  template <class T>
  Sign sign(T val){
    if (val<0) return NEG;
    if (val>0) return POS;
    return ZERO;
  }

#endif