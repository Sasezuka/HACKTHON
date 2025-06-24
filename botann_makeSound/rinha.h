#ifndef RINHA_H // ヘッダガードを修正
#define RINHA_H

#include "Arduino.h"
#include "kaya.h"      // インクルードファイルを修正
#include "constants.h" 

// クラス名をmisuzuからrinhaに変更
class rinha {
public:
    rinha(); // コンストラクタ名を修正

    void handlePlayCommand(int noteIndex);
    void handleStopCommand();
    uint16_t getNextSample();

private:
    kaya _voices[NUM_NOTES]; // hatayaをkayaに変更
    int _currentNoteIndex;
    unsigned long _noteOffStartTime;
    bool _isReleasing;
};

#endif