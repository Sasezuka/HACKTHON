#include "rinha.h"   // インクルードファイルを修正
#include <Arduino.h>     // Arduinoの関数を使用するため
#include "constants.h"   

// コンストラクタ
// クラス名をmisuzuからrinhaに変更
rinha::rinha() :
    _currentNoteIndex(-1),
    _noteOffStartTime(0),
    _isReleasing(false)
{
    // kayaインスタンスを初期化（波形生成を含む）
    // hatayaをkayaに変更
    for (int i = 0; i < NUM_NOTES; ++i) {
        _voices[i].init(NOTE_FREQUENCIES[i], BASE_AMPLITUDE, HARMONICS, NUM_HARMONICS, SAMPLE_RATE);
    }
}

// 外部コマンド（シリアル受信など）から音を再生する
// クラス名をmisuzuからrinhaに変更
void rinha::handlePlayCommand(int noteIndex) {
    // 既に音が鳴っている場合、古い音を停止
    if (_currentNoteIndex != -1) {
        _voices[_currentNoteIndex].noteOff(); 
    }
    
    // 新しい音を再生開始
    _currentNoteIndex = noteIndex;
    _voices[_currentNoteIndex].noteOn(); // 新しい音を再生
    _isReleasing = false; // 再生中はリリース中ではない
    _noteOffStartTime = 0; // 停止タイマーをリセット
}

// 外部コマンド（シリアル受信など）から音を停止する
// クラス名をmisuzuからrinhaに変更
void rinha::handleStopCommand() {
    // 現在鳴っている音がある場合、それを停止
    if (_currentNoteIndex != -1) {
        _voices[_currentNoteIndex].noteOff(); // 音を停止
        _isReleasing = true; // リリース中に設定
        _noteOffStartTime = millis(); // リリース開始時間を記録
    }
}

// クラス名をmisuzuからrinhaに変更
uint16_t rinha::getNextSample() {
    float totalSampleFloat = 0.0f; // 全ての音源の合計サンプル値

    // 全てのkayaインスタンスからサンプルを取得し、合計する
    for (int i = 0; i < NUM_NOTES; ++i) {
        // kaya::getNextSample() は既にMAX_AMPLITUDEが加算されているので、
        // 一旦MAX_AMPLITUDEを引いて中心0に戻してから加算する
        totalSampleFloat += ((float)_voices[i].getNextSample() - MAX_AMPLITUDE);
    }
    // DACの出力範囲にクリッピングして、0-4095の範囲に戻す
    totalSampleFloat += MAX_AMPLITUDE; // DACの中心値を加算

    if (totalSampleFloat < 0.0f) {
        totalSampleFloat = 0.0f;
    } else if (totalSampleFloat > 4095.0f) { // 12bit DACの場合、0-4095の範囲
        totalSampleFloat = 4095.0f;
    }

    return (uint16_t)totalSampleFloat; // 16bit unsigned int で返す
}