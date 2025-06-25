#include "kaya.h"      // インクルードファイルを修正
#include "constants.h" // constants.h をインクルードして、共通定数にアクセスできるようにする
#include <math.h>      // sin関数を使用するため追加 (hataya.cppには元々無かったが、必要と判断)

#ifndef PI // PIが未定義の場合、定義する
#define PI 3.14159265358979323846f
#endif

// コンストラクタ
// クラス名をhatayaからkayaに変更
kaya::kaya() :
    _frequency(0.0f),
    _amplitude(0.0f),
    _sampleRate(0),
    _phaseAccumulator(0.0f),
    _currentStage(IDLE),
    _currentLevel(0.0f),
    _startLevel(0.0f),
    _endLevel(0.0f),
    _stageStartTimeMillis(0)
{
    // 波形バッファを0で初期化
    for (int i = 0; i < WAVEFORM_BUFFER_SIZE; ++i) {
        _waveform[i] = 0;
    }
}

// 音源の初期化と波形生成
// クラス名をhatayaからkayaに変更
void kaya::init(float freq, float amplitude, const float* harmonics, int numHarmonics, uint32_t sampleRate) {
    _frequency = freq;
    _amplitude = amplitude;
    _sampleRate = sampleRate;
    _phaseAccumulator = 0.0f; // 初期化時に位相をリセット

    // 波形生成（加算合成）
    // バッファに波形データを事前計算して格納
    for (int i = 0; i < WAVEFORM_BUFFER_SIZE; ++i) {
        float sampleValue = 0.0f;
        float t = (float)i / WAVEFORM_BUFFER_SIZE; // 0.0f から 1.0f までの正規化された時間

        for (int h = 0; h < numHarmonics; ++h) {
            // harmonics配列には倍音の「重み」が格納されていると仮定
            // ここでは単純な整数倍音 (基音, 2倍音, 3倍音...) に重みを適用
            sampleValue += sin(2.0f * PI * (h + 1) * t) * harmonics[h];
        }

        // -1.0f から 1.0f の範囲に正規化された波形を、0 から MAX_AMPLITUDE*2 の範囲にスケーリング
        // MAX_AMPLITUDE を中心に配置 (例: 2047)
        _waveform[i] = (sample_t)(sampleValue * MAX_AMPLITUDE + MAX_AMPLITUDE);
    }
    // ここでエンベロープの状態はリセットしない。noteOn()が呼ばれたときにリセットされる
}

// 次のサンプル値を取得（ADSRエンベロープ適用済み）
// クラス名をhatayaからkayaに変更
sample_t kaya::getNextSample() {
    // 1. エンベロープ値を計算
    float envelopeValue = calculateEnvelope();

    // 2. 波形サンプルを取得
    // _phaseAccumulator は波形のどの位置からサンプルを取るかを示す
    // 0.0f - 1.0f の範囲で正規化された位相
    _phaseAccumulator += _frequency / _sampleRate; // サンプリングレートごとに位相を進める
    if (_phaseAccumulator >= 1.0f) {
        _phaseAccumulator -= 1.0f; // 1.0fを超えたらラップアラウンド
    }
    
    // _waveform配列のインデックスを計算
    int index = (int)(_phaseAccumulator * WAVEFORM_BUFFER_SIZE);
    
    // 配列の範囲チェック（念のため）
    if (index >= WAVEFORM_BUFFER_SIZE) {
        index = WAVEFORM_BUFFER_SIZE - 1;
    } else if (index < 0) {
        index = 0;
    }

    // _waveform[index] は既に 0〜4095 の範囲にスケーリングされているため、
    // 一度中心値0の範囲 (-MAX_AMPLITUDE から MAX_AMPLITUDE) に戻す
    float rawSample = (float)_waveform[index] - MAX_AMPLITUDE;

    // 3. 振幅とエンベロープを適用
    float finalSampleFloat = rawSample * _amplitude * envelopeValue;

    // 4. DAC出力範囲に再スケーリングし、クリッピング
    finalSampleFloat += MAX_AMPLITUDE; // 0〜4095 の範囲に戻す

    if (finalSampleFloat < 0.0f) {
        finalSampleFloat = 0.0f;
    } else if (finalSampleFloat > 4095.0f) { // 12bit DACの最大値
        finalSampleFloat = 4095.0f;
    }

    // 5. 音量が0に近い場合、ステージをIDLEにする
    // これは isIdle() に任せるか、releaseステージ終了時に行う
    if (_currentStage == RELEASE && envelopeValue < 0.001f) { // ほぼ無音になったら
         _currentStage = IDLE;
         _currentLevel = 0.0f;
    }

    return (sample_t)finalSampleFloat;
}

// 音を鳴らし始める
// クラス名をhatayaからkayaに変更
void kaya::noteOn() {
    _currentStage = ATTACK;
    _currentLevel = 0.0f;        // アタック開始レベル
    _startLevel = 0.0f;
    _endLevel = 1.0f;            // アタック目標レベル
    _stageStartTimeMillis = millis();
}

// 音を鳴らすのをやめる（減衰開始）
// クラス名をhatayaからkayaに変更
void kaya::noteOff() {
    if (_currentStage != IDLE && _currentStage != RELEASE) {
        _currentStage = RELEASE;
        _startLevel = _currentLevel; // 現在の音量レベルからリリース開始
        _endLevel = 0.0f;            // リリース目標レベル
        _stageStartTimeMillis = millis();
    }
}

// 音が完全に停止しているかチェック
// クラス名をhatayaからkayaに変更
bool kaya::isIdle() {
    return _currentStage == IDLE;
}

// 現在のADSRエンベロープ値を計算する関数
// クラス名をhatayaからkayaに変更
float kaya::calculateEnvelope() {
    unsigned long elapsedTime = millis() - _stageStartTimeMillis;
    float level = 0.0f;

    switch (_currentStage) {
        case IDLE:
            level = 0.0f;
            break;

        case ATTACK:
            if (elapsedTime < ATTACK_DURATION_MS) {
                // 線形補間
                level = _startLevel + (_endLevel - _startLevel) * ((float)elapsedTime / ATTACK_DURATION_MS);
            } else {
                level = _endLevel;
                _currentStage = DECAY; // 次のステージへ
                _startLevel = _endLevel; // ディケイ開始レベル
                _endLevel = SUSTAIN_LEVEL_VAL; // ディケイ目標レベル
                _stageStartTimeMillis = millis(); // ステージ開始時間更新
            }
            break;

        case DECAY:
            if (elapsedTime < DECAY_DURATION_MS) {
                // 線形補間
                level = _startLevel + (_endLevel - _startLevel) * ((float)elapsedTime / DECAY_DURATION_MS);
            } else {
                level = _endLevel;
                _currentStage = SUSTAIN; // 次のステージへ
                // サスティンは時間の経過でレベルが変わらないため、startLevel/endLevelは不要だが、currentLevelを維持
                _currentLevel = SUSTAIN_LEVEL_VAL;
                // _stageStartTimeMillis はサスティンステージに滞在し続ける間は更新不要
            }
            break;

        case SUSTAIN:
            level = SUSTAIN_LEVEL_VAL; // サスティンレベルを維持
            break;

        case RELEASE:
            if (elapsedTime < RELEASE_DURATION_MS) {
                // 線形補間
                level = _startLevel + (_endLevel - _startLevel) * ((float)elapsedTime / RELEASE_DURATION_MS);
                // レベルがマイナスにならないようにクリップ
                if (level < 0.0f) level = 0.0f;
            } else {
                level = _endLevel;
                _currentStage = IDLE; // 次のステージへ（無音）
                level = 0.0f;
            }
            break;
    }
    _currentLevel = level; // 現在のレベルを更新
    return _currentLevel;
}