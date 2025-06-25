#include "hataya.h"

// --- コンストラクタ ---
hataya::hataya() :
    _frequency(0.0f), _amplitude(0.0f), _sampleRate(0),
    _phaseAccumulator(0.0f), _currentStage(IDLE), _currentLevel(0.0f),
    _startLevel(0.0f), _endLevel(0.0f), _stageStartTimeMillis(0)
{
    // 波形バッファをゼロクリア
    for (int i = 0; i < WAVEFORM_BUFFER_SIZE; ++i) _waveform[i] = 0;
}

// --- 初期化 ---
void hataya::init(float freq, float amplitude, const float* harmonics, int numHarmonics, uint32_t sampleRate) {
    _frequency = freq;
    _amplitude = amplitude;
    _sampleRate = sampleRate;
    _phaseAccumulator = 0.0f;

    // 波形を1周期分だけ事前計算してテーブルに格納
    for (int i = 0; i < WAVEFORM_BUFFER_SIZE; ++i) {
        float sampleValue = 0.0f;
        float t = (float)i / WAVEFORM_BUFFER_SIZE; // 0.0 ~ 1.0 の正規化時間
        for (int h = 0; h < numHarmonics; ++h) {
            // 加算合成
            sampleValue += sin(2.0f * PI * (h + 1) * t) * harmonics[h];
        }
        // DACの中心値を基準にスケーリング (0 ~ 4095)
        _waveform[i] = (sample_t)(sampleValue * MAX_AMPLITUDE + MAX_AMPLITUDE);
    }
}

// --- 次のサンプルを取得 ---
sample_t hataya::getNextSample() {
    float envelopeValue = calculateEnvelope();
    if (_currentStage == IDLE) return MAX_AMPLITUDE; // 無音時は中心値

    // 位相を進める
    _phaseAccumulator += _frequency / _sampleRate;
    if (_phaseAccumulator >= 1.0f) _phaseAccumulator -= 1.0f;

    // 波形テーブルからサンプル値を取得
    int index = (int)(_phaseAccumulator * WAVEFORM_BUFFER_SIZE);
    float rawSample = (float)_waveform[index] - MAX_AMPLITUDE;

    // エンベロープと振幅を適用
    float finalSampleFloat = rawSample * _amplitude * envelopeValue;

    // DACの出力範囲 (0-4095) に戻す
    finalSampleFloat += MAX_AMPLITUDE;

    // クリッピング
    if (finalSampleFloat < 0.0f) finalSampleFloat = 0.0f;
    else if (finalSampleFloat > 4095.0f) finalSampleFloat = 4095.0f;

    return (sample_t)finalSampleFloat;
}

// --- ノートオン ---
void hataya::noteOn() {
    _currentStage = ATTACK;
    _currentLevel = 0.0f;
    _startLevel = 0.0f;
    _endLevel = 1.0f;
    _stageStartTimeMillis = millis();
}

// --- ノートオフ ---
void hataya::noteOff() {
    if (_currentStage != IDLE) {
        _currentStage = RELEASE;
        _startLevel = _currentLevel; // 現在のレベルから減衰開始
        _endLevel = 0.0f;
        _stageStartTimeMillis = millis();
    }
}

// --- アイドル状態か確認 ---
bool hataya::isIdle() {
    return _currentStage == IDLE;
}

// --- エンベロープ計算 ---
float hataya::calculateEnvelope() {
    unsigned long elapsedTime = millis() - _stageStartTimeMillis;
    float level = _currentLevel;

    switch (_currentStage) {
        case IDLE:
            level = 0.0f;
            break;
        case ATTACK:
            if (elapsedTime < ATTACK_DURATION_MS) {
                level = _startLevel + (_endLevel - _startLevel) * ((float)elapsedTime / ATTACK_DURATION_MS);
            } else {
                level = _endLevel;
                _currentStage = DECAY;
                _startLevel = _endLevel;
                _endLevel = SUSTAIN_LEVEL_VAL;
                _stageStartTimeMillis = millis();
            }
            break;
        case DECAY:
            if (elapsedTime < DECAY_DURATION_MS) {
                level = _startLevel + (_endLevel - _startLevel) * ((float)elapsedTime / DECAY_DURATION_MS);
            } else {
                level = _endLevel;
                _currentStage = SUSTAIN;
            }
            break;
        case SUSTAIN:
            level = SUSTAIN_LEVEL_VAL;
            break;
        case RELEASE:
            if (elapsedTime < RELEASE_DURATION_MS) {
                level = _startLevel - _startLevel * ((float)elapsedTime / RELEASE_DURATION_MS);
            } else {
                level = 0.0f;
                _currentStage = IDLE;
            }
            break;
    }
    _currentLevel = level;
    return _currentLevel;
}
