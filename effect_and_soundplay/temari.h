#ifndef TEMARI_H
#define TEMARI_H

#include <Arduino.h>
#include <SPI.h> // SPI通信のために必要
#include "constants.h" // SAMPLE_RATE, MAX_AMPLITUDE などの共通定数

// リバーブのパラメータ (調整可能)
#define REVERB_MAX_DELAY_SAMPLES 4000 // 最大ディレイサンプル数（リバーブの長さに影響）
#define REVERB_FEEDBACK_GAIN     0.7f // リバーブのフィードバック量 (0.0f-1.0f)

class Temari {
public:
    Temari(); // コンストラクタ

    // エフェクトの初期化
    // DAC出力ピンやサンプリングレートなどを設定
    void init(uint32_t sampleRate, byte dacPin);

    // SPI受信割り込みハンドラから呼び出される関数
    // 受信したオーディオサンプルを内部バッファに追加
    void addReceivedSample(uint16_t sample); // 12bitデータならuint16_t

    // メインループで呼び出し、エフェクトを適用してDACに出力する関数
    void processAndOutput();

    // ★重要: このメソッドは、もしArduino 3でビブラートをかける場合にのみ使用します。
    //   現状の提案ではArduino 2でビブラートをかけるため、直接は使用しません。
    void setVibratoDepth(byte sensorValue); 

private:
    uint32_t _sampleRate;
    byte     _dacPin;

    // --- ビブラート関連（Arduino 3で使わない前提だが、残しておく） ---
    float _vibratoDepth;
    float _vibratoRate;
    float _vibratoPhase;
    static constexpr float DEFAULT_VIBRATO_RATE = 5.0f; // 5 Hz
    static constexpr float MAX_VIBRATO_DEPTH_FACTOR = 0.05f; // 周波数の±5%
    static constexpr float SENSOR_TO_DEPTH_MAP_FACTOR = MAX_VIBRATO_DEPTH_FACTOR / 255.0f;

    // --- リバーブ関連 ---
    // リバーブ用ディレイバッファ (16bit符号付き整数で扱うとメモリ効率が良い)
    int16_t _reverbBuffer[REVERB_MAX_DELAY_SAMPLES]; 
    uint16_t _reverbWritePos; // バッファ書き込み位置
    
    // 簡単なリバーブ（複数のコムフィルターやオールパスフィルターを模倣）
    struct CombFilter {
        uint16_t delaySamples; // 遅延サンプル数
        float    feedback;     // フィードバック量
        uint16_t readPos;      // 読み込み位置
    };
    static constexpr int NUM_COMB_FILTERS = 4; // フィルターの数
    CombFilter _combFilters[NUM_COMB_FILTERS];

    // サンプリングレートに応じたディレイサンプル数の計算ユーティリティ
    uint16_t calculateDelaySamples(float ms);

    // --- 受信バッファ関連 ---
    // Arduino 2から受信したオーディオサンプルを一時的に保持するバッファ
    static constexpr int RECEIVE_BUFFER_SIZE = 128; // SPIブロックサイズより大きく
    volatile uint16_t _receiveBuffer[RECEIVE_BUFFER_SIZE];
    volatile uint16_t _receiveReadPtr;
    volatile uint16_t _receiveWritePtr;

    // --- DAC出力関連 ---
    // サンプルをDAC用に変換
    uint16_t convertToDacValue(float sample);
};

#endif