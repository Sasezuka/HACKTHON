#ifndef MAKESOUND_H
#define MAKESOUND_H

#include <Arduino.h>
#include "constants.h"

const int WAVEFORM_BUFFER_SIZE = 512; // 波形テーブルのサイズ
typedef uint16_t sample_t;            // サンプル値の型

class makeSound {
public:
    makeSound();
    void init(float freq, float amplitude, const float* harmonics, int numHarmonics, uint32_t sampleRate);
    sample_t getNextSample();
    void noteOn();
    void noteOff();
    bool isIdle();

private:
    // 音声合成パラメータ
    float _frequency;
    float _amplitude;
    sample_t _waveform[WAVEFORM_BUFFER_SIZE];
    uint32_t _sampleRate;
    float _phaseAccumulator;

    // ADSRエンベロープ関連
    enum AdsrStage { IDLE, ATTACK, DECAY, SUSTAIN, RELEASE };
    AdsrStage _currentStage;
    float _currentLevel;
    float _startLevel;
    float _endLevel;
    unsigned long _stageStartTimeMillis;

    // ADSR設定値 (ミリ秒)
    const uint32_t ATTACK_DURATION_MS = 10;
    const uint32_t DECAY_DURATION_MS  = 300;
    const float    SUSTAIN_LEVEL_VAL  = 0.5f;
    const uint32_t RELEASE_DURATION_MS= 800;

    float calculateEnvelope();
};

#endif
