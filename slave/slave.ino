#include <Wire.h>

// このArduinoのスレーブI2Cアドレス
const int MY_ADDRESS = 0x8;

// 受信した正弦波の値を格納する変数
volatile int receivedSineValue = 0;
volatile boolean newData = false; // 新しいデータが来たことを示すフラグ

void setup() {
  Wire.begin(MY_ADDRESS); // 指定したアドレスでスレーブとしてI2Cバスを開始
  Serial.begin(115200);
  Serial.println("Slave Arduino Ready (Receiving Sine Wave)! Address: 0x8");

  // マスターからデータを受信したときに呼ばれる関数を登録
  Wire.onReceive(receiveEvent);
}

void loop() {
  // 新しいデータが来たらシリアル表示
  if (newData) {
    Serial.print("Slave: Received sine value: ");
    Serial.println(receivedSineValue);
    newData = false; // フラグをリセット
  }
  delay(10); // 短いディレイでCPU負荷を軽減
}

// データを受信したときに実行される関数
void receiveEvent(int howMany) {
  // 受信するデータが1バイトであることを想定
  if (howMany >= 1) {
    receivedSineValue = Wire.read(); // 1バイト読み込み
    newData = true; // 新しいデータが来たことを示す
  }
  // その他のデータは読み捨て
  while (Wire.available()) {
    Wire.read();
  }
}