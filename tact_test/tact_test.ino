#include "kotone.h"

const int BUTTON_PINS[] = {2, 3, 4, 5, 6, 7, 8, 9};
const int NUM_BUTTONS = sizeof(BUTTON_PINS) / sizeof(BUTTON_PINS[0]);

MyButtonWatcher myWatcher(BUTTON_PINS, NUM_BUTTONS);

// ボタンの状態変化時に呼ばれる関数
void handleButtonStateChange(int buttonIndex, int state) {
  // Serial1を使ってデータを送信します
  Serial1.print("B"); // ボタンイベントであることを示すプレフィックス
  Serial1.print(buttonIndex + 1); // ボタン番号 (1から始まる)
  Serial1.print(":"); // 区切り文字

  // INPUT_PULLUPの場合、ボタンが押されるとLOWになるので注意
  if (state == LOW) { // ボタンが押された状態
    Serial1.println("RELEASE"); // 押されたことを示す
  } else { // ボタンが離された状態
    Serial1.println("PUSH"); // 離されたことを示す
  }
}

void setup() {
  Serial1.begin(115200);  // 他のArduinoとの通信用 (ボーレートは受信側と合わせる)

  myWatcher.onButtonStateChange(handleButtonStateChange);
}

void loop() {
  myWatcher.checkAllButtons();
}