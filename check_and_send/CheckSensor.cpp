#include "CheckSensor.h"

//コンストラクタ実装
MySensorWatcher::MySensorWatcher(const int sensorPins[], int numSensors) {
  _sensorPins = sensorPins;
  _numSensors = numSensors;

  _sensorStates = new int[_numSensors];
  _lastSensorStates = new int[_numSensors];

  for (int i = 0; i < _numSensors; i++) {
    pinMode(_sensorPins[i], INPUT_PULLUP);
    _lastSensorStates[i] = digitalRead(_sensorPins[i]); //前回の状態を初期化
    _sensorStates[i] = _lastSensorStates[i]; //現在の安定状態を初期化
  }
  _callback = nullptr; //コールバック初期化
}

MySensorWatcher::~MySensorWatcher() {
  delete[] _sensorStates;
  delete[] _lastSensorStates;
}

//センサの状態変化時に呼ばれる関数設定
void MySensorWatcher::onSensorStateChange(SensorStateChangeCallback callback) {
  _callback = callback;
}

//全てのセンサの状態をチェックする関数実装
void MySensorWatcher::checkAllSensors() {
  for (int i = 0; i < _numSensors; i++) {
    int currentReading = digitalRead(_sensorPins[i]); //現在のピンの状態を読み込む

    // 現在の読み取り値が前回の安定した状態と異なる場合
    if (currentReading != _lastSensorStates[i]) {
      // 短いディレイを挟んでチャタリングを待つ
      delay(10); 
      // 再度読み込み、本当に状態が変わったか確認 (チャタリングの揺り戻し対策)
      currentReading = digitalRead(_sensorPins[i]);
    }

    int stableState;
    if (currentReading == LOW && _lastSensorStates[i] == LOW) {
      stableState = LOW; // 安定してLOW（押されている）
    } else {
      stableState = HIGH; // それ以外はHIGH（離されている）と判断
    }

    // 安定した状態が前回のコールバック時の状態と異なる場合のみ、コールバックを呼び出す
    if (stableState != _sensorStates[i]) {
      if (_callback != nullptr) { // コールバックが設定されていれば呼び出す
        _callback(i, stableState);
      }
      _sensorStates[i] = stableState; // コールバックした状態を新しい安定状態として記録
    }

    // 次回の比較のために、現在の読み取り値を保存
    _lastSensorStates[i] = currentReading; 
  }
}