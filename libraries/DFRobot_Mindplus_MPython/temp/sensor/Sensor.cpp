#include "Sensor.h"
#include <math.h>

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#define MMC5603NJ_I2C_ADDR 0x30
#define LTR308ALS_I2C_ADDR 0x29

static uint8_t i2cread(void)
{
    return Wire.read();
}

static void i2cwrite(uint8_t x)
{
    Wire.write((uint8_t)x);
}

static void writeRegister(uint8_t i2cAddress, uint8_t reg, uint8_t value)
{
    Wire.setClock(400000);
    Wire.beginTransmission(i2cAddress);
    i2cwrite(reg);
    i2cwrite(value);
    Wire.endTransmission();
}

Accelerometer::Accelerometer()
{
    for (uint8_t i = 0; i < 7; i++)
    {
        GestureHandle[i] = NULL;
    }
    offsetX = 0;
    offsetY = 0;
    offsetZ = 0;
    currentGesture = GestureNone;
    isGestureEnable = false;
    _is_shaked = false;
}

void Accelerometer::init(void)
{
    Wire.begin();
    _qmi8658.begin();
    _qmi8658.setAccelUnit_mg(true); // 使用mg作为单位
    _qmi8658.enableAccel(true);
    delay(100);
}

float Accelerometer::getX(void)
{
    float x, y, z;
    _qmi8658.readAccelMG(x, y, z);
    rawX = x;
    return rawX + offsetX;
}

float Accelerometer::getY(void)
{
    float x, y, z;
    _qmi8658.readAccelMG(x, y, z);
    rawY = y;
    return rawY + offsetY;
}

float Accelerometer::getZ(void)
{
    float x, y, z;
    _qmi8658.readAccelMG(x, y, z);
    rawZ = z;
    return rawZ + offsetZ;
}

void Accelerometer::setOffset(int x, int y, int z)
{
    offsetX = x;
    offsetY = y;
    offsetZ = z;
}

float Accelerometer::getStrength(void)
{
    float x = getX();
    float y = getY();
    float z = getZ();
    return sqrt(x * x + y * y + z * z);
}

void Accelerometer::onGesture(Gesture gesture, mpythonGestureHandlePtr body)
{
    if (!isGestureEnable)
    {
        xTaskCreatePinnedToCore(Accelerometer::taskLoop, "gestureTask", 2048, this, 1, NULL, ARDUINO_RUNNING_CORE);
        isGestureEnable = true;
    }
    GestureHandle[gesture] = body;
}

bool Accelerometer::isGesture(Gesture gesture)
{
    if (!isGestureEnable)
    {
        xTaskCreatePinnedToCore(Accelerometer::taskLoop, "gestureTask", 2048, this, 1, NULL, ARDUINO_RUNNING_CORE);
        isGestureEnable = true;
    }
    if (gesture != currentGesture)
        return false;
    return true;
}

void Accelerometer::taskLoop(void *param)
{
    Accelerometer *self = (Accelerometer *)param;
    char type = ' ';
    float _last_x, _last_y, _last_z, diff_x, diff_y, diff_z;
    int _count_shaked = 0;
    while(1){
        wait:
        delay(100);//delay放前面，让其他iic设备先行
        if(self->_is_shaked){
            _count_shaked += 1;
            if(_count_shaked==5)
                _count_shaked = 0;
        }
        float x = 0.0;
        float y = 0.0;
        float z = 0.0;

        x = self->getX() / 1000.0; // 转换为g
        y = self->getY() / 1000.0;
        z = self->getZ() / 1000.0;
        if(_last_x == 0 && _last_y == 0 && _last_z == 0){
            _last_x = x; _last_y = y; _last_z = z;
            goto yyy;
        }
        diff_x = x - _last_x; diff_y = y - _last_y; diff_z = z - _last_z;
        _last_x = x; _last_y = y; _last_z = z;
        if(_count_shaked > 0)
            goto yyy;
        self->_is_shaked = (diff_x * diff_x + diff_y * diff_y + diff_z * diff_z) > 1;
        yyy:
        if(self->_is_shaked){
            self->currentGesture = Shake;
            if(type == '0') goto wait;
            type = '0';
            if(self->GestureHandle[Shake]) self->GestureHandle[Shake]();
            goto wait;
        }
        else if(x < -0.5){
            self->currentGesture = TiltForward;
            if(type == '5') goto wait;
            type='5';
            if(self->GestureHandle[TiltForward]) self->GestureHandle[TiltForward]();
        }else if(x > 0.5){
            self->currentGesture = TiltBack;
            if(type == '6')  goto wait;
            type='6';
            if(self->GestureHandle[TiltBack]) self->GestureHandle[TiltBack]();
        }else if(y < -0.5){
            self->currentGesture = TiltRight;
            if(type == '4')  goto wait;
            type='4';
            if(self->GestureHandle[TiltRight]) self->GestureHandle[TiltRight]();
        }else if(y > 0.5){
            self->currentGesture = TiltLeft;
            if(type == '3')  goto wait;
            type='3';
            if(self->GestureHandle[TiltLeft]) self->GestureHandle[TiltLeft]();
        }else if(fabs(x)<0.2 && fabs(y)<0.2 && z>0){
            self->currentGesture = ScreenDown;
            if(type == '2')  goto wait;
            type='2';
            if(self->GestureHandle[ScreenDown]) self->GestureHandle[ScreenDown]();
        }else if(fabs(x)<0.2 && fabs(y)<0.2 && z<0){
            self->currentGesture = ScreenUp;
            if(type == '1')  goto wait;
            type='1';
            if(self->GestureHandle[ScreenUp]) self->GestureHandle[ScreenUp]();
        }else{
            self->currentGesture = GestureNone;
            type = ' ';
        }
    }
}

Magnetometer::Magnetometer() {
    offsetX = 0;
    offsetY = 0;
    offsetZ = 0;
    rawX = 0;
    rawY = 0;
    rawZ = 0;
}

void Magnetometer::init(void) {
    Wire.begin();
    // 初始化MMC5603NJ磁力计
    _mmc5603.begin();
    _mmc5603.setContinuousMode(true);
    _mmc5603.setDataRate(100); // 设置数据率为100Hz
}

void Magnetometer::readData(void) {
    mPython_MMC5603_Data_t data;
    if (_mmc5603.getData(&data)) {
        rawX = data.magX;
        rawY = data.magY;
        rawZ = data.magZ;
    }
}

float Magnetometer::getX(void) {
    readData();
    return rawX + offsetX;
}

float Magnetometer::getY(void) {
    readData();
    return rawY + offsetY;
}

float Magnetometer::getZ(void) {
    readData();
    return rawZ + offsetZ;
}

void Magnetometer::setOffset(int x, int y, int z) {
    offsetX = x;
    offsetY = y;
    offsetZ = z;
}

LightSensor::LightSensor() {
}

void LightSensor::init(void) {
    Wire.begin();
    // 初始化LTR-308ALS-01数字光线传感器
    _ltr308.begin();
    _ltr308.setPowerUp();
    _ltr308.setGain(1); // 设置3X增益
    _ltr308.setMeasurementRate(2, 2); // 设置100ms积分时间和100ms测量速率
}

uint16_t LightSensor::readLux(void) {
    unsigned long data;
    double lux;
    byte gain, integrationTime, measurementRate;
    
    _ltr308.getGain(gain);
    _ltr308.getMeasurementRate(integrationTime, measurementRate);
    
    if (_ltr308.getData(data) && _ltr308.getLux(gain, integrationTime, data, lux)) {
        return (uint16_t)lux;
    }
    return 0;
}

uint16_t LightSensor::getLux(void) {
    return readLux();
}