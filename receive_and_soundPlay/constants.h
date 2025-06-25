#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <Arduino.h> // uint16_tなどの型定義のため

// --- オーディオ関連 ---
#define SAMPLE_RATE 16000
#define MAX_AMPLITUDE 2047 // 12bit DAC (0-4095) の中心値

// --- 通信関連 ---
#define SERIAL_BAUDRATE 115200
#define MAX_RECEIVE_BUFFER_SIZE 64

// --- 音源パラメータ ---
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
const int NUM_NOTES = sizeof(NOTE_FREQUENCIES) / sizeof(NOTE_FREQUENCIES[0]);

#define BASE_AMPLITUDE 0.8f // 基本振幅 (0.0f - 1.0f)

const float HARMONICS[] = {
    1.0f,   // 1次倍音（基音）
    0.048f, // 2次倍音
    0.039f, // 3次倍音
    0.036f, // 4次倍音
    0.007f, // 5次倍音
    0.006f, // 6次倍音
    0.006f, // 7次倍音
    0.008f, // 8次倍音
    0.001f  // 9次倍音
};
const int NUM_HARMONICS = sizeof(HARMONICS) / sizeof(HARMONICS[0]);

#endif
