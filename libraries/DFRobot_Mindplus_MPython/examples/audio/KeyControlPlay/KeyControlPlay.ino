#include <MPython.h>

namespace {
// 本地音频文件保存在 LittleFS 的固定路径。
// 请先确认这个文件已经写入 LittleFS 的 lfs2 分区中。
// 现在可以不写前导 /，库内部会自动补齐。
constexpr char kAudioFile[] = "test.mp3";

// 初始播放音量，范围通常为 0~100。
constexpr uint8_t kInitialVolume = 60;

// 每次触摸调节的音量步进。
constexpr uint8_t kVolumeStep = 5;

// 状态轮询间隔，单位为毫秒。
constexpr uint32_t kStatusIntervalMs = 300;

// 开始播放后的触摸保护时间，避免解码启动阶段误触发音量调节。
constexpr uint32_t kTouchGuardMs = 800;

uint8_t gVolume = kInitialVolume;
uint32_t gTouchReadyMs = 0;
bool gTouchPPressed = false;
bool gTouchYPressed = false;
}  // namespace

void printControlHints() {
    Serial.println("A: pause/resume");
    Serial.println("B: stop");
    Serial.println("A+B: restart");
    Serial.println("P touch: volume down");
    Serial.println("Y touch: volume up");
}

void setVolumeAndPrint(uint8_t volume, const char *reason) {
    gVolume = volume > 100 ? 100 : volume;
    mPython.audio.setVolume(gVolume);
    Serial.printf("%s volume=%u\n", reason, (unsigned)gVolume);
}

void calibrateTouchPad(TouchPad &pad, const char *name) {
    uint32_t sum = 0;
    for (int i = 0; i < 8; ++i) {
        sum += pad.read();
        delay(5);
    }
    uint32_t baseline = sum / 8;
    uint32_t margin = baseline / 8;
    if (margin < 3000) margin = 3000;
    pad.threshold = baseline + margin;
    Serial.printf("Touch %s baseline=%lu threshold=%u\n",
                  name,
                  (unsigned long)baseline,
                  pad.threshold);
}

bool startPlayback() {
    if (!mPython.audio.play(kAudioFile)) {
        Serial.println("Play start failed. Check whether the file exists and format is supported.");
        return false;
    }
    gTouchReadyMs = millis() + kTouchGuardMs;
    gTouchPPressed = false;
    gTouchYPressed = false;
    return true;
}

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println("=== KeyControlPlay Example ===");

    // 初始化音频模块。
    if (!mPython.audio.begin()) {
        Serial.println("Audio init failed.");
        return;
    }

    // 校准触摸按键阈值，降低空闲状态下误触发音量调节的概率。
    calibrateTouchPad(mPython.touchPadP, "P");
    calibrateTouchPad(mPython.touchPadY, "Y");

    // 设置播放音量。
    setVolumeAndPrint(kInitialVolume, "Init");

    // 播放 LittleFS 中已有的本地音频文件。play() 会立即返回，可在主线程里查看播放状态。
    if (!startPlayback()) {
        return;
    }

    printControlHints();
}

void loop() {
    static uint32_t lastStatusMs = 0;
    static Audio::PlayState lastState = Audio::PLAY_STATE_IDLE;

    if (mPython.buttonAB.isPressed()) {
        Serial.println("A+B pressed: restart");
        mPython.audio.stop();
        delay(100);
        if (!startPlayback()) {
            Serial.println("Restart failed.");
        }
        delay(250);
    } else if (mPython.buttonA.isPressed()) {
        if (mPython.audio.state() == Audio::PLAY_STATE_PAUSED) {
            Serial.println("A pressed: resume");
            mPython.audio.resume();
        } else if (mPython.audio.state() == Audio::PLAY_STATE_PLAYING) {
            Serial.println("A pressed: pause");
            mPython.audio.pause();
        }
        delay(250);
    } else if (mPython.buttonB.isPressed()) {
        Serial.println("B pressed: stop");
        mPython.audio.stop();
        delay(250);
    }

    bool pTouched = millis() >= gTouchReadyMs &&
                    (mPython.audio.state() == Audio::PLAY_STATE_PLAYING ||
                     mPython.audio.state() == Audio::PLAY_STATE_PAUSED) &&
                    mPython.touchPadP.isTouched();
    bool yTouched = millis() >= gTouchReadyMs &&
                    (mPython.audio.state() == Audio::PLAY_STATE_PLAYING ||
                     mPython.audio.state() == Audio::PLAY_STATE_PAUSED) &&
                    mPython.touchPadY.isTouched();

    if (pTouched && !gTouchPPressed) {
        uint8_t nextVolume = gVolume > kVolumeStep ? gVolume - kVolumeStep : 0;
        setVolumeAndPrint(nextVolume, "P touched:");
    }
    if (yTouched && !gTouchYPressed) {
        uint8_t nextVolume = gVolume >= 100 - kVolumeStep ? 100 : gVolume + kVolumeStep;
        setVolumeAndPrint(nextVolume, "Y touched:");
    }
    gTouchPPressed = pTouched;
    gTouchYPressed = yTouched;

    Audio::PlayState state = mPython.audio.state();
    if (state != lastState || millis() - lastStatusMs >= kStatusIntervalMs) {
        Serial.printf("state=%s volume=%u\n", mPython.audio.stateName(), (unsigned)gVolume);
        lastState = state;
        lastStatusMs = millis();
    }

    if (state == Audio::PLAY_STATE_STOPPED || state == Audio::PLAY_STATE_ERROR) {
        Serial.printf("final state=%s\n", mPython.audio.stateName());
        Serial.println("Press A+B to play again.");
        delay(300);
    }
}
