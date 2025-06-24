#ifndef TEMARI_H
#define TEMARI_H

#include <Arduino.h>
// #include <SPI.h> // SPI通信はI2Cに変わるため不要
#include "constants.h" // SAMPLE_RATE, MAX_AMPLITUDE などの共通定数

// リバーブのパラメータ (調整可能) - 今回は未使用だが定義は残しておく
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
    // 今回はmain.inoのreceiveEventで直接処理するため、この関数は現状は不要
    void addReceivedSample(uint16_t sample); // 12bitデータならuint16_t

    // メインループで呼び出し、エフェクトを適用してDACに出力する関数
    // 今回はmain.inoのloop()で直接DAC出力するため、この関数は現状は不要
    void processAndOutput();

    // 距離センサー値を受け取り、ビブラートの深さを設定 (今回は未使用)
    void setVibratoDepth(byte sensorValue);

private:
    uint32_t _sampleRate;
    byte     _dacPin;

    // --- ビブラート関連（今回は未使用だが、定義は残す） ---
    float _vibratoDepth;
    float _vibratoRate;
    float _vibratoPhase;
    static constexpr float DEFAULT_VIBRATO_RATE = 5.0f; // 5 Hz
    static constexpr float MAX_VIBRATO_DEPTH_FACTOR = 0.05f; // 周波数の±5%
    static constexpr float SENSOR_TO_DEPTH_MAP_FACTOR = MAX_VIBRATO_DEPTH_FACTOR / 255.0f;

    // --- リバーブ関連（今回は未使用だが、定義は残す） ---
    // int16_t _reverbBuffer[REVERB_MAX_DELAY_SAMPLES];
    // uint16_t _reverbWritePos;

    // struct CombFilter {
    //     uint16_t delaySamples;
    //     float    feedback;
    //     uint16_t readPos;
    // };
    // static constexpr int NUM_COMB_FILTERS = 4;
    // CombFilter _combFilters[NUM_COMB_FILTERS];

    // サンプリングレートに応じたディレイサンプル数の計算ユーティリティ (今回は未使用だが残す)
    uint16_t calculateDelaySamples(float ms);

    // --- 受信バッファ関連（今回はmain.inoで直接管理するためTemariからは削除） ---
    // volatile uint16_t _receiveBuffer[RECEIVE_BUFFER_SIZE];
    // volatile uint16_t _receiveReadPtr;
    // volatile uint16_t _receiveWritePtr;

    // --- DAC出力関連 ---
    // サンプルをDAC用に変換 (これはDAC出力で直接使うので残す)
    uint16_t convertToDacValue(float sample);
};

#endif // TEMARI_H