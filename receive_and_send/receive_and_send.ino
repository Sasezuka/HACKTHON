#include "FspTimer.h"
#include "misuzu.h"
#include "constants.h" 
// #include <SPI.h> // SPIライブラリは不要になるのでコメントアウトまたは削除

FspTimer timer;
misuzu myHarpController; // misuzuクラスのインスタンス

// SPI_SS_PINは不要になるので削除
// const int SPI_SS_PIN = 10; 

// シリアル受信バッファの宣言（グローバルスコープ）
char receiveBuffer[MAX_RECEIVE_BUFFER_SIZE]; //受信データ保存用バッファ
byte receiveBufferIndex = 0; // データの現在保存量管理インデックス


// タイマー割り込みで呼ばれる、音源からサンプルを取得しSerial1で送信する関数
void callback_generateAndSendSample(timer_callback_args_t *arg) {
  // getNextSample() は 0-4095 の12bit値を返す想定
  uint16_t sampleToSend = myHarpController.getNextSample(); 

  // ★★★ 変更点：Serial1 でオーディオサンプルをArduino 3に送信 ★★★
  // 12bitデータを2バイトで送信
  // sampleToSend は 0-4095 の範囲 (12bit)
  // 受信側が上位8bit, 下位4bitの順序を期待するなら、SPIと同じロジックを使う
  // 通常のシリアル通信では、単純に2バイトを連続して送るのが一般的です。
  // その場合、受信側は2バイトをそのまま16bitデータとして結合します。
  
  // 例: 16bitデータを2バイトで送信 (0-4095の範囲なら上位4bitは常に0x00)
  byte highByte = (sampleToSend >> 8) & 0xFF; // 上位バイト
  byte lowByte = sampleToSend & 0xFF;       // 下位バイト

  Serial1.write(highByte); // 上位バイトを送信
  Serial1.write(lowByte);  // 下位バイトを送信

  // SPI関連のコードは削除またはコメントアウト
  // digitalWrite(SPI_SS_PIN, LOW); 
  // SPI.beginTransaction(SPISettings(24000000, MSBFIRST, SPI_MODE0)); 
  // SPI.transfer((sampleToSend >> 4) & 0xFF); 
  // SPI.transfer(sampleToSend & 0x0F);       
  // SPI.endTransaction(); 
  // digitalWrite(SPI_SS_PIN, HIGH); 
}

void setup() {
  Serial.begin(SERIAL_BAUDRATE);   // PCへのデバッグ出力 (USB Serial)
  // ★★★ 修正点：Serial1 のボーレートを constants.h の値に合わせる ★★★
  // SPIは非常に高速でしたが、UARTもできるだけ高速に設定
  Serial1.begin(SERIAL_BAUDRATE);  // Arduino 1からのノート情報受信 (Serial1のピンを使用)
  Serial.println("---  Arduino 2 Setup Start (Synth Engine) ---");

  // タイマー設定
  uint8_t type;
  int8_t ch = FspTimer::get_available_timer(type);
  if (ch < 0) {
    Serial.println("Error: No available timer for audio generation!");
    while (1) {
      delay(1000); 
    }
  }
  // ★★★ 修正点：タイマー割り込み周期をサンプルレート Hz で設定 ★★★
  timer.begin(TIMER_MODE_PERIODIC, type, ch, SAMPLE_RATE, 50.0f, 
              callback_generateAndSendSample, nullptr);
  timer.setup_overflow_irq();
  timer.open();
  timer.start();

  // SPIのセットアップ (Masterモード) は不要になるので削除またはコメントアウト
  // pinMode(SPI_SS_PIN, OUTPUT);
  // digitalWrite(SPI_SS_PIN, HIGH); 
  // SPI.begin();                    
  
  Serial.println("Arduino 2 Setup Complete.");
}

void loop() {
  // Arduino 1からのシリアルポートのデータを読み込む (N<noteIndex><state>\n 形式)
  while (Serial1.available()) {
    char inChar = (char)Serial1.read();

    if (receiveBufferIndex < (MAX_RECEIVE_BUFFER_SIZE - 1)) {
      receiveBuffer[receiveBufferIndex++] = inChar;
    } else {
      receiveBufferIndex = 0;
      Serial.println("Warning: Receive buffer overflow!");
    }

    if (inChar == '\n') {
      receiveBuffer[receiveBufferIndex] = '\0'; // null終端
      
      // デバッグ出力はパフォーマンスに影響するため、テスト時以外はコメントアウト推奨
      // Serial.print("[Debug] Received via Serial1: ");
      // Serial.println(receiveBuffer);

      // ★★★ 修正点：Arduino 1 のボタンイベントフォーマットに対応 ★★★
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

          // ボタン番号を0から始まるインデックスに変換 (もし必要なら)
          // 例: ボタン1 -> noteIndex 0, ボタン2 -> noteIndex 1 など
          int noteMapIndex = buttonIndex - 1; // 1番ボタンをnoteIndex 0にマップ

          if (noteMapIndex >= 0 && noteMapIndex < NUM_NOTES) { 
            if (stateStr == "PUSH") { // ボタンが押された (INPUT_PULLUPなのでLOW)
              Serial.print(">> Button ");
              Serial.print(buttonIndex);
              Serial.println(" PUSH - Starting synthesis.");
              myHarpController.handlePlayCommand(noteMapIndex); // マップしたインデックスを渡す
            } else if (stateStr == "RELEASE") { // ボタンが離された (INPUT_PULLUPなのでHIGH)
              Serial.print(">> Button ");
              Serial.print(buttonIndex);
              Serial.println(" RELEASE - Stopping synthesis.");
              myHarpController.handleStopCommand(); 
            } else {
              Serial.print("Error: Invalid state string received: ");
              Serial.println(stateStr);
            }
          } else {
            Serial.print("Error: Invalid mapped note index: ");
            Serial.println(noteMapIndex);
          }
        } else {
          Serial.println("Error: Invalid serial command format (missing colon).");
        }
      } else {
        Serial.println("Error: Invalid serial command format (missing 'B' prefix).");
      }
      
      receiveBufferIndex = 0;
      receiveBuffer[0] = '\0'; 
    }
  }
}