// --- Arduino2_SPI_Master_Test.ino ---

#include <SPI.h>

const int SS_PIN = 10; // スレーブセレクトピン (Arduino 3のSSピンに接続)

volatile uint16_t sendValue = 0; // 送信する値 (0からカウントアップ)

void setup() {
  Serial.begin(115200);
  Serial.println("Arduino 2 (Master) SPI Test Started!");

  // SSピンを出力に設定し、初期状態はHIGH (スレーブ非選択)
  pinMode(SS_PIN, OUTPUT);
  digitalWrite(SS_PIN, HIGH);

  // SPIをマスターモードで初期化
  // クロックは 1MHz に設定
  SPI.begin();
  Serial.println("SPI Master Initialized.");
}

void loop() {
  // 1秒ごとにデータを送信
  delay(1000); 

  Serial.print("Sending: ");
  Serial.println(sendValue);

  // スレーブを選択 (SSピンをLOW)
  digitalWrite(SS_PIN, LOW); 

  // SPIトランザクション開始 (推奨される方法)
  // 1MHz, MSBFIRST, SPI_MODE0
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); // ★クロック速度を1MHzに戻す★

  // 16bitデータを2バイトに分割して送信
  byte highByte = (sendValue >> 4) & 0xFF; // 12bitデータの上位8bit
  byte lowByte = sendValue & 0x0F;         // 12bitデータの下位4bit (他のビットは0)

  SPI.transfer(highByte); // 上位バイトを送信
  SPI.transfer(lowByte);  // 下位バイトを送信

  // SPIトランザクション終了
  SPI.endTransaction();

  // スレーブ選択を解除 (SSピンをHIGH)
  digitalWrite(SS_PIN, HIGH);

  // 送信する値をインクリメント (12bit値なので最大4095)
  sendValue++;
  if (sendValue > 4095) {
    sendValue = 0;
  }
}