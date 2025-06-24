#include <Arduino.h>
#include "FspTimer.h"  // FspTimerクラスのヘッダファイル
#include "rinha.h"     // インクルードファイルを修正 (misuzu.h -> rinha.h)
#include "constants.h" // 全ての共通定数を含む

FspTimer timer;
// misuzuをrinhaに変更
rinha myHarpController; // rinhaクラスのインスタンス

// --- ローカルボタン入力のための設定 ---
#define NUM_LOCAL_BUTTONS NUM_NOTES 

// ボタンの状態を管理する構造体
struct LocalButtonState {
  int   buttonPin;         // デジタルピン番号
  int   currentButtonState; // 現在のボタン状態
  int   lastButtonState;   // 前回のボタン状態 (デバウンス用)
  int   assignedNoteIndex; // rinhaに渡すノートインデックス (0-NUM_NOTES-1)
};

LocalButtonState localButtons[NUM_LOCAL_BUTTONS];

// 現在どのノートが物理的に押されているかを追跡（モノフォニック制御のため）
int currentActiveButtonNoteIndex = -1; // -1はどのボタンも押されていない状態


// タイマー割り込みで呼ばれる、rinha音源からサンプルを取得しDACに出力する関数
void callback_generateAndSendSample(timer_callback_args_t *arg) {
  // rinhaクラスから次のオーディオサンプルを取得 (0-4095 の12bit値)
  uint16_t sampleToOutput = myHarpController.getNextSample(); 

  // A0ピンのDACに出力
  analogWrite(A0, sampleToOutput); 
}

void setup() {
  Serial.begin(SERIAL_BAUDRATE);   // PCへのデバッグ出力 (USB Serial)
  Serial.println("--- Arduino 2 セットアップ開始 (スタンドアロン rinha シンセ - デバッグ) ---"); // 表示メッセージを修正

  // DAC (Digital to Analog Converter) の解像度を12ビットに設定します。
  analogWriteResolution(12); // 0～4095 の範囲
  pinMode(A0, OUTPUT);       // A0ピンを出力に設定

  // FspTimer設定 (rinha音源のサンプル生成・DAC出力のため)
  uint8_t type;
  int8_t ch = FspTimer::get_available_timer(type);
  if (ch < 0) {
    Serial.println("エラー: オーディオ生成用の利用可能なタイマーがありません！");
    while (1) {
      delay(1000); 
    }
  }
  timer.begin(TIMER_MODE_PERIODIC, type, ch, SAMPLE_RATE, 50.0f, 
              callback_generateAndSendSample, nullptr);
  timer.setup_overflow_irq(); // オーバーフロー割り込みを設定
  timer.open();   // タイマーを開く
  timer.start();  // タイマーを開始

  // --- ローカルボタンの初期設定 ---
  int buttonPins[] = {2, 3, 4, 5, 6, 7, 8, 9}; 

  for (int i = 0; i < NUM_LOCAL_BUTTONS; i++) {
    localButtons[i].buttonPin = buttonPins[i];
    localButtons[i].currentButtonState = LOW; 
    localButtons[i].lastButtonState = LOW; 
    localButtons[i].assignedNoteIndex = i; 
    
    pinMode(localButtons[i].buttonPin, INPUT); 
  }

  // rinhaインスタンスの初期化はrinha::rinha()コンストラクタで自動的に行われます。
  Serial.println("rinhaコントローラーを初期化しました。"); // 表示メッセージを修正

  Serial.println("Arduino 2 セットアップ完了。D2-D9のボタンを押してrinhaシンセを試してください！"); // 表示メッセージを修正
}

void loop() {
  // --- ローカルボタンの状態を継続的に監視します ---
  int  currentlyPressedButtonNoteIndex = -1; 

  for (int i = 0; i < NUM_LOCAL_BUTTONS; i++) {
    localButtons[i].currentButtonState = digitalRead(localButtons[i].buttonPin);

    // デバウンス処理
    if (localButtons[i].currentButtonState != localButtons[i].lastButtonState) {
      delay(10); 
      localButtons[i].currentButtonState = digitalRead(localButtons[i].buttonPin); 
    }
    
    // 「HIGHかつHIGHの時になる仕組み」を適用
    if (localButtons[i].lastButtonState == HIGH && localButtons[i].currentButtonState == HIGH) {
      currentlyPressedButtonNoteIndex = localButtons[i].assignedNoteIndex; 
    }
    
    // デバッグ用（オプション）：ボタン状態の変化をシリアルモニターに出力したい場合はコメント解除してください。
    // if (localButtons[i].currentButtonState != localButtons[i].lastButtonState) {
    //   Serial.print("ボタン "); Serial.print(localButtons[i].buttonPin);
    //   Serial.print(" 状態変化: "); 
    //   Serial.println(localButtons[i].currentButtonState == HIGH ? "HIGH" : "LOW");
    // }

    localButtons[i].lastButtonState = localButtons[i].currentButtonState; 
  }

  // --- rinhaへの音の再生制御 ---
  if (currentlyPressedButtonNoteIndex != -1) {
    // いずれかのボタンが押されている場合
    if (currentActiveButtonNoteIndex != currentlyPressedButtonNoteIndex) {
      myHarpController.handlePlayCommand(currentlyPressedButtonNoteIndex);
      currentActiveButtonNoteIndex = currentlyPressedButtonNoteIndex; 
      Serial.print("rinhaでノート再生: インデックス "); // 表示メッセージを修正
      Serial.print(currentActiveButtonNoteIndex);
      Serial.print(" 周波数: ");
      Serial.println(NOTE_FREQUENCIES[currentActiveButtonNoteIndex]);
    }
  } else {
    // どのボタンもHIGH状態を継続していない場合
    if (currentActiveButtonNoteIndex != -1) {
      myHarpController.handleStopCommand(); 
      currentActiveButtonNoteIndex = -1; 
      Serial.println("rinhaの音を停止しました (リリースフェーズ開始)。"); // 表示メッセージを修正
    }
  }
}