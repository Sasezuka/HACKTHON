// --- Arduino2_I2C_Master_Test.ino ---

#include <Wire.h> // I2C通信ライブラリをインクルード

const int SLAVE_ADDRESS = 8; // スレーブのアドレス。0-127の間で自由に設定できます。

volatile uint16_t sendValue = 0; // 送信する値 (0からカウントアップ)

void setup() {
  Serial.begin(115200);
  Serial.println("Arduino 2 (Master) I2C Test Started!");

  Wire.begin(); // マスターとしてI2C通信を開始
  Serial.println("I2C Master Initialized.");
}

void loop() {
  delay(1000); // 1秒ごとにデータを送信

  Serial.print("Sending: ");
  Serial.println(sendValue);

  // I2C通信を開始し、スレーブアドレスを指定
  Wire.beginTransmission(SLAVE_ADDRESS); 
  
  // 12bitデータを2バイトに分けて送信
  // highByteはsendValueの8ビット分、lowByteはsendValueの残り4ビット
  // sendValueが12ビットなので、0-4095
  byte highByte = highByte = (sendValue >> 8) & 0xFF; // 上位8ビット
  byte lowByte = sendValue & 0xFF;                  // 下位8ビット（下位4ビットはスレーブ側でマスク）

  Wire.write(highByte); // 上位バイトを送信
  Wire.write(lowByte);  // 下位バイトを送信

  // 通信を終了
  Wire.endTransmission();

  // 送信する値をインクリメント (12bit値なので最大4095)
  sendValue++;
  if (sendValue > 4095) {
    sendValue = 0;
  }
}