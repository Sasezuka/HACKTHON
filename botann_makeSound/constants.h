// --- constants.h ---
#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <Arduino.h> // byte, uint16_t などの型定義のために必要

// --- 1. オーディオ関連の共通設定 ---

// サンプリングレート (Hz)
#define SAMPLE_RATE 16000
#define MAX_AMPLITUDE 2047 

// --- 2. 音源関連の共通設定 (Arduino 2 で主に使用) ---
// 各ノートの基本周波数 (Hz)
const float NOTE_FREQUENCIES[] = {
    261.63, // C4
    293.66, // D4
    329.63, // E4
    349.23, // F4
    392.00, // G4
    440.00, // A4
    493.88, // B4
    523.25  // C5
};
// ノート数 (配列のサイズから自動計算)
const int NUM_NOTES = sizeof(NOTE_FREQUENCIES) / sizeof(NOTE_FREQUENCIES[0]);

// 基本となる音量 (hatayaクラスで使用)
#define BASE_AMPLITUDE 0.5f // 0.0f - 1.0f の範囲で調整

// ハープの倍音構成
const float HARMONICS[] = {
    1.0f,   // 1次倍音（基音）
    0.048f,   // 2次倍音
    0.039f,   // 3次倍音
    0.036,   // 4次倍音
    0.007f,  // 5次倍音
    0.006f,  // 6次倍音
    0.006f,  // 7次倍音
    0.008f,  // 8次倍音
    0.001f   // 9次倍音
};

const int NUM_HARMONICS = sizeof(HARMONICS) / sizeof(HARMONICS[0]);

// --- 3. エフェクト関連の共通設定 (Arduino 3 で主に使用) ---
// ディレイバッファの長さ (ミリ秒)
// これはディレイタイムの目安。SAMPLE_RATE と合わせて DELAY_BUFFER_SIZE を決定
#define DELAY_BUFFER_LENGTH_MS 200 // 200ミリ秒のディレイ

// ディレイバッファのサイズ (サンプル数)
// SAMPLE_RATE が 18000Hz なら、200ms = 0.2s * 18000 = 3600サンプル
#define DELAY_BUFFER_SIZE (uint32_t)(SAMPLE_RATE * (DELAY_BUFFER_LENGTH_MS / 1000.0f))

// ディレイのフィードバック量 (0.0f - 1.0f)
#define DELAY_FEEDBACK 0.7f

// ドライ/ウェットミックス比 (0.0f - 1.0f)
#define DRY_MIX 0.4f // 原音の割合
#define WET_MIX 0.6f // エフェクト音の割合

// --- 4. その他の共通設定 ---
// Serial通信のボーレート (Arduino 1とArduino 2で共通)
#define SERIAL_BAUDRATE 115200 // ★★★ 修正点 ★★★

// シリアル受信バッファの最大サイズ (Arduino 2で利用)
#define MAX_RECEIVE_BUFFER_SIZE 64 

#endif