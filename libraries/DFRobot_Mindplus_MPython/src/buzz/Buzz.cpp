#include "Buzz.h"

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

const char *const dadadadum = "r4:2|g|g|g|eb:8|r:2|f|f|f|d:8|";
const char *const entertainer = "d4:1|d#|e|c5:2|e4:1|c5:2|e4:1|c5:3|c:1|d|d#|e|c|d|e:2|b4:1|d5:2|c:4|";
const char *const prelude = "c4:1|e|g|c5|e|g4|c5|e|c4|e|g|c5|e|g4|c5|e|c4|d|g|d5|f|g4|d5|f|c4|d|g|d5|f|g4|d5|f|b3|d4|g|d5|f|g4|d5|f|b3|d4|g|d5|f|g4|d5|f|c4|e|g|c5|e|g4|c5|e|c4|e|g|c5|e|g4|c5|e|";
const char *const ode = "e4|e|f|g|g|f|e|d|c|c|d|e|e:6|d:2|d:8|e:4|e|f|g|g|f|e|d|c|c|d|e|d:6|c:2|c:8|";
const char *const nyan = "f#5:2|g#|c#:1|d#:2|b4:1|d5:1|c#|b4:2|b|c#5|d|d:1|c#|b4:1|c#5:1|d#|f#|g#|d#|f#|c#|d|b4|c#5|b4|d#5:2|f#|g#:1|d#|f#|c#|d#|b4|d5|d#|d|c#|b4|c#5|d:2|b4:1|c#5|d#|f#|c#|d|c#|b4|c#5:2|b4|c#5|b4|f#:1|g#|b:2|f#:1|g#|b|c#5|d#|b4|e5|d#|e|f#|b4:2|b|f#:1|g#|b|f#|e5|d#|c#|b4|f#|d#|e|f#|b:2|f#:1|g#|b:2|f#:1|g#|b|b|c#5|d#|b4|f#|g#|f#|b:2|b:1|a#|b|f#|g#|b|e5|d#|e|f#|b4:2|c#5|";
const char *const ringtone = "c4:1|d|e:2|g|d:1|e|f:2|a|e:1|f|g:2|b|c5:4|";
const char *const funk = "c2:2|c|d#|c:1|f:2|c:1|f:2|f#|g|c|c|g|c:1|f#:2|c:1|f#:2|f|d#|";
const char *const blues = "c2:2|e|g|a|a#|a|g|e|c2:2|e|g|a|a#|a|g|e|f|a|c3|d|d#|d|c|a2|c2:2|e|g|a|a#|a|g|e|g|b|d3|f|f2|a|c3|d#|c2:2|e|g|e|g|f|e|d|";
const char *const birthday = "c4:3|c:1|d:4|c:4|f|e:8|c:3|c:1|d:4|c:4|g|f:8|c:3|c:1|c5:4|a4|f|e|d|a#:3|a#:1|a:4|f|g|f:8|";
const char *const wedding = "c4:4|f:3|f:1|f:8|c:4|g:3|e:1|f:8|c:4|f:3|a:1|c5:4|a4:3|f:1|f:4|e:3|f:1|g:8|";
const char *const funeral = "c3:4|c:3|c:1|c:4|d#:3|d:1|d:3|c:1|c:3|b2:1|c3:4|";
const char *const punchline = "c4:3|g3:1|f#|g|g#:3|g|r|b|c4|";
const char *const baddy = "c3:3|r|d:2|d#|r|c|r|f#:8|";
const char *const chase = "a4:1|b|c5|b4|a:2|r|a:1|b|c5|b4|a:2|r|a:2|e5|d#|e|f|e|d#|e|b4:1|c5|d|c|b4:2|r|b:1|c5|d|c|b4:2|r|b:2|e5|d#|e|f|e|d#|e|";
const char *const baDing = "b5:1|e6:3|";
const char *const wawawawaa = "e3:3|r:1|d#:3|r:1|d:4|r:1|c#:8|";
const char *const JumpUp = "c5:1|d|e|f|g|";
const char *const jumpDown = "g5:1|f|e|d|c|";
const char *const PowerUp = "g4:1|c5|e|g:2|e:1|g:3|";
const char *const powerDown = "g5:1|d#|c|g4:2|b:1|c5:3|";
int freqTable[] = {31, 33, 35, 37, 39, 41, 44, 46, 49, 52, 55, 58, 62, 65, 69, 73, 78, 82, 87, 92, 98, 104, 110, 117, 123, 131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 247, 262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988, 1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976, 2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951, 4186};

Buzz::Buzz()
    : _pin(16), _freq(0), _on(false), _playing(false), ticks(4), tempo(60)
{
}

void Buzz::begin(uint8_t pin) {
    _pin = pin;
    pinMode(_pin, OUTPUT);
}

void Buzz::on(void)
{
    if (_pin < 0)
        return;
    _on = true;
}

void Buzz::off(void)
{
    if (_pin < 0)
        return;
    noTone(_pin);
    digitalWrite(_pin, 0); // fireBeetleEsp32 停止异常bug
    _on = false;
}

bool Buzz::isOn(void)
{
    return _on;
}

void Buzz::setTicksTempo(uint32_t _ticks, uint32_t _tempo)
{
    ticks = _ticks;
    tempo = _tempo;
}

void Buzz::freq(uint32_t _freq)
{
    if (!_on)
    {
        on();
        _on = true;
    }
    tone(_pin, _freq);
}



void Buzz::freq(uint32_t _freq, Beat beat)
{
    long delay_time = 0;
    switch (beat)
    {
    case BEAT_1:
        delay_time = 1000;
        break;
    case BEAT_1_2:
        delay_time = 500;
        break;
    case BEAT_1_4:
        delay_time = 250;
        break;
    case BEAT_2:
        delay_time = 2000;
        break;
    case BEAT_4:
        delay_time = 4000;
        break;
    case BEAT_3_4:
        delay_time = 750;
        break;
    case BEAT_3_2:
        delay_time = 1500;
        break;
    case BEAT_3:
        delay_time = 3000;
        break;
    }
    if (!_on)
    {
        on();
        _on = true;
    }
    tone(_pin, _freq);
    if (delay_time)
    {
        delay_time = delay_time * ((60.0 / tempo) * (4.0 / ticks));
        delay(delay_time);
    }

    off();
}

void Buzz::play(Music music, MelodyOptions options)
{
    stop();
    buzzMelody.currentDuration = 4; //Default duration (Crotchet)
    buzzMelody.currentOctave = 4;   //Middle octave
    buzzMelody.beatsPerMinute = 120;
    buzzMelody.melody = String(getMelody(music));
    buzzMelody.opt = options;
    if (buzzMelody.melody == "null")
        return;
    _playing = true;
    xTaskCreatePinnedToCore(Buzz::taskLoop, "taskLoop", 1024, this, 1, &buzzMelody.task, ARDUINO_RUNNING_CORE);
    if (options == Once || options == Forever)
    {
        while (_playing)
        {
            yield();
        }
    }
    else
    {
        return;
    }
}

void Buzz::stop()
{
    if (_playing)
    { // 如果发现后台正在播放，删除后台任务
        _playing = false;
        off();
        if (buzzMelody.task)
        {
            vTaskDelete(buzzMelody.task);
            buzzMelody.task = NULL;
        }
        delay(50);
    }
    else
    { // 停止音符
        off();
    }
}

void Buzz::playNextNote(String note)
{
    // cache elements
    String currNote = note;
    int currentDuration = buzzMelody.currentDuration;
    int currentOctave = buzzMelody.currentOctave;
    String currLen = currNote;

    int noteValue = 0;
    boolean isrest = false;
    int beatPos;
    boolean parsingOctave = true;

    for (int pos = 0; pos < currLen.length(); pos++)
    {
        char noteChar = currNote[pos];
        switch (noteChar)
        {
        case 'c':
        case 'C':
            noteValue = 1;
            break;
        case 'd':
        case 'D':
            noteValue = 3;
            break;
        case 'e':
        case 'E':
            noteValue = 5;
            break;
        case 'f':
        case 'F':
            noteValue = 6;
            break;
        case 'g':
        case 'G':
            noteValue = 8;
            break;
        case 'a':
        case 'A':
            noteValue = 10;
            break;
        case 'b':
        case 'B':
            noteValue = 12;
            break;
        case 'r':
        case 'R':
            isrest = true;
            break;
        case '#':
            noteValue++;
            break;
        case ':':
            parsingOctave = false;
            beatPos = pos;
            break;
        default:
            if (parsingOctave)
                currentOctave = (int)currNote[pos] - 48;
        }
    }
    if (!parsingOctave)
    {
        currentDuration = (int)currNote[beatPos + 1] - 48;
    }
    int beat = (60000 / buzzMelody.beatsPerMinute) / 4;
    if (isrest)
    {
        off();
    }
    else
    {
        int keyNumber = noteValue + (12 * (currentOctave - 1));
        int frequency = keyNumber >= 0 && keyNumber < sizeof(freqTable) / sizeof(freqTable[0]) ? freqTable[keyNumber] : 0;
        if (frequency > 0) {
            if (!_on) {
                on();
                _on = true;
            }
            tone(_pin, frequency);
            delay(currentDuration * beat);
            off();
            delay(5);
        }
    }
    buzzMelody.currentDuration = currentDuration;
    buzzMelody.currentOctave = currentOctave;
}

const char *const Buzz::getMelody(Music music)
{
    switch (music)
    {
    case DADADADUM:
        return dadadadum;
    case ENTERTAINER:
        return entertainer;
    case PRELUDE:
        return prelude;
    case ODE:
        return ode;
    case NYAN:
        return nyan;
    case RINGTONE:
        return ringtone;
    case FUNK:
        return funk;
    case BLUES:
        return blues;
    case BIRTHDAY:
        return birthday;
    case WEDDING:
        return wedding;
    case FUNERAL:
        return funeral;
    case PUNCHLINE:
        return punchline;
    case BADDY:
        return baddy;
    case CHASE:
        return chase;
    case BA_DING:
        return baDing;
    case WAWAWAWAA:
        return wawawawaa;
    case JUMP_UP:
        return JumpUp;
    case JUMP_DOWN:
        return jumpDown;
    case POWER_UP:
        return PowerUp;
    case POWER_DOWN:
        return powerDown;
    default:
        return "null";
    }
}

void Buzz::taskLoop(void *param)
{
    Buzz *self = (Buzz *)param;
    self->_playing = true;
    int head=0, trail=0, index=0;
    while (self->_playing) {
        for(; index < self->buzzMelody.melody.length(); index++){
            if(self->buzzMelody.melody[index] == '|'){
                trail = index;
                self->playNextNote(self->buzzMelody.melody.substring(head, trail));
                index++;
                head = index;
                break;
            }
        }
        if (head >= self->buzzMelody.melody.length()) {
            if(self->buzzMelody.opt == Forever || 
                self->buzzMelody.opt == ForeverInBackground) {
                head=0, trail=0, index=0;
            }else{
                break;
            }
        }
    }
    self->_playing = false;
    self->buzzMelody.task = NULL;
    vTaskDelete(NULL); // 播放完成删除任务
}