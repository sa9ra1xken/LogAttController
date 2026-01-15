#ifndef STEREO_CONTROL_H
#define STEREO_CONTROL_H

#include "Screen.h"

class StereoController {
public:
  StereoController(Screen& screen, double db_per_step, int coarse_step):
    _screen(screen), _db_per_step(db_per_step), _coarse_step(coarse_step) {}
  void begin();
  void onButtonReleased(int duration);
  void onKnobTurned(int notch);
  void preserveState();
  void SendAtt();
  void setCoarse();
  void setFine();

private:
  SPISettings mySPISettings = SPISettings(8000000, LSBFIRST, SPI_MODE0);
  Screen _screen;
  double _db_per_step;
  int _coarse_step;

  enum Mode{
    VOLUME,
    BALANCE
  };

  Mode _mode;
  bool _fine;

  int _volume;
  int _balance;

  void showVolumeHeader();
  void showDeciBel(float value);
  void showBalanceHeader();
  void showBalanceFooter();
  void sendAtt();
  void showHeader();

};

#endif
