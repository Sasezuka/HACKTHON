#include "kotone.h"

// コンストラクタの実装
MyButtonWatcher::MyButtonWatcher(const int buttonPins[], int numButtons) {
  _buttonPins = buttonPins;
  _numButtons = numButtons;

  _buttonStates = new int[_numButtons];
  _lastButtonStates = new int[_numButtons];

  for (int i = 0; i < _numButtons; i++) {
    pinMode(_buttonPins[i], INPUT_PULLUP); // INPUT_PULLUPに変更 (後述)
    _lastButtonStates[i] = digitalRead(_buttonPins[i]);
    _buttonStates[i] = _lastButtonStates[i];
  }
  _callback = nullptr; // コールバックを初期化
}

// ボタンの状態変化時に呼ばれる関数を設定
void MyButtonWatcher::onButtonStateChange(ButtonStateChangeCallback callback) {
  _callback = callback;
}

// 全てのボタンの状態をチェックする関数の実装
void MyButtonWatcher::checkAllButtons() {
  for (int i = 0; i < _numButtons; i++) {
    int currentButtonState = digitalRead(_buttonPins[i]);

    if (currentButtonState != _lastButtonStates[i]) {
      delay(50); // チャタリング対策 (少し長めにして確実に状態が安定するのを待つ)
      // 再度読み込み、本当に状態が変わったか確認 (チャタリングの揺り戻し対策)
      currentButtonState = digitalRead(_buttonPins[i]); 
      if (currentButtonState != _lastButtonStates[i]) { // 再度確認
        if (_callback != nullptr) { // コールバックが設定されていれば呼び出す
          _callback(i, currentButtonState);
        }
        _lastButtonStates[i] = currentButtonState;
      }
    }
  }
}