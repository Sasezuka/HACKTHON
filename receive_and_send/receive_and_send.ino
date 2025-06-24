#include <Arduino.h>
#include <Wire.h>      // I2C通信ライブラリを追加
#include "FspTimer.h"  // FspTimerクラスのヘッダファイル
#include "misuzu.h"    // misuzuクラスのヘッダファイル
#include "constants.h" // 全ての共通定数を含む

FspTimer timer;
misuzu myHarpController; // misuzuクラスのインスタンス

// --- I2Cスレーブデバイスのアドレス (Arduino 3のアドレス) ---
const int I2C_SLAVE_ADDRESS = 0x8; // Arduino 3のI2Cアドレスと一致させる

// --- ローカルボタン入力のための設定 (Arduino 1からのシリアル受信はloop()で処理) ---
// (このファイルではボタンは直接読み込まないが、定数として残しておく)
#define NUM_LOCAL_BUTTONS NUM_NOTES

// シリアル受信バッファの宣言（グローバルスコープ）
char receiveBuffer[MAX_RECEIVE_BUFFER_SIZE]; //受信データ保存用バッファ
byte receiveBufferIndex = 0; // データの現在保存量管理インデックス


// タイマー割り込みで呼ばれる、misuzu音源からサンプルを取得しI2Cで送信する関数
void callback_generateAndSendSample(timer_callback_args_t *arg) {
  // misuzuクラスから次のオーディオサンプルを取得 (0-4095 の12bit値)
  uint16_t rawSample = myHarpController.getNextSample();

  // 12ビットのサンプル値 (0-4095) を8ビットのI2C送信値 (0-255) にスケーリング
  // (float)rawSample / 4095.0f は 0.0f から 1.0f の範囲に正規化
  // それを 255.0f 倍することで 0.0f から 255.0f の範囲にする
  uint8_t scaledSample = (uint8_t)((float)rawSample / 4095.0f * 255.0f);

  // ★★★ ここがI2C送信への変更点 ★★★
  // I2Cでスレーブ (Arduino 3) に1バイトのデータを送信
  Wire.beginTransmission(I2C_SLAVE_ADDRESS);
  Wire.write(scaledSample); // スケーリングしたサンプルを送信
  Wire.endTransmission();
}

void setup() {
  Serial.begin(SERIAL_BAUDRATE);   // PCへのデバッグ出力 (USB Serial)
  Serial1.begin(SERIAL_BAUDRATE);  // Arduino 1からのノート情報受信 (Serial1のピンを使用)

  // I2Cマスターとしてバスを開始
  Wire.begin();

  // タイマー設定
  uint8_t type;
  int8_t ch = FspTimer::get_available_timer(type);
  if (ch < 0) {
    while (1) {
      delay(1000);
    }
  }
  timer.begin(TIMER_MODE_PERIODIC, type, ch, SAMPLE_RATE, 50.0f,
              callback_generateAndSendSample, nullptr);
  timer.setup_overflow_irq();
  timer.open();
  timer.start();
}

void loop() {
  // Arduino 1からのシリアルポートのデータを読み込む (N<noteIndex><state>\n 形式)
  while (Serial1.available()) {
    char inChar = (char)Serial1.read();

    if (receiveBufferIndex < (MAX_RECEIVE_BUFFER_SIZE - 1)) {
      receiveBuffer[receiveBufferIndex++] = inChar;
    } else {
      receiveBufferIndex = 0;
    }

    if (inChar == '\n') {
      receiveBuffer[receiveBufferIndex] = '\0'; // null終端

      // Arduino 1 は "B<ボタン番号>:<PUSH/RELEASE>" を送信
      if (receiveBuffer[0] == 'B' && receiveBufferIndex >= 3) { // 最低 "B1:P"
        int colonPos = -1;
        for (int i = 1; i < receiveBufferIndex; ++i) {
          if (receiveBuffer[i] == ':') {
            colonPos = i;
            break;
          }
        }

        if (colonPos != -1) {
          // ボタン番号の抽出
          String btnNumStr = String(receiveBuffer).substring(1, colonPos);
          int buttonIndex = btnNumStr.toInt(); // 1から始まるボタン番号

          // 状態文字列の抽出
          String stateStr = String(receiveBuffer).substring(colonPos + 1);
          stateStr.trim(); // 空白文字を除去

          // ボタン番号を0から始まるインデックスに変換
          int noteMapIndex = buttonIndex - 1; // 1番ボタンをnoteIndex 0にマップ

          if (noteMapIndex >= 0 && noteMapIndex < NUM_NOTES) {
            if (stateStr == "PUSH") { // ボタンが押された (INPUT_PULLUPなのでLOW)
              myHarpController.handlePlayCommand(noteMapIndex); // マップしたインデックスを渡す
            } else if (stateStr == "RELEASE") { // ボタンが離された (INPUT_PULLUPなのでHIGH)
              myHarpController.handleStopCommand();
            } else {
            }
          } else {
          }
        } else {
        }
      } else {
      }

      receiveBufferIndex = 0;
      receiveBuffer[0] = '\0';
    }
  }
}