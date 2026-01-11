#ifndef SH1106G_DRIVER_H
#define SH1106G_DRIVER_H

#include "DisplayDriverIF.h"
#include <Adafruit_SH110X.h>

class SH1106GDriver : public DisplayDriverIF {
public:
  SH1106GDriver(Adafruit_SH1106G& d) : disp(d) {}

  Adafruit_GFX& gfx() override { return disp; }
  void display() override { disp.display(); }
  uint16_t width() override { return disp.width(); }
  uint16_t height() override { return disp.height(); }

private:
  Adafruit_SH1106G& disp;
};

#endif
