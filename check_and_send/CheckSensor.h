#ifndef CheckSensor_h
#define CheckSensor_h

#include "Arduino.h"

//センサの状態変化を通知するコールバック関数の定義
typedef void (*SensorStateChangeCallback)(int sensorIndex, int state);

class MySensorWatcher {
  public:
    MySensorWatcher(const int sensorPins[], int numSensors);

    //センサの状態をチェックし，変化があったらコールバック関数を呼び出す
    void checkAllSensors();

    //センサの状態変化時に呼ばれる関数を設定
    void onSensorStateChange(SensorStateChangeCallback callback);

  private:
    const int* _sensorPins;
    int _numSensors;
    int* _sensorStates;
    int* _lastSensorStates;
    SensorStateChangeCallback _callback;
};

#endif