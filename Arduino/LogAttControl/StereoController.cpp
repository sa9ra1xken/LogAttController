#include <EEPROM.h>
#include "StereoController.h"
#include "Screen.h"
#include "utilities.h"

#define EEPROM_VOLUME 0
#define EEPROM_BALANCE EEPROM_VOLUME + sizeof(_volume)

void StereoController::begin(){

  EEPROM.get(EEPROM_VOLUME, _volume);
  EEPROM.get(EEPROM_BALANCE, _balance);
 
  _mode = VOLUME;
  showVolumeHeader();
  showDeciBel(_volume * _db_per_step);
  sendAtt();
}

void StereoController::showVolumeHeader(){
  if (_fine)  _screen.writeHeader("Vol:fine  ");
  else        _screen.writeHeader("Vol:coarse"); 
}

void StereoController::showBalanceHeader(){
  if (_fine)  _screen.writeHeader("Bal:fine  ");
  else        _screen.writeHeader("Bal:coarse"); 
}

void StereoController::showDeciBel(float db){
  String str = String(db, 1);
  str.concat("dB");
  _screen.writeBody(str);
}

void StereoController::onButtonReleased(int duration){
  switch (_mode){
    case VOLUME:
      if (duration<15){
        _fine = !_fine;
        showVolumeHeader();
      }
      else{
        _mode = BALANCE;
        showBalanceHeader();
        showDeciBel(abs(_balance) * _db_per_step);
        showBalanceFooter();
      }
      break;
    case BALANCE:
      if (duration<15){
        _fine = !_fine;
        showBalanceHeader();
      }
      else{
        _mode = VOLUME;
        showVolumeHeader();
        showDeciBel(_volume * _db_per_step);
        _screen.writeFooter("");
      }
      break;
  }
}

void StereoController::showBalanceFooter(){
  switch(sign(_balance)){
    case NEG:
      _screen.writeFooter("L>R");
      break;
    case ZERO:
      _screen.writeFooter("L=R");
      break;
    case POS:
      _screen.writeFooter("L<R");
      break;
  }
}

void StereoController::onKnobTurned(int notch){
  switch (_mode){
    case VOLUME:
      _volume += _fine ? notch : notch * _coarse_step;
      _volume = constrain(_volume, -255, 0);
      showDeciBel(_volume * _db_per_step); 
      sendAtt();
      break;
    case BALANCE:
      int old_balance = _balance;
      _balance += _fine ? notch : notch * _coarse_step;
      _balance = constrain(_balance, -50, 50);
      showDeciBel(abs(_balance) * _db_per_step); 
      if (sign(old_balance)!=sign(_balance)) showBalanceFooter();
      sendAtt();
      break;
  }
}

void StereoController::preserveState(){
  int temp;
  EEPROM.get(EEPROM_VOLUME, temp);
  if (_volume != temp) EEPROM.put(EEPROM_VOLUME, _volume);
  EEPROM.get(EEPROM_BALANCE, temp);
  if (_balance != temp) EEPROM.put(EEPROM_BALANCE, _balance);
}

void StereoController::sendAtt(){

  int left = _volume;
  int right = _volume;

  if (_balance < 0) right += _balance;
  if (_balance > 0) left -= _balance;

  byte data_right = swapBits(-right, 5, 7);
  byte data_left = swapBits(reverseByte(-left), 0, 2);

  SPI.beginTransaction(mySPISettings);
  digitalWrite(SS, LOW);
  SPI.transfer(~data_right);
  SPI.transfer(~data_left);
  digitalWrite(SS, HIGH);
  SPI.endTransaction();

}

void StereoController::showHeader(){
  switch (_mode){
    case VOLUME:
      showVolumeHeader();
      break;
    case BALANCE:
      showBalanceHeader();
      break;
  }
}

void StereoController::setFine(){
  _fine = true;
  showHeader();
}  

void StereoController::setCoarse(){
  _fine = false;
  showHeader();
}  
