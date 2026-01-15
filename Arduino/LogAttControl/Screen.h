#ifndef SCREEN_H
#define SCREEN_H

#include "DisplayDriverIF.h"

class Screen {
public:
  Screen(DisplayDriverIF& d);
  void init();
  void writeHeader(String str);
  void writeBody(String str);
  void writeFooter(String str);
  void clear();
private:
  DisplayDriverIF& _disp;
  bool initialized = false;
};

#endif
