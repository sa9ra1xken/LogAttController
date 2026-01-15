#include "Screen.h"
#include <Adafruit_SSD1306.h>

Screen::Screen(DisplayDriverIF& d)
  : _disp(d) {
  }

void Screen::init(){
  Adafruit_GFX& g = _disp.gfx();
  g.fillScreen(SSD1306_BLACK);
  g.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  g.setRotation(2);
  initialized = true;
}

void Screen::writeHeader(String str) {
  if (!initialized) init();
  Adafruit_GFX& g = _disp.gfx();
  g.fillRect(0, 0, 128, 20, SSD1306_BLACK);
  g.setCursor(0, 0);
  g.setTextSize(2);
  g.print(str);
  _disp.display();
}

void Screen::writeBody(String str) {
  if (!initialized) init();
  Adafruit_GFX& g = _disp.gfx();
  g.fillRect(0, 20, 128, 48-20, SSD1306_BLACK);
  g.setCursor(0, 20);
  g.setTextSize(3);
  g.print(str);
  _disp.display();
}

void Screen::writeFooter(String str) {
  if (!initialized) init();
  Adafruit_GFX& g = _disp.gfx();
  g.fillRect(0, 48, 128, 64-48, SSD1306_BLACK);
  g.setCursor(0, 48);
  g.setTextSize(2);
  g.print(str);
  _disp.display();
}

void Screen::clear() {
  if (!initialized) init();
  Adafruit_GFX& g = _disp.gfx();
  g.fillScreen(SSD1306_BLACK);  
  _disp.display();
}
