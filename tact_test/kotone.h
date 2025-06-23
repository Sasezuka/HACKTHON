#ifndef kotone_h
#define kotone_h

#include "Arduino.h"

// ボタンの状態変化を通知するためのコールバック関数の型定義
// 引数: buttonIndex (ボタンの番号 0から始まる), state (HIGH/LOW)
typedef void (*ButtonStateChangeCallback)(int buttonIndex, int state);

class MyButtonWatcher {
  public:
    MyButtonWatcher(const int buttonPins[], int numButtons);
    
    // シリアルポートを設定する関数 (今回は使わないが、応用としてSerial1などを使う場合に便利)
    // void setSerialPort(HardwareSerial& serialPort); // 例

    // ボタンの状態を全てチェックし、変化があったらコールバック関数を呼び出す
    void checkAllButtons();

    // ボタンの状態変化時に呼ばれる関数を設定する
    void onButtonStateChange(ButtonStateChangeCallback callback);

  private:
    const int* _buttonPins;
    int _numButtons;
    int* _buttonStates;
    int* _lastButtonStates;
    ButtonStateChangeCallback _callback; // 登録されたコールバック関数
};

#endif