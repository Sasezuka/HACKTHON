#include "FspTimer.h"
#include "constants.h"
#include "misuzu.h"

//グローバル変数
FspTimer timer;
misuzu myHarpController;

char receiveBuffer[MAX_RECEIVE_BUFFER_SIZE];
byte receiveBufferIndex = 0;

//音声用バッファ
#define AUDIO_BUFFER_SIZE 256
volatile uint16_t audioBuffer[AUDIO_BUFFER_SIZE];
volatile int bufferWriteIndex = 0;
volatile int bufferReadIndex = 0;
volatile int bufferCount = 0;

//コールバック関数定義
void callback_generateSample(timer_callback_args_t *arg) {
    if (bufferCount < AUDIO_BUFFER_SIZE) {
        uint16_t rawSample = myHarpController.getNextSample();
        audioBuffer[bufferWriteIndex] = rawSample;
        bufferWriteIndex = (bufferWriteIndex + 1) % AUDIO_BUFFER_SIZE;
        bufferCount++;
    }
}

void setup() {
    // USBシリアルをデバッグ表示用に初期化
    Serial.begin(SERIAL_BAUDRATE);
    Serial.println("\nArduino1セットアップ開始");

    //シリアル通信のために初期化
    Serial1.begin(SERIAL_BAUDRATE);
    // Serial.println("デバッグ表示");

    //DAC初期化
    pinMode(A0, OUTPUT);
    analogWriteResolution(12);
    // Serial.println("DACの初期化を行いました");

    //サンプル生成用タイマーのセットアップ
    uint8_t type;
    int8_t ch = FspTimer::get_available_timer(type);
    if (ch < 0) {
        Serial.println("Error: No available timer!");
        while (1) delay(1000);
    }
    timer.begin(TIMER_MODE_PERIODIC, type, ch, SAMPLE_RATE, 50.0f, callback_generateSample, nullptr);
    timer.setup_overflow_irq();
    timer.open();
    timer.start();
    // Serial.print("タイマースタート，サンプルレートは");
    // Serial.print(SAMPLE_RATE);
    // Serial.println("Hzです");

    Serial.println("セットアップが完了しました");
}

//シリアルコマンドを処理する関数
void processSerialCommand() {
    while (Serial1.available()) {
        char inChar = (char)Serial1.read();

        if (inChar == '\n' || inChar == '\r') {
            if (receiveBufferIndex > 0) {
                receiveBuffer[receiveBufferIndex] = '\0';

                if (receiveBuffer[0] == 'B' && receiveBufferIndex >= 3) {
                    char* colonPtr = strchr(receiveBuffer, ':');
                    if (colonPtr != nullptr) {
                        *colonPtr = '\0';
                        int buttonIndex = atoi(receiveBuffer + 1);
                        char* statePtr = colonPtr + 1;
                        int noteMapIndex = buttonIndex - 1;

                        if (noteMapIndex >= 0 && noteMapIndex < NUM_NOTES) {
                            if (strcmp(statePtr, "PUSH") == 0) {
                                //デバッグ表示
                                Serial.print(">> NOTE ON (from Serial1): "); Serial.println(buttonIndex);
                                myHarpController.handlePlayCommand(noteMapIndex);
                            } else if (strcmp(statePtr, "RELEASE") == 0) {
                                Serial.print(">> NOTE OFF (from Serial1): "); Serial.println(buttonIndex);
                                myHarpController.handleStopCommand();
                            }
                        }
                    }
                }
                receiveBufferIndex = 0;
            }
        } else {
            if (receiveBufferIndex < (MAX_RECEIVE_BUFFER_SIZE - 1)) {
                receiveBuffer[receiveBufferIndex++] = inChar;
            }
        }
    }
}

void loop() {
    //Serial1からコマンド処理
    processSerialCommand();

    uint16_t sampleToPlay;
    bool hasSample = false;

    noInterrupts();
    if (bufferCount > 0) {
        sampleToPlay = audioBuffer[bufferReadIndex];
        bufferReadIndex = (bufferReadIndex + 1) % AUDIO_BUFFER_SIZE;
        bufferCount--;
        hasSample = true;
    }
    interrupts();

    if (hasSample) {
        analogWrite(A0, sampleToPlay);
    }
}
