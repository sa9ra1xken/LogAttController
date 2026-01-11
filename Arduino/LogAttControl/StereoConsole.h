#ifndef STEREO_CONTROL
#define STEREO_CONTROL

#include "BaseConsole.h"

class StereoControl : BaseConsole 
{
  public:
    enum Mode {
      VOLUME,
      BALANCE
    };

    Mode mode;
    void knob_change(int delta);
    void button_pressed();
    void button_released(int duration_ms);
    
  private:
    int asd;
};

#endif