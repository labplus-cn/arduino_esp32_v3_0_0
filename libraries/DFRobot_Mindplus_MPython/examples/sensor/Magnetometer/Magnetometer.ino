#include <MPython.h>

void setup() {
    Serial.begin(115200);
    mPython.begin();
    Serial.println("=== Magnetometer Example ===");
}

void loop() {
    float x = mPython.magnetometer.getX();
    float y = mPython.magnetometer.getY();
    float z = mPython.magnetometer.getZ();

    Serial.print("X: ");  Serial.print(x, 2);
    Serial.print(" uT  Y: "); Serial.print(y, 2);
    Serial.print(" uT  Z: "); Serial.print(z, 2);
    Serial.println(" uT");

    delay(500);
}
