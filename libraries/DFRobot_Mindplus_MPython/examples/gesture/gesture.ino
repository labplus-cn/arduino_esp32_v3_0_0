#include <MPython.h>

void TiltForwardEvent(){
  Serial.println("TiltForward");
}
void TiltBackEvent(){
  Serial.println("TiltBack");
}
void TiltRightEvent(){
  Serial.println("TiltRight");
}
void TiltLeftEvent(){
  Serial.println("TiltLeft");
}
void ScreenDownEvent(){
  Serial.println("ScreenDown");
}
void ScreenUpEvent(){
  Serial.println("ScreenUp");
}
void ShakeEvent(){
  Serial.println("Shake");
}

void setup() {
  mPython.begin();
  Serial.begin(9600);
  accelerometer.onGesture(accelerometer.TiltForward, TiltForwardEvent);
  accelerometer.onGesture(accelerometer.TiltBack, TiltBackEvent);
  accelerometer.onGesture(accelerometer.TiltRight, TiltRightEvent);
  accelerometer.onGesture(accelerometer.TiltLeft, TiltLeftEvent);
  accelerometer.onGesture(accelerometer.ScreenDown, ScreenDownEvent);
  accelerometer.onGesture(accelerometer.ScreenUp, ScreenUpEvent);
  accelerometer.onGesture(accelerometer.Shake, ShakeEvent);
}

void loop() {

}

/*
#include <MPython.h>

void setup() {
  mPython.begin();
  Serial.begin(9600);
}

void loop() {
  if(accelerometer.isGesture(accelerometer.TiltForward)){
    Serial.println("TiltForward");
  }
  if(accelerometer.isGesture(accelerometer.TiltBack)){
    Serial.println("TiltBack");
  }
  if(accelerometer.isGesture(accelerometer.TiltRight)){
    Serial.println("TiltRight");
  }
  if(accelerometer.isGesture(accelerometer.TiltLeft)){
    Serial.println("TiltLeft");
  }
  if(accelerometer.isGesture(accelerometer.ScreenDown)){
    Serial.println("ScreenDown");
  }
  if(accelerometer.isGesture(accelerometer.ScreenUp)){
    Serial.println("ScreenUp");
  }
  if(accelerometer.isGesture(accelerometer.Shake)){
    Serial.println("Shake");
  }
}
*/



