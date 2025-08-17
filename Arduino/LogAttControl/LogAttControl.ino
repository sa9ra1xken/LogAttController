#include <Wire.h>
#include <Adafruit_GFX.h>         //Adarfuitの画像描写ライブラリーを読み込む
#include <Adafruit_SSD1306.h>     //AdarfuitのSSD1306用ライブラリーを読み込む

#include <Arduino.h>
#include <RotaryEncoder.h>

#define RE_IN2 A2
#define RE_IN1 A3

// A pointer to the dynamic created rotary encoder instance.
// This will be done in setup()
RotaryEncoder *encoder = nullptr;

void checkPosition()
{
  encoder->tick(); // just call tick() to check the state.
}

const int SCREEN_WIDTH = 128;     //ディスプレイのサイズ指定
const int SCREEN_HEIGHT = 64;     //ディスプレイのサイズ指定
const int SCREEN_ADDRESS = 0x3C;  //I2Cのアドレス指定

Adafruit_SSD1306 display( SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);  //ディスプレイ制御用のインスタンスを作成。この時にデイスプレのサイズを渡す。

int count = 1;                    //countを整数型の変数として定義

void setup() {
  display.begin( SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS );
  display.clearDisplay();
  display.display();

  pinMode(RE_IN1, INPUT_PULLUP);
  pinMode(RE_IN2, INPUT_PULLUP);

  // setup the rotary encoder functionality

  // use FOUR3 mode when PIN_IN1, PIN_IN2 signals are always HIGH in latch position.
  // encoder = new RotaryEncoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::FOUR3);

  // use FOUR0 mode when PIN_IN1, PIN_IN2 signals are always LOW in latch position.
  // encoder = new RotaryEncoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::FOUR0);

  // use TWO03 mode when PIN_IN1, PIN_IN2 signals are both LOW or HIGH in latch position.
  encoder = new RotaryEncoder(RE_IN1, RE_IN2, RotaryEncoder::LatchMode::TWO03);

  // register interrupt routine
  attachInterrupt(digitalPinToInterrupt(RE_IN1), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RE_IN2), checkPosition, CHANGE);
} 

void loop() {
  static int pos = 0;
  encoder->tick(); // just call tick() to check the state.
  int newPos = encoder->getPosition();
  if (pos != newPos) {
    display.clearDisplay();
    display.setTextSize( 3 );
    display.setTextColor( SSD1306_WHITE, SSD1306_BLACK);
    display.setCursor(0, 0);
    display.print(String(newPos*0.2,1));
    display.print("dB");
    display.display();
    pos = newPos;
  }
}