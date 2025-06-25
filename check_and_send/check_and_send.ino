#include "CheckSensor.h"

const int SENSOR_PINS[] = {2, 3, 4, 5, 6, 7, 8, 9};
const int NUM_SENSORS = sizeof(SENSOR_PINS) / sizeof(SENSOR_PINS[0]);

MySensorWatcher myWatcher(SENSOR_PINS, NUM_SENSORS);

// センサの状態変化時に呼ばれる関数
void handleSensorStateChange(int sensorIndex, int state) {
  Serial1.print("B"); // ボタンイベントであることを示すプレフィックス
  Serial1.print(sensorIndex + 1); // ボタン番号 (1から始まる)
  Serial1.print(":"); // 区切り文字

  if (state == HIGH) { //光が当たっていない
    Serial1.println("RELEASE"); // 押されたことを示す
  } else { //光が当たっている
    Serial1.println("PUSH"); // 離されたことを示す
  }
}

void setup() {
  Serial1.begin(115200);  //Arduinoとの通信用

  myWatcher.onSensorStateChange(handleSensorStateChange);
}

void loop() {
  myWatcher.checkAllSensors();
}