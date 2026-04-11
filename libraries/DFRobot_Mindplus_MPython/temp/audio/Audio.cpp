#include "Audio.h"

Audio::Audio() {
    _recording = false;
    _playing = false;
    _sampleRate = 16000;
    _bitsPerSample = 16;
    _channels = 1;
}

void Audio::begin() {
    // 初始化I2S接口
    _i2s.begin(I2S_MODE_STD, _sampleRate, _bitsPerSample);
    // 这里可以添加ES8388的初始化代码
}

void Audio::startRecording(uint32_t sampleRate, uint8_t bitsPerSample, uint8_t channels) {
    _sampleRate = sampleRate;
    _bitsPerSample = bitsPerSample;
    _channels = channels;
    
    _i2s.end();
    _i2s.begin(I2S_MODE_RX, _sampleRate, _bitsPerSample);
    _recording = true;
    _playing = false;
}

void Audio::stopRecording() {
    if (_recording) {
        _i2s.end();
        _recording = false;
    }
}

void Audio::startPlaying(uint32_t sampleRate, uint8_t bitsPerSample, uint8_t channels) {
    _sampleRate = sampleRate;
    _bitsPerSample = bitsPerSample;
    _channels = channels;
    
    _i2s.end();
    _i2s.begin(I2S_MODE_TX, _sampleRate, _bitsPerSample);
    _playing = true;
    _recording = false;
}

void Audio::stopPlaying() {
    if (_playing) {
        _i2s.end();
        _playing = false;
    }
}

size_t Audio::read(uint8_t* buffer, size_t size) {
    if (!_recording) {
        return 0;
    }
    return _i2s.read(buffer, size);
}

size_t Audio::write(uint8_t* buffer, size_t size) {
    if (!_playing) {
        return 0;
    }
    return _i2s.write(buffer, size);
}

bool Audio::isRecording() {
    return _recording;
}

bool Audio::isPlaying() {
    return _playing;
}