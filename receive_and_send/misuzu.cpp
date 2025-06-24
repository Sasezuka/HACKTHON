#include "misuzu.h"
#include <Arduino.h>     // Arduinoの関数を使用するため
#include "constants.h"   

// コンストラクタ
misuzu::misuzu() :
    _currentNoteIndex(-1),
    _noteOffStartTime(0),
    _isReleasing(false)
{
    // hatayaインスタンスを初期化（波形生成を含む）
    for (int i = 0; i < NUM_NOTES; ++i) {
        _voices[i].init(NOTE_FREQUENCIES[i], BASE_AMPLITUDE, HARMONICS, NUM_HARMONICS, SAMPLE_RATE);
    }
}

// 外部コマンド（シリアル受信など）から音を再生する
void misuzu::handlePlayCommand(int noteIndex) {
    if (_currentNoteIndex != -1) {
        _voices[_currentNoteIndex].noteOff(); // 古い音を停止
    }
    
    // 新しい音を再生開始
    _currentNoteIndex = noteIndex;
    _voices[_currentNoteIndex].noteOn(); // 新しい音を再生
    _isReleasing = false; // 再生中はリリース中ではない
    _noteOffStartTime = 0; // 停止タイマーをリセット
}

// 外部コマンド（シリアル受信など）から音を停止する
void misuzu::handleStopCommand() {
    // 現在鳴っている音がある場合、それを停止
    if (_currentNoteIndex != -1) {
        _voices[_currentNoteIndex].noteOff(); // 音を停止
        _isReleasing = true; // リリース中に設定
        _noteOffStartTime = millis(); // リリース開始時間を記録
    }
}

uint16_t misuzu::getNextSample() {
    if (_currentNoteIndex != -1) {
        uint16_t sample = _voices[_currentNoteIndex].getNextSample();
        // リリース中であれば、hatayaがisIdle()になったら_currentNoteIndexをリセット
        // (misuzu::handleStopCommandで_isReleasingがtrueになった後に、このチェックを入れる)
        // _isReleasingがtrueの場合、_voices[_currentNoteIndex].isIdle()で確認
        if (_isReleasing && _voices[_currentNoteIndex].isIdle()) {
           _currentNoteIndex = -1;
           _isReleasing = false;
           _noteOffStartTime = 0;
        }
        return sample;
    }
    return MAX_AMPLITUDE; // 音が鳴っていないときは無音（DACの中心値）を返す
}