#include "temari.h"
#include <math.h> // sin()関数のために必要

// コンストラクタ
Temari::Temari() :
    _sampleRate(0),
    _dacPin(0),
    _vibratoDepth(0.0f),
    _vibratoRate(DEFAULT_VIBRATO_RATE),
    _vibratoPhase(0.0f),
    _reverbWritePos(0),
    _receiveReadPtr(0),
    _receiveWritePtr(0)
{
    // リバーブバッファの初期化
    for (int i = 0; i < REVERB_MAX_DELAY_SAMPLES; ++i) {
        _reverbBuffer[i] = 0;
    }

    // コムフィルターの初期設定（適当な遅延時間）
    // 異なる遅延時間とフィードバックを設定することで豊かなリバーブに
    // これらは init() 後に _sampleRate を使って再計算されるべきだが、
    // コンストラクタではデフォルト値で仮初期化
    _combFilters[0] = {100, REVERB_FEEDBACK_GAIN, 0}; // 適当な初期値
    _combFilters[1] = {150, REVERB_FEEDBACK_GAIN * 0.9f, 0};
    _combFilters[2] = {200, REVERB_FEEDBACK_GAIN * 0.8f, 0};
    _combFilters[3] = {250, REVERB_FEEDBACK_GAIN * 0.7f, 0};
}

// エフェクトの初期化
void Temari::init(uint32_t sampleRate, byte dacPin) {
    _sampleRate = sampleRate;
    _dacPin = dacPin;
    pinMode(_dacPin, OUTPUT); // DACピンを出力に設定

    // コムフィルターの遅延サンプル数を、実際のサンプルレートに基づいて計算し直す
    _combFilters[0].delaySamples = calculateDelaySamples(50); // 50ms
    _combFilters[1].delaySamples = calculateDelaySamples(75); // 75ms
    _combFilters[2].delaySamples = calculateDelaySamples(100); // 100ms
    _combFilters[3].delaySamples = calculateDelaySamples(125); // 125ms
    
    // 各フィルターの読み込み位置を初期化
    for(int i = 0; i < NUM_COMB_FILTERS; ++i) {
        _combFilters[i].readPos = 0; 
    }
    
    // R4 MinimaのDACを有効化 (DAC0ピンを使う場合)
    analogWriteResolution(12);

    Serial.println("Temari (Effect) initialized with DAC.");
}

// SPI受信割り込みハンドラから呼び出される関数
// 受信したオーディオサンプルを内部リングバッファに追加
void Temari::addReceivedSample(uint16_t sample) {
    // バッファオーバーフローチェック
    uint16_t nextWritePos = (_receiveWritePtr + 1) % RECEIVE_BUFFER_SIZE;
    if (nextWritePos == _receiveReadPtr) {
        // バッファオーバーフロー！データが失われる可能性があります。
        // エラー処理や、マスター側への信号を送るなどを検討
        return; 
    }
    _receiveBuffer[_receiveWritePtr] = sample;
    _receiveWritePtr = nextWritePos;
}

// メインループで呼び出し、エフェクトを適用してDACに出力する関数
void Temari::processAndOutput() {
    // 受信バッファに処理すべきサンプルがあるか確認
    if (_receiveReadPtr == _receiveWritePtr) {
        // バッファが空なら何もしない
        return;
    }

    // 1. 受信バッファからサンプルを読み出す
    // SPIで受信する12bitデータは、0-4095の範囲と仮定
    uint16_t rawInputSample = _receiveBuffer[_receiveReadPtr];
    _receiveReadPtr = (_receiveReadPtr + 1) % RECEIVE_BUFFER_SIZE;

    // 入力サンプルを-1.0fから1.0fの浮動小数点数に正規化
    // MAX_AMPLITUDE (2047) が中心値となるように調整
    float inputSample = (static_cast<float>(rawInputSample) - MAX_AMPLITUDE) / MAX_AMPLITUDE;

    // --- 2. リバーブの適用 ---
    float wetSample = 0.0f; // リバーブ成分

    // 各コムフィルターを処理
    for (int i = 0; i < NUM_COMB_FILTERS; ++i) {
        CombFilter& cf = _combFilters[i];

        // 過去のディレイサンプルを読み出す
        // _reverbBuffer は int16_t なので、-32768〜32767の範囲
        // これを -1.0f〜1.0f に正規化
        float delayedSample = static_cast<float>(_reverbBuffer[cf.readPos]) / 32767.0f; 

        // ディレイバッファに現在の入力サンプル + フィードバックを書き込む
        // inputSample + (delayedSample * cf.feedback) がコムフィルターの入力
        // 書き込む前に int16_t の範囲に再度スケーリング
        int16_t valueToBuffer = static_cast<int16_t>((inputSample + (delayedSample * cf.feedback)) * 32767.0f);
        
        // 書き込み位置は共通の_reverbWritePosを使用
        // これはバッファ全体への書き込み位置
        // cf.readPosは各コムフィルター固有の読み込み位置
        _reverbBuffer[_reverbWritePos] = valueToBuffer;

        // 次の読み込み位置を計算 (ディレイ量に基づく)
        // リングバッファの読み込み位置を更新
        cf.readPos = (_reverbWritePos + REVERB_MAX_DELAY_SAMPLES - cf.delaySamples) % REVERB_MAX_DELAY_SAMPLES;

        wetSample += delayedSample; // 各コムフィルターの出力を合計
    }

    // リバーブバッファの書き込み位置を進める
    _reverbWritePos = (_reverbWritePos + 1) % REVERB_MAX_DELAY_SAMPLES;

    // --- 3. ドライ/ウェットミックス ---
    // constants.h で DRY_MIX と WET_MIX を定義しておく
    float outputSample = inputSample * DRY_MIX + wetSample * WET_MIX;

    // 出力サンプルのクリッピング (オーバーフロー防止)
    if (outputSample > 1.0f) outputSample = 1.0f;
    if (outputSample < -1.0f) outputSample = -1.0f;

    // --- 4. DACに出力 ---
    uint16_t dacValue = convertToDacValue(outputSample);
    analogWrite(_dacPin, dacValue);
}

// 距離センサー値を受け取り、ビブラートの深さを設定
// ★この関数は、もしArduino 3でビブラートをかける場合にのみ意味があります。
//   Arduino 2でビブラートをかける場合は、この関数は使用されません。
void Temari::setVibratoDepth(byte sensorValue) {
    _vibratoDepth = static_cast<float>(sensorValue) * SENSOR_TO_DEPTH_MAP_FACTOR;
}


// --- ユーティリティ関数 ---
uint16_t Temari::calculateDelaySamples(float ms) {
    return (uint16_t)((ms / 1000.0f) * _sampleRate);
}

// サンプルをDAC出力用に変換
// -1.0f ～ 1.0f の浮動小数点数を 0 ～ 4095 (12bit DAC) にマッピング
uint16_t Temari::convertToDacValue(float sample) {
    // 0.0fをMAX_AMPLITUDE（中心値）、1.0fをMAX_AMPLITUDE*2、-1.0fを0としてマッピング
    return static_cast<uint16_t>(roundf((sample + 1.0f) * MAX_AMPLITUDE));
}