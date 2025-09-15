#include <Arduino.h>
#include <SPI.h>
#include <Vrekrer_scpi_parser.h>
#include <Wire.h>
#include <Adafruit_GFX.h>      //Adarfuitの画像描写ライブラリー
#include <Adafruit_SSD1306.h>  //AdarfuitのSSD1306用ライブラリー

const int SCREEN_WIDTH = 128;     //ディスプレイのサイズ指定
const int SCREEN_HEIGHT = 64;     //ディスプレイのサイズ指定
const int SCREEN_ADDRESS = 0x3C;  //I2Cのアドレス指定
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

SPISettings mySPISettings = SPISettings(8000000, LSBFIRST, SPI_MODE0);

SCPI_Parser myParser;


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





byte reverseByte(byte b) {
  byte reversedB = 0;
  for (int i = 0; i < 8; i++) {
    if (bitRead(b, i)) {              // Check if the i-th bit is set
      bitWrite(reversedB, 7 - i, 1);  // Set the corresponding bit in the reversed byte
    }
  }
  return reversedB;
}

byte swapBits(byte b, int pos1, int pos2) {
  byte mask1 = 0x01 << pos1;
  byte mask2 = 0x01 << pos2;
  bool source1 = b & mask1;
  bool source2 = b & mask2;
  byte dest1 = source2 ? mask1 : 0x00;
  byte dest2 = source1 ? mask2 : 0x00;
  byte unchanged = b & ~(mask1 | mask2);
  return unchanged | dest1 | dest2;
}

void SendAttenuaion(int left, int right) {

  byte data_right = swapBits(right, 5, 7);
  byte data_left = swapBits(reverseByte(left), 0, 2);

  SPI.beginTransaction(mySPISettings);
  digitalWrite(SS, LOW);
  SPI.transfer(~data_right);
  SPI.transfer(~data_left);

  digitalWrite(SS, HIGH);
  SPI.endTransaction();
}

void SetAttImmidiate(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  int left;
  int right;
  if (parameters.Size() <= 0) return;
  left = constrain(String(parameters[0]).toInt(), 0, 255);
  right = left;
  if (parameters.Size() > 1) {
    right = constrain(String(parameters[1]).toInt(), 0, 255);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print(String(left));
  display.display();

  SendAttenuaion(left, right);
}

void write(String str) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print(String(str));
  display.display();
}

void setup() {

  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.display();

  myParser.SetCommandTreeBase(F("ATT"));
  myParser.RegisterCommand(F(":IMMidiate"), &SetAttImmidiate);
  Serial.begin(9600);
  while (!Serial)
    ;  // Uno R4では必要
  pinMode(SS, OUTPUT);
  SPI.begin();
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  if (Serial.available() > 0) {
    digitalWrite(LED_BUILTIN, HIGH);  // 通信を受信したらLED ON
    delay(100);                       // 100ms点灯
    digitalWrite(LED_BUILTIN, LOW);   // 消灯
    delay(100);
    String str = Serial.readString();
    //write(String(str.length()));
    write(str);

    //myParser.ProcessInput(Serial, "\n");
  }
}
