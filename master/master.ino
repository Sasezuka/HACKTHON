#include <Wire.h>
#include <math.h> // sin関数を使うために必要

// スレーブのI2Cアドレス
const int SLAVE_ADDRESS = 0x8;

// 正弦波のパラメータ
const float amplitude = 127.5; // 0-255の範囲に収めるための振幅の中心
const float offset = 127.5;    // 0-255の範囲に収めるためのオフセット

float angle = 0.0;             // 現在の角度（ラジアン）
const float angleIncrement = M_PI / 16.0; // 角度の増加量（PI/16で約32ステップ/周期）

void setup() {
  Wire.begin(); // マスターとしてI2Cバスを開始
  Serial.begin(115200);
  Serial.println("Master Arduino Ready (Sending Sine Wave)!");
}

void loop() {
  // 正弦波の値を計算 (0〜255の整数値)
  int sineValue = (int)(amplitude * sin(angle) + offset);

  // 送信範囲の確認（念のため）
  if (sineValue < 0) sineValue = 0;
  if (sineValue > 255) sineValue = 255;

  Serial.print("Master: Sending sine value: ");
  Serial.println(sineValue);

  // I2Cでスレーブに1バイトのデータを送信
  Wire.beginTransmission(SLAVE_ADDRESS);
  Wire.write((byte)sineValue); // 整数値をバイト型にキャストして送信
  Wire.endTransmission();

  // 角度を更新
  angle += angleIncrement;
  if (angle >= 2.0 * M_PI) { // 2π（360度）を超えたらリセット
    angle -= 2.0 * M_PI;
  }

  delay(50); // 送信間隔（波形の滑らかさに影響）
}