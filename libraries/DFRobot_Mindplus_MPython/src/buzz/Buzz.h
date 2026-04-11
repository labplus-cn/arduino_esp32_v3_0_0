#ifndef __BUZZ_H
#define __BUZZ_H

#include "Arduino.h"
#include "driver/ledc.h"

//To the beat
enum Beat {
    BEAT_1 = 4,
    BEAT_1_2 = 2,
    BEAT_1_4 = 1,
    BEAT_3_4 = 3,
    BEAT_3_2 = 6,
    BEAT_2 = 8,
    BEAT_3 = 12,
    BEAT_4 = 16
};

//music
enum Melodies {
    DADADADUM = 0,
    ENTERTAINER,
    PRELUDE,
    ODE,
    NYAN,
    RINGTONE,
    FUNK,
    BLUES,
    BIRTHDAY,
    WEDDING,
    FUNERAL,
    PUNCHLINE,
    BADDY,
    CHASE,
    BA_DING,
    WAWAWAWAA,
    JUMP_UP,
    JUMP_DOWN,
    POWER_UP,
    POWER_DOWN,
};

//Play mode
enum MelodyOptions {
    Once = 1,
    Forever = 2,
    OnceInBackground = 4,
    ForeverInBackground = 8
};

class Buzz {
public:
    Buzz();
    void begin(uint8_t pin = 21);
    void on(void);
    void off(void);
    bool isOn(void);
    void setTicksTempo(uint32_t _ticks, uint32_t _tempo);
    void freq(uint32_t _freq = 500);
    void freq(int _freq = 500);
    void freq(double _freq = 500.0);
    void freq(uint32_t _freq, Beat beat);
    void freq(int _freq, Beat beat);
    void freq(double _freq, Beat beat);
    void freq(uint32_t _freq, double beat);
    void freq(uint32_t _freq, uint32_t beat);
    void freq(uint32_t _freq, int beat);
    void redirect(uint32_t pin);
    void stop();
    void play(Melodies melodies, MelodyOptions options = Once);

private:
    int16_t  _pin;
    uint16_t _freq;
    bool     _on;
    bool     _playing;
    float ticks;
    float tempo;

    struct buzzMelody_t {
        String melody;
        int currentDuration;
        int currentOctave;
        int beatsPerMinute;
        MelodyOptions opt;
        TaskHandle_t task;
    } buzzMelody;

    void     playNextNote(String tone);
    const char* const getMelody(Melodies melody);
    static void taskLoop(void *param);
};

#endif