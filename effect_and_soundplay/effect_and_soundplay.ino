#include <Arduino.h>
#include <Wire.h>      // I2C通信ライブラリを追加
#include "temari.h"    // temariクラスのヘッダファイル
#include "constants.h" // SAMPLE_RATE, MAX_AMPLITUDE などの共通定数

// このArduinoのスレーブI2Cアドレス (Arduino 2のI2C_SLAVE_ADDRESSと一致させる)
const int MY_I2C_ADDRESS = 0x8;

// temariクラスのインスタンス
Temari myEffectProcessor;

// 受信したデータを格納するためのバッファとポインタ
// I2CのonReceive割り込み関数は非常に短く保つ必要があるため、
// データをバッファに格納し、loop()で処理するのが一般的です。
volatile uint8_t receivedSampleBuffer[128]; // I2Cで受信する8bitサンプル用バッファ
volatile uint16_t receivedWritePtr = 0;
volatile uint16_t receivedReadPtr = 0;

// I2Cデータを受信したときに実行される関数
// ISR (割り込みサービスルーチン) のため、非常にシンプルに保つ必要があります。
void receiveEvent(int howMany) {
  while (Wire.available()) {
    // Arduino 2は1バイト (8bit) でスケーリングされたサンプルを送ってくる想定
    uint8_t incomingByte = Wire.read();

    // バッファオーバーフローチェック
    uint16_t nextWritePos = (receivedWritePtr + 1) % sizeof(receivedSampleBuffer);
    if (nextWritePos == receivedReadPtr) {
        return;
    }
    receivedSampleBuffer[receivedWritePtr] = incomingByte;
    receivedWritePtr = nextWritePos;
  }
}

void setup() {
  Serial.begin(SERIAL_BAUDRATE);   // PCとのデバッグ用シリアル通信

  // I2Cスレーブとしてバスを開始、アドレス指定
  Wire.begin(MY_I2C_ADDRESS);
  // マスターからデータを受信したときにreceiveEvent関数を呼び出す
  Wire.onReceive(receiveEvent);
  // temariクラスの初期化
  // DACピンはA0を使うと仮定
  myEffectProcessor.init(SAMPLE_RATE, A0);
}

void loop() {
  // 受信バッファに処理すべきサンプルがあるか確認
  if (receivedReadPtr != receivedWritePtr) {
    // 1. 受信バッファからサンプルを読み出す
    uint8_t scaledSample = receivedSampleBuffer[receivedReadPtr];
    receivedReadPtr = (receivedReadPtr + 1) % sizeof(receivedSampleBuffer);

    // 2. 受け取った8bitサンプルをDACの12bitレンジに変換（単純なパススルー）
    // 0-255 の8bit値を 0-4095 の12bit値にスケーリング
    // 変換: val_12bit = val_8bit * (4095 / 255)
    // 4095 / 255 は約16.0588... なので、16倍に近い
    // 最も簡単なのは (val_8bit << 4) で下位4ビットをゼロ埋めすること
    // 正確なスケーリング: val_8bit * 16 + val_8bit / 16 (おおよそ)
    uint16_t dacValue = (uint16_t)scaledSample * 16; // 簡易スケーリング (255 -> 4080)
    // uint16_t dacValue = map(scaledSample, 0, 255, 0, 4095); // map関数も使える

    // 3. DACに出力
    analogWrite(A0, dacValue);
  }
  // リバーブ処理などは一旦保留
  // myEffectProcessor.processAndOutput(); // これはtemariにI2C受信機能を統合する場合に使う
}