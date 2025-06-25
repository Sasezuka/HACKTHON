#include "misuzu.h"
#include "constants.h"

// --- コンストラクタ ---
misuzu::misuzu() :
    _currentNoteIndex(-1), _isReleasing(false)
{
    // 全てのボイスを初期化
    for (int i = 0; i < NUM_NOTES; ++i) {
        _voices[i].init(NOTE_FREQUENCIES[i], BASE_AMPLITUDE, HARMONICS, NUM_HARMONICS, SAMPLE_RATE);
    }
}

// --- 再生コマンド処理 ---
void misuzu::handlePlayCommand(int noteIndex) {
    if (noteIndex < 0 || noteIndex >= NUM_NOTES) return;
    
    // 別の音が鳴っていれば、それを止める（レガートのような挙動）
    if (_currentNoteIndex != -1 && _currentNoteIndex != noteIndex) {
        _voices[_currentNoteIndex].noteOff();
    }
    
    // 新しい音を再生
    _currentNoteIndex = noteIndex;
    _voices[_currentNoteIndex].noteOn();
    _isReleasing = false;
}

// --- 停止コマンド処理 ---
void misuzu::handleStopCommand() {
    if (_currentNoteIndex != -1) {
        _voices[_currentNoteIndex].noteOff();
        _isReleasing = true;
    }
}

// --- 次のサンプルを取得 ---
uint16_t misuzu::getNextSample() {
    if (_currentNoteIndex != -1) {
        // 現在の音のサンプル値を取得
        uint16_t sample = _voices[_currentNoteIndex].getNextSample();
        
        // リリース中に音が完全に消えたかチェック
        if (_isReleasing && _voices[_currentNoteIndex].isIdle()) {
           _currentNoteIndex = -1; // 音が消えたのでインデックスをリセット
           _isReleasing = false;
        }
        return sample;
    }
    // 何も鳴っていないときはDACの中心値を返す
    return MAX_AMPLITUDE;
}
