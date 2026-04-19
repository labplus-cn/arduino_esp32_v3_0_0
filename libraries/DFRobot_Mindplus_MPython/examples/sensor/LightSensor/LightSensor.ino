#include <MPython.h>

void setup() {
    Serial.begin(115200);
    mPython.begin();
    Serial.println("=== LightSensor Example ===");
}

void loop() {
    uint16_t lux = mPython.lightSensor.getLux();
    Serial.print("Lux: ");
    Serial.println(lux);
    delay(500);
}
