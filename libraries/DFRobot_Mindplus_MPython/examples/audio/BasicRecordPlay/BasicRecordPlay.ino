#include <MPython.h>
#include <LittleFS.h>

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println("=== BasicRecordPlay DEBUG V2 ===");

    if (!mPython.audio.begin()) {
        Serial.println("audio init failed");
        return;
    }

    Serial.println("start record -> /record.wav");
    if (!mPython.audio.startRecord("/record.wav", 16000, 16, 1)) {
        Serial.println("record start failed");
        return;
    }

    uint32_t start = millis();
    size_t total = 0;
    while (millis() - start < 3000) {
        total += mPython.audio.recordChunk();
        delay(10);
    }
    mPython.audio.stopRecord();
    Serial.printf("record done, captured=%u bytes\n", (unsigned)total);

    File f = LittleFS.open("/record.wav", FILE_READ);
    if (!f) {
        Serial.println("open /record.wav failed");
        return;
    }
    Serial.printf("/record.wav size=%u bytes\n", (unsigned)f.size());
    uint8_t header[16] = {0};
    size_t n = f.read(header, sizeof(header));
    Serial.print("header=");
    for (size_t i = 0; i < n; ++i) {
        if (header[i] < 16) Serial.print('0');
        Serial.print(header[i], HEX);
        Serial.print(' ');
    }
    Serial.println();
    f.close();

    delay(1000);
    Serial.println("play /record.wav");
    if (!mPython.audio.playFile("/record.wav")) {
        Serial.println("play failed");
    } else {
        Serial.println("play done");
    }
}

void loop() {
}
