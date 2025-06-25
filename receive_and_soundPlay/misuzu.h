#ifndef MISUZU_H
#define MISUZU_H

#include "hataya.h"
#include "constants.h"

class misuzu {
public:
    misuzu();
    void handlePlayCommand(int noteIndex);
    void handleStopCommand();
    uint16_t getNextSample();

private:
    hataya _voices[NUM_NOTES]; // 各ノートに対応する音源
    int _currentNoteIndex;      // 現在鳴っている音のインデックス
    bool _isReleasing;          // リリース中かどうかのフラグ
};

#endif
