#ifndef SSD1306_DRIVER_H
#define SSD1306_DRIVER_H

#include "DisplayDriverIF.h"
#include <Adafruit_SSD1306.h>

class SSD1306Driver : public DisplayDriverIF {
public:
  SSD1306Driver(Adafruit_SSD1306& d) : disp(d) {}

  Adafruit_GFX& gfx() override { return disp; }
  void display() override { disp.display(); }
  uint16_t width() override { return disp.width(); }
  uint16_t height() override { return disp.height(); }

private:
  Adafruit_SSD1306& disp;
};

#endif
