#include <MPython.h>

void setup(){
    Serial.begin(115200);
	accelerometer.init();
}
void Aup(void){
  Serial.printf("-----buttonA ----up---\n");
}
void Adown(void){
  Serial.printf("-----buttonA ----down---\n");
  delay(1000);
}
void Bup(void){
  Serial.printf("-----buttonB ----up---\n");
}
void Bdown(void){
  Serial.printf("-----buttonB ----down---\n");
  delay(1000);
}

void Pup(void){
  Serial.printf("-----TouchPad PPP value=----up---\n");
}
void Pdown(void){
  Serial.printf("-----TouchPad PPP value=----down---\n");
  delay(1000);
}
void Yup(void){
  Serial.printf("-----TouchPad YYY value=----up---\n");
}
void Ydown(void){
  Serial.printf("-----TouchPad PPP value=----down---\n");
  delay(1000);
}

void Tup(void){
  Serial.printf("-----TouchPad TTT value=----up---\n");
}
void Tdown(void){
  Serial.printf("-----TouchPad TTT value=----down---\n");
  delay(1000);
}
void Hup(void){
  Serial.printf("-----TouchPad HHH value=----up---\n");
}
void Hdown(void){
  Serial.printf("-----TouchPad HHH value=----down---\n");
  delay(1000);
}

void Oup(void){
  Serial.printf("-----TouchPad OOO value=----up---\n");
}
void Odown(void){
  Serial.printf("-----TouchPad OOO value=----down---\n");
  delay(1000);
}

void Nup(void){
  Serial.printf("-----TouchPad NNN value=----up---\n");
}
void Ndown(void){
  Serial.printf("-----TouchPad NNN value=----down---\n");
  delay(1000);
}
void loop(void){
  Serial.println("-------------------------");
  Serial.printf("light=%d\n",light.read());
	Serial.printf("sound=%d\n",sound.read());
	Serial.printf("X=%f Y=%f Z=%f Strength=%f\n",accelerometer.getX(),accelerometer.getY(),accelerometer.getZ(),accelerometer.getStrength());
 buttonA.setPressedCallback(Adown);
 buttonA.setUnPressedCallback(Aup);
 buttonB.setPressedCallback(Bdown);
 buttonB.setUnPressedCallback(Bup);
 touchPadP.setTouchedCallback(Pdown);
 touchPadP.setUnTouchedCallback(Pup);
  touchPadY.setTouchedCallback(Ydown);
 touchPadY.setUnTouchedCallback(Yup);
  touchPadT.setTouchedCallback(Tdown);
 touchPadT.setUnTouchedCallback(Tup);
  touchPadH.setTouchedCallback(Hdown);
 touchPadH.setUnTouchedCallback(Hup);
  touchPadO.setTouchedCallback(Odown);
 touchPadO.setUnTouchedCallback(Oup);
  touchPadN.setTouchedCallback(Ndown);
 touchPadN.setUnTouchedCallback(Nup);
 delay(500);
 buzz.on();
 for(int i=0;i<10;i++){
   buzz.freq(262+i*10.0);
   delay(200);
 }
	rgb.write(0,25,0,0);delay(100);
	rgb.write(1,25,0,0);delay(100);
	rgb.write(2,25,0,0);delay(100);
	rgb.write(0,0,25,0);delay(100);
	rgb.write(1,0,25,0);delay(100);
	rgb.write(2,0,25,0);delay(100);
	rgb.write(0,0,0,25);delay(100);
	rgb.write(1,0,0,25);delay(100);
	rgb.write(2,0,0,25);delay(100);
}