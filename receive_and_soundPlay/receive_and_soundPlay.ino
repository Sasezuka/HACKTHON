#include "FspTimer.h"
#include "constants.h"
#include "misuzu.h"

// --- グローバル変数 ---
FspTimer timer;
misuzu myHarpController;

char receiveBuffer[MAX_RECEIVE_BUFFER_SIZE];
byte receiveBufferIndex = 0;

// オーディオバッファ
#define AUDIO_BUFFER_SIZE 256
volatile uint16_t audioBuffer[AUDIO_BUFFER_SIZE];
volatile int bufferWriteIndex = 0;
volatile int bufferReadIndex = 0;
volatile int bufferCount = 0;

// --- タイマー割り込みコールバック ---
void callback_generateSample(timer_callback_args_t *arg) {
    if (bufferCount < AUDIO_BUFFER_SIZE) {
        uint16_t rawSample = myHarpController.getNextSample();
        audioBuffer[bufferWriteIndex] = rawSample;
        bufferWriteIndex = (bufferWriteIndex + 1) % AUDIO_BUFFER_SIZE;
        bufferCount++;
    }
}

// --- 初期設定 ---
void setup() {
    // USBシリアルをデバッグ出力用に初期化
    Serial.begin(SERIAL_BAUDRATE);
    Serial.println("\n--- R4 WiFi Standalone Synth ---");

    // ハードウェアシリアル(D0/RX, D1/TX)をコマンド受信用に初期化
    Serial1.begin(SERIAL_BAUDRATE);
    Serial.println("Listening for commands on Serial1 (Pins D0, D1).");
    Serial.println("Debug output will be shown here.");

    // DACの初期化 (A0ピン)
    pinMode(A0, OUTPUT);
    analogWriteResolution(12);
    Serial.println("DAC initialized on A0 (12-bit).");

    // サンプル生成用タイマーのセットアップ
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
    Serial.print("Audio generation timer started at ");
    Serial.print(SAMPLE_RATE);
    Serial.println(" Hz.");

    Serial.println("Setup complete. Waiting for commands on Serial1...");
}

// --- シリアルコマンド処理 ---
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
                                // デバッグメッセージはUSBシリアル(Serial)に出力
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

// --- メインループ (改善版) ---
void loop() {
    // Serial1からのコマンドを処理
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
