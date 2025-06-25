#ifndef CheckSensor_h
#define CheckSensor_h

#include "Arduino.h"

// ボタンの状態変化を通知するためのコールバック関数の型定義
// 引数: buttonIndex (ボタンの番号 0から始まる), state (HIGH/LOW)
typedef void (*SensorStateChangeCallback)(int sensorIndex, int state);

class MySensorWatcher {
  public:
    MySensorWatcher(const int buttonPins[], int numSensors);

    // ボタンの状態を全てチェックし、変化があったらコールバック関数を呼び出す
    void checkAllSensors();

    // ボタンの状態変化時に呼ばれる関数を設定する
    void onSensorStateChange(SensorStateChangeCallback callback);

  private:
    const int* _buttonPins;
    int _numSensors;
    int* _buttonStates;
    int* _lastSensorStates;
    SensorStateChangeCallback _callback; // 登録されたコールバック関数
};

#endif