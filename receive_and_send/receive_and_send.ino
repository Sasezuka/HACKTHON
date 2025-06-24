#include <Arduino.h>
#include <Wire.h>
#include "FspTimer.h"
#include "misuzu.h"
#include "constants.h"

FspTimer timer;
misuzu myHarpController;

const int I2C_SLAVE_ADDRESS = 0x8;

#define NUM_LOCAL_BUTTONS NUM_NOTES // このファイルでは直接使わないが、定数として残しておく

char receiveBuffer[MAX_RECEIVE_BUFFER_SIZE];
byte receiveBufferIndex = 0;

// --- I2Cバッファリングのためのグローバル変数 ---
#define AUDIO_BUFFER_SIZE 128 // オーディオバッファのサイズ (例: 128サンプル)
#define I2C_BATCH_SIZE 16     // 一度にI2Cで送信するサンプル数 (例: 16サンプル)

volatile uint8_t audioBuffer[AUDIO_BUFFER_SIZE];
volatile int bufferWriteIndex = 0; // ISRがバッファに書き込む位置
volatile int bufferReadIndex = 0;  // loop()がバッファから読み出す位置
volatile int bufferCount = 0;      // バッファ内に現在あるサンプル数

// タイマー割り込みで呼ばれる、misuzu音源からサンプルを取得しバッファに格納する関数
void callback_generateAndSendSample(timer_callback_args_t *arg) {
  uint16_t rawSample = myHarpController.getNextSample();
  uint8_t scaledSample = (uint8_t)((float)rawSample / 4095.0f * 255.0f);

  // ★ I2C送信はここで行わない ★

  // バッファが満タンでなければ、サンプルを格納
  // volatile変数にアクセスするため、割り込みを一時的に無効化 (安全のため)
  noInterrupts();
  if (bufferCount < AUDIO_BUFFER_SIZE) {
    audioBuffer[bufferWriteIndex] = scaledSample;
    bufferWriteIndex = (bufferWriteIndex + 1) % AUDIO_BUFFER_SIZE; // リングバッファのようにインデックスを更新
    bufferCount++;
  } else {
    // バッファオーバーフロー: データが生成速度より送信速度が遅い
    // Serial.println("Audio Buffer Overflow!"); // 割り込み内でのSerial.printは避けるべきだが、デバッグ用
  }
  interrupts();
}

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  Serial1.begin(SERIAL_BAUDRATE);
  Serial.println("--- Arduino 2 Setup Start (Synth Engine - I2C Buffered) ---");

  // I2Cマスターとしてバスを開始し、高速化設定
  Wire.begin();
  Wire.setClock(400000); // I2Cクロックを400kHzに設定 (DACが対応していれば1MHzも試す)
  Serial.println("I2C Master initialized at 400kHz.");

  uint8_t type;
  int8_t ch = FspTimer::get_available_timer(type);
  if (ch < 0) {
    Serial.println("Error: No available timer for audio generation!");
    while (1) {
      delay(1000);
    }
  }
  timer.begin(TIMER_MODE_PERIODIC, type, ch, SAMPLE_RATE, 50.0f,
              callback_generateAndSendSample, nullptr);
  timer.setup_overflow_irq();
  timer.open();
  timer.start();

  Serial.println("misuzu controller initialized.");
  Serial.println("Arduino 2 Setup Complete. Waiting for note commands from Arduino 1 via Serial1.");
}

void loop() {
  // --- シリアル受信処理 ---
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

      if (receiveBuffer[0] == 'B' && receiveBufferIndex >= 3) {
        char* colonPtr = strchr(receiveBuffer + 1, ':');

        if (colonPtr != nullptr) {
          *colonPtr = '\0';
          int buttonIndex = atoi(receiveBuffer + 1);

          char* statePtr = colonPtr + 1;
          size_t len = strlen(statePtr);
          if (len > 0 && statePtr[len-1] == '\r') {
            statePtr[len-1] = '\0';
          }
          if (len > 0 && statePtr[len-1] == '\n') {
            statePtr[len-1] = '\0';
          }

          int noteMapIndex = buttonIndex - 1;

          if (noteMapIndex >= 0 && noteMapIndex < NUM_NOTES) {
            if (strncmp(statePtr, "PUSH", 4) == 0) {
              Serial.print(">> Received: Button ");
              Serial.print(buttonIndex);
              Serial.println(" PUSH - Starting synthesis.");
              myHarpController.handlePlayCommand(noteMapIndex);
            } else if (strncmp(statePtr, "RELEASE", 7) == 0) {
              Serial.print(">> Received: Button ");
              Serial.print(buttonIndex);
              Serial.println(" RELEASE - Stopping synthesis.");
              myHarpController.handleStopCommand();
            } else {
              Serial.print("Error: Invalid state string received: ");
              Serial.println(statePtr);
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

  // --- I2Cバッファからの送信処理 ---
  // バッファに送信可能なデータ（バッチサイズ以上）がある場合
  // volatile変数アクセスとbufferCount, bufferReadIndex更新のため、noInterrupts()で囲む
  noInterrupts();
  if (bufferCount >= I2C_BATCH_SIZE) {
    // 送信するバッチの先頭インデックスとサイズを一時変数にコピー
    int currentReadBatchIndex = bufferReadIndex;
    int samplesToProcess = I2C_BATCH_SIZE;

    // 割り込みを再度有効にする (I2C送信は時間がかかるため、ISRが動けるようにする)
    interrupts();

    Wire.beginTransmission(I2C_SLAVE_ADDRESS);
    for (int i = 0; i < samplesToProcess; i++) {
      // バッファからサンプルを読み出し、I2Cで書き込む
      Wire.write(audioBuffer[currentReadBatchIndex]);
      currentReadBatchIndex = (currentReadBatchIndex + 1) % AUDIO_BUFFER_SIZE;
    }
    Wire.endTransmission();

    // I2C送信完了後、バッファの状態を更新 (安全のため、再度割り込み無効化)
    noInterrupts();
    bufferReadIndex = currentReadBatchIndex;
    bufferCount -= samplesToProcess;
    interrupts();
  } else {
    // バッファに十分なデータがない場合は、CPUを少し休ませる（他のタスクの実行を助ける）
    // delay(1); // 必要に応じてコメント解除。厳密なリアルタイム性には注意
  }
}

