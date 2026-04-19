#include <MPython.h>

void setup() {
    Serial.begin(115200);
    mPython.begin();
    Serial.println("=== Accelerometer Example ===");
}

void loop() {
    float x        = mPython.accelerometer.getX();
    float y        = mPython.accelerometer.getY();
    float z        = mPython.accelerometer.getZ();
    float strength = mPython.accelerometer.getStrength();

    Serial.print("X: ");        Serial.print(x, 2);
    Serial.print(" mg  Y: ");   Serial.print(y, 2);
    Serial.print(" mg  Z: ");   Serial.print(z, 2);
    Serial.print(" mg  Strength: "); Serial.print(strength, 2);
    Serial.println(" mg");

    delay(500);
}
