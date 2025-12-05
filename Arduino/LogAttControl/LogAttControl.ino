//#define SSD1306
#define SH1106
const int SCREEN_WIDTH = 128;     //ディスプレイのサイズ指定
const int SCREEN_HEIGHT = 64;     //ディスプレイのサイズ指定
const int SCREEN_ADDRESS = 0x3C;  //I2Cのアドレス指定

#ifdef SSD1306
#include <Adafruit_SSD1306.h>  //AdarfuitのSSD1306用ライブラリー
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
#else
#ifdef SH1106
#include <Adafruit_SH110X.h>  //AdarfuitのAH110X用ライブラリー
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
#define SSD1306_BLACK SH110X_BLACK
#define SSD1306_WHITE SH110X_WHITE 
#else
#error "No oled model defined. Please define SSD1306 or SH1106." 
#endif
#endif

//#include <Wire.h>
#include <Arduino.h>
#include <RotaryEncoder.h>
#include <SPI.h>
#include <Vrekrer_scpi_parser.h>
#include "CBTimer.h"
#include <EEPROM.h>

#define IR_38K_PIN 0
#define IR_455K_PIN 1
#define RE_IN1 2
#define RE_IN2 3
#define BUTTON_IN 7

#define DECODE_NEC
//#define DECODE_BEO
//#define USE_THRESHOLD_DECODER 
// Mode 1: Mode with gaps between frames
// Do NOT define ENABLE_BEO_WITHOUT_FRAME_GAP and set RECORD_GAP_MICROS to at least 16000 to accept the unusually long 3. start space
// Can only receive single messages. Back to back repeats will result in overflow
// Mode 2: Break at start mode
// Define ENABLE_BEO_WITHOUT_FRAME_GAP and set RECORD_GAP_MICROS to less than 15000
#include <IRremote.hpp>

//#include "IrBeo4.h"

RotaryEncoder* encoder = nullptr;
volatile bool LAST_BUTTON_STATUS;

SPISettings mySPISettings = SPISettings(8000000, LSBFIRST, SPI_MODE0);

SCPI_Parser myParser;

static CBTimer timer;

enum Function {
  VOLUME,
  BALANCE,
};

enum Granularity {
  COARSE,
  FINE
};  

volatile Function function;
volatile Granularity granularity; 
volatile int volume;
volatile int balance;

void OnPinChanged() {
  encoder->tick();
}

void SendAttenuaion(int left, int right) {

  Serial.println(String(left) + "/" + String(right));

  byte data_right = swapBits(-right, 5, 7);
  byte data_left = swapBits(reverseByte(-left), 0, 2);

  SPI.beginTransaction(mySPISettings);
  digitalWrite(SS, LOW);
  SPI.transfer(~data_right);
  SPI.transfer(~data_left);
  digitalWrite(SS, HIGH);
  SPI.endTransaction();
}

const int address_of_volume = 0;
const int address_of_balance = address_of_volume + sizeof(volume);

void VolumeChanged() {
  if (volume > 0) {
    volume = 0;
  }
  if (volume < -255) {
    volume = -255;
  }
  encoder->setPosition(volume);

  EEPROM.put(address_of_volume, volume);

  Show();

  int left = volume;
  int right = volume;

  if (balance < 0) right += balance;
  if (balance > 0) left -= balance;

  if (left < -255) left = -255;
  if (right < -255) right = -255;

  SendAttenuaion(left, right);
}

void BalanceChanged() {
  if (balance > 50) {
    balance = 50;
  }
  if (balance < -50) {
    balance = -50;
  }
  encoder->setPosition(balance);

  EEPROM.put(address_of_balance, balance);   

  Show();

  int left = volume;
  int right = volume;

  if (balance < 0) right += balance;
  if (balance > 0) left -= balance;

  if (left < -255) left = -255;
  if (right < -255) right = -255;

  SendAttenuaion(left, right);
}

void SetAttImmidiate(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  int left;
  int right;
  if (parameters.Size() <= 0) return;
  left = -constrain(String(parameters[0]).toInt(), 0, 255);
  right = left;
  if (parameters.Size() > 1) {
    right = -constrain(String(parameters[1]).toInt(), 0, 255);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print(String(left));
  display.display();

  SendAttenuaion(left, right);
}

volatile bool ButtonPressed;
volatile bool ButtonReleased;

void OnTimerExpired() {
  bool BUTTON_STATUS = digitalRead(BUTTON_IN);
  if (BUTTON_STATUS != LAST_BUTTON_STATUS) {
    if (BUTTON_STATUS == LOW) ButtonPressed = true;
    else ButtonReleased = true;
  }
  LAST_BUTTON_STATUS = BUTTON_STATUS;
}

void WriteHeader(String str) {
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print(str);
}

void WriteBody(String str) {
  display.setCursor(0, 20);
  display.setTextSize(3);
  display.print(str);
}

void WriteFooter(String str) {
  display.setCursor(0, 48);
  display.setTextSize(2);
  display.print(str);
}

void InitOled() {
  #ifdef SSD1306
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  #endif
  #ifdef SH1106
  display.begin(SCREEN_ADDRESS);
  #endif

  display.setRotation(2);
  display.clearDisplay();
  display.display();

}

void Show() {

  Serial.write("show\n");

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  switch (function) {
    case VOLUME:
      WriteHeader(F("Volume"));
      WriteBody(String(volume * 0.2, 1) + "dB");
      break;
    case BALANCE:
      if (balance < 0) {
        WriteHeader(F("Right is"));
        WriteBody(String(abs(balance) * 0.2, 1) + "dB");
        WriteFooter(F("lower"));
      } else if (balance > 0) {
        WriteHeader(F("Left is"));
        WriteBody(String(balance * 0.2, 1) + "dB");
        WriteFooter(F("lower"));
      } else {
        WriteHeader(F("Balance"));
        WriteBody(String(0.0, 1) + "dB");
      }
      break;
  }
  display.display();
}

void setup() {
  EEPROM.get(address_of_volume, volume);
  EEPROM.get(address_of_balance, balance);
  Serial.begin(9600);
  InitOled();
  pinMode(RE_IN1, INPUT_PULLUP);
  pinMode(RE_IN2, INPUT_PULLUP);
  encoder = new RotaryEncoder(RE_IN1, RE_IN2, RotaryEncoder::LatchMode::FOUR3);

  pinMode(BUTTON_IN, INPUT_PULLUP);
  LAST_BUTTON_STATUS = digitalRead(BUTTON_IN);

  function = VOLUME;
  encoder->setPosition(volume);
  Show();

  attachInterrupt(digitalPinToInterrupt(RE_IN1), OnPinChanged, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RE_IN2), OnPinChanged, CHANGE);

  myParser.SetCommandTreeBase(F("ATT"));
  myParser.RegisterCommand(F(":IMMidiate"), &SetAttImmidiate);

  pinMode(SS, OUTPUT);
  SPI.begin();

  ButtonPressed = false;
  ButtonReleased = false;

  timer.begin(20/* msec*/, OnTimerExpired);
  IrReceiver.begin(IR_38K_PIN, DISABLE_LED_FEEDBACK);
  VolumeChanged();
  Show();
}

void ToggleModes() {
  switch (function) {
    case VOLUME:
      function = BALANCE;
      encoder->setPosition(balance);
      break;
    case BALANCE:
      function = VOLUME;
      encoder->setPosition(volume);
      break;
  }
  Show();
}

void OnButtonPressed() {
  ButtonPressed = false;
}

void OnButtonReleased() {
  ButtonReleased = false;
  /*switch (myMode) {
    case VOLUME:
      myMode = BALANCE;
      encoder->setPosition(balance);
      break;
    case BALANCE:
      myMode = VOLUME;
      encoder->setPosition(volume);
      break;
  }*/
  ToggleModes();
  Show();
}

//httpsja.aliexpress.comitem1005008599583899.htmlspm=a2g0o.order_list.order_list_main.12.4acf585ayYGnBV&gatewayAdapt=glo2jpn

void HandleIR() {
  if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
    Serial.println(F("Received noise or an unknown (or not yet enabled) protocol"));
    IrReceiver.printIRResultRawFormatted(&Serial, true);
    IrReceiver.resume();
  } else {
    IrReceiver.resume();
    IrReceiver.printIRResultShort(&Serial);
    //IrReceiver.printIRSendUsage(&Serial);
  }
  Serial.println();

  if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
    switch (IrReceiver.decodedIRData.command) {
      case 0x15:
        Serial.println(F("Repeated command 0x15."));
        volume--;
        VolumeChanged();
        break;
      case 0x09:
        Serial.println(F("Repeated command 0x09."));
        volume++;
        VolumeChanged();
        break;
    }

  } else {
    switch (IrReceiver.decodedIRData.command) {
      case 0x15:
        Serial.println(F("Received command 0x15."));
        volume--;
        VolumeChanged();
        break;
      case 0x09:
        Serial.println(F("Received command 0x09."));
        volume++;
        VolumeChanged();
        break;
      case 0x46:
        Serial.println(F("Received command 0x46."));
        ToggleModes();
        break;
    }
  }
}


void loop() {

  if (Serial.available() > 0) myParser.ProcessInput(Serial, "\n");
  if (ButtonPressed) OnButtonPressed();
  if (ButtonReleased) OnButtonReleased();

  int newPos = encoder->getPosition();
  switch (function) {
    case VOLUME:
      if (newPos != volume) {
        volume = newPos;
        VolumeChanged();
      }
      break;
    case BALANCE:
      if (newPos != balance) {
        balance = newPos;
        BalanceChanged();
      }
      break;
  }

  if (IrReceiver.decode()) HandleIR();
}