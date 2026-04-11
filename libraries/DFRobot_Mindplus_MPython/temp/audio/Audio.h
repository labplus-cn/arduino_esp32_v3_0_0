#ifndef __AUDIO_H
#define __AUDIO_H

#include "Arduino.h"
#include <I2S.h>

class Audio {
public:
    Audio();
    void begin();
    void startRecording(uint32_t sampleRate = 16000, uint8_t bitsPerSample = 16, uint8_t channels = 1);
    void stopRecording();
    void startPlaying(uint32_t sampleRate = 16000, uint8_t bitsPerSample = 16, uint8_t channels = 1);
    void stopPlaying();
    size_t read(uint8_t* buffer, size_t size);
    size_t write(uint8_t* buffer, size_t size);
    bool isRecording();
    bool isPlaying();

private:
    I2SClass _i2s;
    bool _recording;
    bool _playing;
    uint32_t _sampleRate;
    uint8_t _bitsPerSample;
    uint8_t _channels;
};

#endif