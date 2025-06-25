#include "CheckSensor.h"

// コンストラクタの実装
MySensorWatcher::MySensorWatcher(const int buttonPins[], int numSensors) {
  _buttonPins = buttonPins;
  _numSensors = numSensors;

  _buttonStates = new int[_numSensors];
  _lastSensorStates = new int[_numSensors];
  // _lastDebounceTime = new unsigned long[_numSensors]; // ← この行は不要になります

  for (int i = 0; i < _numSensors; i++) {
    // レーザーハープの前提 (遮ったらLOW) に合わせ、INPUT_PULLUP を使用
    pinMode(_buttonPins[i], INPUT_PULLUP);
    _lastSensorStates[i] = digitalRead(_buttonPins[i]); // 前回の状態を初期化
    _buttonStates[i] = _lastSensorStates[i]; // 現在の安定状態を初期化
    // _lastDebounceTime[i] = 0; // ← この行は不要になります
  }
  _callback = nullptr; // コールバックを初期化
}

// デストラクタ (メモリ解放のため。この実装の場合、_lastDebounceTimeのdeleteも不要になる)
// もしコンストラクタから _lastDebounceTime の new を削除したら、ここからも削除してください
MySensorWatcher::~MySensorWatcher() {
  delete[] _buttonStates;
  delete[] _lastSensorStates;
  // delete[] _lastDebounceTime; // ← この行は不要になります
}

// ボタンの状態変化時に呼ばれる関数を設定
void MySensorWatcher::onSensorStateChange(SensorStateChangeCallback callback) {
  _callback = callback;
}

// 全てのボタンの状態をチェックする関数の実装
void MySensorWatcher::checkAllSensors() {
  for (int i = 0; i < _numSensors; i++) {
    int currentReading = digitalRead(_buttonPins[i]); // 現在のピンの状態を読み込む

    // 現在の読み取り値が前回の安定した状態と異なる場合
    if (currentReading != _lastSensorStates[i]) {
      // 短いディレイを挟んでチャタリングを待つ
      delay(10); 
      // 再度読み込み、本当に状態が変わったか確認 (チャタリングの揺り戻し対策)
      currentReading = digitalRead(_buttonPins[i]);
    }

    // ★ ここが参考コードのデバウンスロジックです ★
    // レーザーを遮ったらLOWになる前提で、LOWが安定しているかを確認
    // 前回がLOWで、今回もLOWなら、安定して押されている（レーザーが遮られている）と判断
    int stableState;
    if (currentReading == LOW && _lastSensorStates[i] == LOW) {
      stableState = LOW; // 安定してLOW（押されている）
    } else {
      stableState = HIGH; // それ以外はHIGH（離されている）と判断
    }

    // 安定した状態が前回のコールバック時の状態と異なる場合のみ、コールバックを呼び出す
    if (stableState != _buttonStates[i]) {
      if (_callback != nullptr) { // コールバックが設定されていれば呼び出す
        _callback(i, stableState);
      }
      _buttonStates[i] = stableState; // コールバックした状態を新しい安定状態として記録
    }

    // 次回の比較のために、現在の読み取り値を保存
    _lastSensorStates[i] = currentReading; 
  }
}