#include <Wire.h>
#include <Adafruit_GFX.h>         //Adarfuitの画像描写ライブラリー
#include <Adafruit_SSD1306.h>     //AdarfuitのSSD1306用ライブラリー
#include <Arduino.h>
#include <RotaryEncoder.h>
#include <SPI.h>
#include "bit_manipulation.h"
#include <Vrekrer_scpi_parser.h>
//#include <MsTimer2.h>

const int SCREEN_WIDTH = 128;     //ディスプレイのサイズ指定
const int SCREEN_HEIGHT = 64;     //ディスプレイのサイズ指定
const int SCREEN_ADDRESS = 0x3C;  //I2Cのアドレス指定
Adafruit_SSD1306 display( SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

#define RE_IN1 2
#define RE_IN2 3
#define BUTTON_IN 7
RotaryEncoder *encoder = nullptr;
bool LAST_BUTTON_STATUS;

SPISettings mySPISettings = SPISettings(8000000, LSBFIRST, SPI_MODE0);

//SCPI_Parser myParser;

enum Mode{
  VOLUME,
  BALANCE,
  FF00,
};

Mode myMode; 
int volume;
int balance;
int ff00;

void checkPosition()
{
  encoder->tick();
}

void ButtonPressed(){
}

void ButtonReleased(){
  switch(myMode){
    case VOLUME:
      myMode = BALANCE;
      encoder->setPosition(balance);
    break;  
    case BALANCE:
      myMode = FF00;
      encoder->setPosition(ff00);
    break;  
    case FF00:
      myMode = VOLUME;
      encoder->setPosition(volume);
    break;  
  };
  Show();
}

void WriteHeader(String str){
      display.setCursor(0, 0);
      display.setTextSize( 2 );
      display.print(str);
}

void WriteBody(String str){
      display.setCursor(0, 20);
      display.setTextSize( 3 );
      display.print(str);
}

void WriteFooter(String str){
      display.setCursor(0, 48);
      display.setTextSize( 2 );
      display.print(str);
}

void SendAttenuaion(){

  int left = volume;
  int right = volume;

  if (balance < 0) right += balance; 
  if (balance > 0) left -= balance;

  if (left < -255) left = -255; 
  if (right < -255) right = -255;

  byte data_right = swapBits(-right, 5, 7); 
  byte data_left = swapBits(reverseByte(-left), 0, 2);

  Serial.print( data_left );
  Serial.print("/");
  Serial.println( data_right );

  SPI.beginTransaction(mySPISettings);
  digitalWrite(SS, LOW);
  SPI.transfer(~data_right);
  SPI.transfer(~data_left);
    
  digitalWrite(SS, HIGH);
  SPI.endTransaction();

}

void SendFF00(){

  int data = (ff00 == 0) ? 0 : 0xff;

  SPI.beginTransaction(mySPISettings);
  digitalWrite(SS, LOW);
  SPI.transfer(data);
  SPI.transfer(data);
  digitalWrite(SS, HIGH);
  SPI.endTransaction();

}

void Show(){
  display.clearDisplay();
  display.setTextColor( SSD1306_WHITE, SSD1306_BLACK);
  switch(myMode){

    case VOLUME:
      WriteHeader(F("Volume"));
      WriteBody(String(volume * 0.2, 1)+"dB");
    break;  

    case BALANCE:
      if (balance < 0){
        WriteHeader(F("Right is"));
        WriteBody(String(abs(balance) * 0.2, 1)+"dB");
        WriteFooter(F("lower"));
      }
      else if (balance > 0){
        WriteHeader(F("Left is"));
        WriteBody(String(balance * 0.2, 1)+"dB");
        WriteFooter(F("lower"));
      }
      else{
        WriteHeader(F("Balance"));
        WriteBody(String(0.0, 1)+"dB");
      }
    break;

    case FF00:
        WriteHeader(F("FF00"));
        WriteBody(String(ff00 & 0x1));
    break;

  };
  display.display();
}

void VolumeChanged(){
  if (volume > 0){volume = 0; encoder->setPosition(volume);}
  if (volume < -255){volume = -255; encoder->setPosition(volume);}
  Show();
  SendAttenuaion();
}

void BalanceChanged(){
  if (balance > 50){balance = 50; encoder->setPosition(balance);}
  if (balance < -50){balance = -50; encoder->setPosition(balance);}
  Show();
  SendAttenuaion();
}

void FF00Changed(){
  if (ff00 > 1){ff00 = 1; encoder->setPosition(ff00);}
  if (ff00 < 0){ff00 = 0; encoder->setPosition(ff00);}
  Show();
  SendFF00();
}

/*
void SetAttImmidiate(SCPI_C commands, SCPI_P parameters, Stream& interface) {

}
*/

void CheckButton(){
  bool BUTTON_STATUS = digitalRead(BUTTON_IN);  
  if (BUTTON_STATUS != LAST_BUTTON_STATUS){
    if (BUTTON_STATUS == LOW) ButtonPressed();
    else ButtonReleased();
  }
  LAST_BUTTON_STATUS = BUTTON_STATUS; 
}

void setup() {
  
  Serial.begin( 9600 );

  display.begin( SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS );
  display.clearDisplay();
  display.display();

  pinMode(RE_IN1, INPUT_PULLUP);
  pinMode(RE_IN2, INPUT_PULLUP);
  encoder = new RotaryEncoder(RE_IN1, RE_IN2, RotaryEncoder::LatchMode::TWO03);

  pinMode(BUTTON_IN, INPUT_PULLUP);
  LAST_BUTTON_STATUS = digitalRead(BUTTON_IN);

  myMode = VOLUME;
  encoder->setPosition(volume);
  Show();

  attachInterrupt(digitalPinToInterrupt(RE_IN1), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RE_IN2), checkPosition, CHANGE);

  //myParser.SetCommandTreeBase(F("ATT"));
  //myParser.RegisterCommand(F(":IMMidiate"), &SetAttImmidiate);

  SPI.begin();
 
  //MsTimer2::set(20, CheckButton);
  //MsTimer2::start();

 } 

void loop() {
  /*
  myParser.Execute(char *message, Stream &interface)
  */

  
  delay(10);
  bool BUTTON_STATUS = digitalRead(BUTTON_IN);  
  if (BUTTON_STATUS != LAST_BUTTON_STATUS){
    if (BUTTON_STATUS == LOW) ButtonPressed();
    else ButtonReleased();
  }
  LAST_BUTTON_STATUS = BUTTON_STATUS; 
  

  int newPos = encoder->getPosition();
  switch (myMode){
    case VOLUME:
      if (newPos != volume){
        volume = newPos;
        VolumeChanged();
      }
      break;
    case BALANCE:
      if (newPos != balance){
        balance = newPos;
        BalanceChanged();
      }
      break;
    case FF00:
      if (newPos != ff00){
        ff00 = newPos;
        FF00Changed();
      }
      break;
  } 
}