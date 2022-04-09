// ボタンを押すと、
// 1000ppm以下だったらゲートが一定時間開
// 1000ppm以上だったらゲートが開かない

#include <M5StickC.h>
#include "MHZ19_uart.h"
#include "esp32-hal-ledc.h"

// CO2
#define RX_PIN 36 // Rx pin which the MHZ19 Tx pin is attached to
#define TX_PIN 0 // Tx pin which the MHZ19 Rx pin is attached to
#define CO2_UPDATE_INTERVAL 5000 // msec
MHZ19_uart mhz19; // Constructor for library

int ppmValue = 0;

// Gate
#define TIMER_WIDTH 16
#define COUNT_LOW 1500
#define COUNT_HIGH 6500

void updatePpm()
{
  // 値の更新
  ppmValue = mhz19.getPPM();

  // 表示の更新
  int height = M5.Lcd.height();
  int width = M5.Lcd.width();

  M5.Lcd.fillRect(0, 0, width, height, BLACK);
  
  M5.Lcd.setCursor(20, 10);
  M5.Lcd.setTextSize(2);

  if(ppmValue < 1000){
    M5.Lcd.setTextColor(WHITE);
  }else{
    M5.Lcd.setTextColor(RED);
  }
  M5.Lcd.printf("%4d",ppmValue);

  M5.Lcd.setCursor(90, 55);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);
  M5.Lcd.printf("ppm");
}

void gateUp()
{
  for(int i = COUNT_LOW; i < COUNT_HIGH; i = i + 100){
    // Serial.println(i);
    ledcWrite(1, i);
    delay(30);
  }
}

void gateDown()
{
  for(int i = COUNT_HIGH; i >= COUNT_LOW; i = i - 100){
    // Serial.println(i);
    ledcWrite(1, i);
    delay(30);
  }
}

// the setup routine runs once when M5StickC starts up
void setup(){
  // Initialize the M5StickC object
  M5.begin();
  // 画面を横向きに
  M5.Lcd.setRotation(1);
  M5.Lcd.setTextFont(2);

  // MHZ19のSETUP
  mhz19.begin(RX_PIN, TX_PIN);
  mhz19.setAutoCalibration(false);
  delay(3000);

  // lecd
  ledcSetup(1, 50, TIMER_WIDTH);
  ledcAttachPin(32, 1);
  
  // LCD display
  M5.Lcd.print("Hello World");
}


enum GateState{
  Init,
  Open,
  Close
};
GateState gateOpenState = Init;
// the loop rsoutine runs over and over again forever
void loop() {
  // Buttonクラスを利用するときには必ずUpdateを呼んで状態を更新する
  M5.update();

  updatePpm();
  
  // ホームボタンを押したか？（1度だけ取得可能）
  if(ppmValue >= 1000 && gateOpenState != Open){
    Serial.printf("since co2 is upper 1000, so gate wont move\n",ppmValue);
    gateOpenState = Open;
    gateUp();
  }else if(ppmValue < 1000 && gateOpenState != Close){
    Serial.printf("gate move\n",ppmValue);
    gateOpenState = Close;
    gateDown();
  }
  
  // 1秒Wait
  delay(1000);
}
