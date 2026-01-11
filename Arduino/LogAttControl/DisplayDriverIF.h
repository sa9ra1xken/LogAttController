#ifndef DISPLAY_DRIVER_IF_H
#define DISPLAY_DRIVER_IF_H

#include <Adafruit_GFX.h>

class DisplayDriverIF {
public:
  virtual Adafruit_GFX& gfx() = 0;
  virtual void display() = 0;
  virtual uint16_t width() = 0;
  virtual uint16_t height() = 0;
  virtual ~DisplayDriverIF() {}
};

#endif
