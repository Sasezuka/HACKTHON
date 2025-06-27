#ifndef PLAYSOUND_H
#define PLAYSOUND_H

#include "makeSound.h"
#include "constants.h"

class playSound {
public:
    playSound();
    void handlePlayCommand(int noteIndex);
    void handleStopCommand();
    uint16_t getNextSample();

private:
    makeSound _voices[NUM_NOTES]; // 各ノートに対応する音源
    int _currentNoteIndex;      // 現在鳴っている音のインデックス
    bool _isReleasing;          // リリース中かどうかのフラグ
};

#endif
