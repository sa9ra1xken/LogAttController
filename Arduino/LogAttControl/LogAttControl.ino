//#define SSD1306
#define SH1106

#define IR_38K_PIN 0
#define IR_455K_PIN 1
#define RE_IN1 2
#define RE_IN2 3
#define BUTTON_IN 7
#define TIMER_INTERVAL_mS 20
#define INTERVAL2SAVE_STATE_mS 1000
#define DECIBEL_PER_STEP 0.2
#define COARSE_STEP 5

#include <Arduino.h>
#include <RotaryEncoder.h>
#include <SPI.h>
#include <Wire.h>
#include <Vrekrer_scpi_parser.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_SH110X.h>
#include "CBTimer.h"
#include "Screen.h" 
#include "StereoController.h"
#include "ButtonListner.h"

#ifdef SSD1306
  #include "SSD1306Driver.h"
  Adafruit_SSD1306 oled(128, 64, &Wire);
  SSD1306Driver oled_driver(oled);
#else
  #ifdef SH1106
    #include "SH1106GDriver.h"
    Adafruit_SH1106G oled(128, 64, &Wire);
    SH1106GDriver oled_driver(oled);
  #else
    #error "No oled model defined. Please define SSD1306 or SH1106." 
  #endif
#endif

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

Screen screen(oled_driver);
RotaryEncoder* encoder = nullptr;
SCPI_Parser myParser;
static CBTimer timer;
StereoController controller(screen, DECIBEL_PER_STEP, COARSE_STEP);
Button button(BUTTON_IN);

void OnPinChanged() {
  encoder->tick();
}

/*
int TestChannelCount(){
  SPI.beginTransaction(mySPISettings);
  uint8_t rb = SPI.transfer(0xff);
  SPI.endTransaction();
  Serial.write("readback:{0}\n,rb");
  return 0;
}
*/

/*
void SetAttImmidiate(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  int left;
  int right;
  if (parameters.Size() <= 0) return;
  left = -constrain(String(parameters[0]).toInt(), 0, 255);
  right = left;
  if (parameters.Size() > 1) {
    right = -constrain(String(parameters[1]).toInt(), 0, 255);
  }

  SendAttenuaion(left, right);
}
*/

//////////////////////////////////////
// Timer interrupt
////////////////////////////////////// 

int tick_counter = 0;
void OnTimerExpired() {
  button.tick();
  tick_counter++;
  if (tick_counter >= INTERVAL2SAVE_STATE_mS / TIMER_INTERVAL_mS ){
    tick_counter = 0;
    controller.preserveState();
  }
}

//////////////////////////////////////////////////
// IR reception
// httpsja.aliexpress.comitem1005008599583899.htmlspm=a2g0o.order_list.order_list_main.12.4acf585ayYGnBV&gatewayAdapt=glo2jpn
//////////////////////////////////////////////////

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
        controller.setFine();
        controller.onKnobTurned(-1);
        break;
      case 0x40:
        controller.setCoarse();
        controller.onKnobTurned(-1);
        break;
      case 0x09:
        controller.setFine();
        controller.onKnobTurned(1);
        break;
      case 0x43:
        controller.setCoarse();
        controller.onKnobTurned(+1);
        break;
    }

  } else {
    switch (IrReceiver.decodedIRData.command) {
      case 0x15:
        controller.setFine();
        controller.onKnobTurned(-1);
        break;
      case 0x40:
        controller.setCoarse();
        controller.onKnobTurned(-1);
        break;
      case 0x09:
        controller.setFine();
        controller.onKnobTurned(1);
        break;
      case 0x43:
        controller.setCoarse();
        controller.onKnobTurned(+1);
        break;
      case 0x46:
        controller.onButtonReleased(20);
        break;
    }
  }
}

////////////////////////////////
// Setup
////////////////////////////////

void setup() {

  Serial.begin(9600);
  
  pinMode(RE_IN1, INPUT_PULLUP);
  pinMode(RE_IN2, INPUT_PULLUP);
  encoder = new RotaryEncoder(RE_IN1, RE_IN2, RotaryEncoder::LatchMode::FOUR3);
  encoder->setPosition(0);
  attachInterrupt(digitalPinToInterrupt(RE_IN1), OnPinChanged, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RE_IN2), OnPinChanged, CHANGE);

  pinMode(BUTTON_IN, INPUT_PULLUP);
  timer.begin(TIMER_INTERVAL_mS, OnTimerExpired);
  
  //myParser.SetCommandTreeBase(F("ATT"));
  //myParser.RegisterCommand(F(":IMMidiate"), &SetAttImmidiate);

  pinMode(SS, OUTPUT);
  SPI.begin();
  IrReceiver.begin(IR_38K_PIN, DISABLE_LED_FEEDBACK);
  Wire.begin();
#ifdef SSD1306
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
#endif
#ifdef SH1106
  oled.begin(0x3C);
#endif
  controller.begin();
  button.begin();
}

////////////////////////////////
// Loop
////////////////////////////////

void loop() {
  
  if (Serial.available() > 0) myParser.ProcessInput(Serial, "\n");
  
  if (button.wasReleased){
    button.wasReleased = false;
    controller.onButtonReleased(button.duration);
  }
  
  int pos = encoder->getPosition();
  if (pos != 0) {
    encoder->setPosition(0);
    controller.onKnobTurned(pos);
  }

  if (IrReceiver.decode()) HandleIR();
}
