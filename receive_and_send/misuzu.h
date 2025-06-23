#ifndef misuzu_h
#define misuzu_h

#include "Arduino.h"
#include "hataya.h"    
#include "constants.h" 

class misuzu {
public:
    misuzu();

    void handlePlayCommand(int noteIndex);
    void handleStopCommand();
    uint16_t getNextSample();

private:
    hataya _voices[NUM_NOTES]; 
    int _currentNoteIndex;
    unsigned long _noteOffStartTime;
    bool _isReleasing;
};

#endif