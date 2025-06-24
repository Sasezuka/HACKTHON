#include "temari.h"
#include <math.h> // sin()関数のために必要

// コンストラクタ
Temari::Temari() :
    _sampleRate(0),
    _dacPin(0)
    // 受信バッファ関連、リバーブ関連の初期化は今回使わないため削除またはコメントアウト
    // _vibratoDepth(0.0f),
    // _vibratoRate(DEFAULT_VIBRATO_RATE),
    // _vibratoPhase(0.0f),
    // _reverbWritePos(0),
    // _receiveReadPtr(0),
    // _receiveWritePtr(0)
{
    // リバーブバッファの初期化は今回使わないため削除またはコメントアウト
    // for (int i = 0; i < REVERB_MAX_DELAY_SAMPLES; ++i) {
    //     _reverbBuffer[i] = 0;
    // }

    // コムフィルターの初期設定は今回使わないため削除またはコメントアウト
    // _combFilters[0] = {100, REVERB_FEEDBACK_GAIN, 0};
    // _combFilters[1] = {150, REVERB_FEEDBACK_GAIN * 0.9f, 0};
    // _combFilters[2] = {200, REVERB_FEEDBACK_GAIN * 0.8f, 0};
    // _combFilters[3] = {250, REVERB_FEEDBACK_GAIN * 0.7f, 0};
}

// エフェクトの初期化
void Temari::init(uint32_t sampleRate, byte dacPin) {
    _sampleRate = sampleRate;
    _dacPin = dacPin;
    pinMode(_dacPin, OUTPUT); // DACピンを出力に設定

    // コムフィルターの遅延サンプル数の計算は今回使わないため削除またはコメントアウト
    // _combFilters[0].delaySamples = calculateDelaySamples(50);
    // _combFilters[1].delaySamples = calculateDelaySamples(75);
    // _combFilters[2].delaySamples = calculateDelaySamples(100);
    // _combFilters[3].delaySamples = calculateDelaySamples(125);

    // 各フィルターの読み込み位置の初期化は今回使わないため削除またはコメントアウト
    // for(int i = 0; i < NUM_COMB_FILTERS; ++i) {
    //     _combFilters[i].readPos = 0;
    // }

    // R4 MinimaのDACを有効化 (DAC0ピンを使う場合)
    analogWriteResolution(12); // 12bit DACを使用することを設定

    Serial.println("Temari (Effect) initialized with DAC.");
}

// SPI受信割り込みハンドラから呼び出される関数
// 受信したオーディオサンプルを内部リングバッファに追加
// 今回はmain.inoのreceiveEventで直接処理するため、この関数は未使用または簡略化
void Temari::addReceivedSample(uint16_t sample) {
    // 現状は未使用
}

// メインループで呼び出し、エフェクトを適用してDACに出力する関数
// 今回はmain.inoのloop()で直接DAC出力するため、この関数は未使用または簡略化
void Temari::processAndOutput() {
    // 現状はDAC出力のパススルーをmain.inoのloop()で直接行います。
    // 将来的にリバーブ処理をTemariクラスに戻す際にここを実装します。
}

// 距離センサー値を受け取り、ビブラートの深さを設定 (今回は未使用)
void Temari::setVibratoDepth(byte sensorValue) {
    // 現状は未使用
}


// --- ユーティリティ関数 ---
// calculateDelaySamplesはリバーブで使うため、今回は未使用だが残しておく
uint16_t Temari::calculateDelaySamples(float ms) {
    return (uint16_t)((ms / 1000.0f) * _sampleRate);
}

// サンプルをDAC出力用に変換 (これはDAC出力で直接使うので残す)
// -1.0f ～ 1.0f の浮動小数点数を 0 ～ 4095 (12bit DAC) にマッピング
uint16_t Temari::convertToDacValue(float sample) {
    // 0.0fをMAX_AMPLITUDE（中心値）、1.0fをMAX_AMPLITUDE*2、-1.0fを0としてマッピング
    return static_cast<uint16_t>(roundf((sample + 1.0f) * MAX_AMPLITUDE));
}