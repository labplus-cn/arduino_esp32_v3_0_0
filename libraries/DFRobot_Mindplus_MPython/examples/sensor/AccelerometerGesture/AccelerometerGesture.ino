#include <MPython.h>

void onShake() {
    Serial.println("Gesture: Shake");
}

void onScreenUp() {
    Serial.println("Gesture: Screen Up");
}

void onScreenDown() {
    Serial.println("Gesture: Screen Down");
}

void onTiltLeft() {
    Serial.println("Gesture: Tilt Left");
}

void onTiltRight() {
    Serial.println("Gesture: Tilt Right");
}

void onTiltForward() {
    Serial.println("Gesture: Tilt Forward");
}

void onTiltBack() {
    Serial.println("Gesture: Tilt Back");
}

void setup() {
    Serial.begin(115200);
    mPython.begin();
    Serial.println("=== Accelerometer Gesture Example ===");

    mPython.accelerometer.onGesture(Accelerometer::Shake,       onShake);
    mPython.accelerometer.onGesture(Accelerometer::ScreenUp,    onScreenUp);
    mPython.accelerometer.onGesture(Accelerometer::ScreenDown,  onScreenDown);
    mPython.accelerometer.onGesture(Accelerometer::TiltLeft,    onTiltLeft);
    mPython.accelerometer.onGesture(Accelerometer::TiltRight,   onTiltRight);
    mPython.accelerometer.onGesture(Accelerometer::TiltForward, onTiltForward);
    mPython.accelerometer.onGesture(Accelerometer::TiltBack,    onTiltBack);
}

void loop() {
    delay(100);
}
