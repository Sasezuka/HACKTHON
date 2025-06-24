//エンベロープ関連の宣言

#ifndef KAYA_H // ヘッダガードを修正
#define KAYA_H

#include <Arduino.h> 

const int WAVEFORM_BUFFER_SIZE = 512; 
const uint16_t MAX_AMPLITUDE = 2047;

typedef uint16_t sample_t;

// クラス名をhatayaからkayaに変更
class kaya {
public:
    kaya(); // コンストラクタ

    // 音源の初期化（周波数設定と波形生成）
    // freq: 音の周波数, amplitude: 基本振幅, harmonics: 倍音配列, numHarmonics: 倍音数, sampleRate: サンプルレート
    void init(float freq, float amplitude, const float* harmonics, int numHarmonics, uint32_t sampleRate);

    // 次のサンプル値を取得（ADSRエンベロープ適用済み）
    sample_t getNextSample();

    void noteOn();  // 音を鳴らし始める
    void noteOff(); // 音を鳴らすのをやめる（減衰開始）
    bool isIdle();  // 音が完全に停止しているかチェック

private:
    float _frequency;             // 音の周波数
    float _amplitude;             // 基本振幅
    sample_t _waveform[WAVEFORM_BUFFER_SIZE]; // 波形データバッファ
    uint32_t _sampleRate;         // サンプルレート
    float _phaseAccumulator;      // 現在の波形再生位置（小数点以下を含むフェーズ）

    // エンベロープ（ADSR）関連の変数
    enum AdsrStage {
        IDLE,    // 無音
        ATTACK,  // 立ち上がり
        DECAY,   // 減衰
        SUSTAIN, // 持続
        RELEASE  // 余韻
    };
    AdsrStage _currentStage;      // 現在のADSRステージ
    float _currentLevel;          // 現在の音量レベル (0.0f～1.0f)
    float _startLevel;            // 現在のステージ開始時の音量レベル
    float _endLevel;              // 現在のステージ終了時の目標音量レベル
    unsigned long _stageStartTimeMillis; // 現在のステージが開始した時刻 (millis()値)

    // ADSRフェーズの期間（ミリ秒単位で設定）
    const uint32_t ATTACK_DURATION_MS = 5;   
    const uint32_t DECAY_DURATION_MS  = 300;   
    const float    SUSTAIN_LEVEL_VAL  = 0.03f; 
    const uint32_t RELEASE_DURATION_MS= 600;  

    // 現在のADSRエンベロープ値を計算する関数
    float calculateEnvelope();
};

#endif // KAYA_H