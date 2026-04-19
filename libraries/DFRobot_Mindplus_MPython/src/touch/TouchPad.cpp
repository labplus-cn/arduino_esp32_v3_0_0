#include "TouchPad.h"
#include "esp32-hal-touch.h"

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

// 触摸中断回调函数
static void touchInterruptCallback(void *arg)
{
    // 只在中断中设置标志位，不执行其他操作
    TouchPad *self = (TouchPad *)arg;
    self->touchDetected = true;
}

// 任务函数
static void taskLoop(void *param)
{
    TouchPad *self = (TouchPad *)param;
    bool lastState = false;
    
    while(1) {
        // 检查触摸状态
        if (self->touchDetected) {
            self->touchDetected = false;
            
            // 使用touchInterruptGetLastStatus获取触摸状态
            bool currentState = touchInterruptGetLastStatus(self->io);
            
            if (lastState != currentState) {
                if (currentState && self->touchedCb) {
                    self->touchedCb();
                }
                if (!currentState && self->untouchedCb) {
                    self->untouchedCb();
                }
                lastState = currentState;
            }
        }
        
        delay(50);
    }
}

TouchPad::TouchPad(uint8_t _io)
    : threshold(50000), io(_io), valuePrevPrev(20000), valuePrev(20000), value(20000)
{
    touchedCb = NULL;
    untouchedCb = NULL;
    touchDetected = false;
    taskCreated = false;
}

uint32_t TouchPad::read(void)
{
    return touchRead(io);
}

bool TouchPad::isTouched(void)
{
    valuePrevPrev = valuePrev;
    valuePrev = value;
    value = touchRead(io);
    // 对于ESP32-S3，touchRead()返回值越大表示触摸越强烈
    return (value > threshold);
}

void TouchPad::setTouchedCallback(CBFunc cb)
{
    touchedCb = cb;
    if ((touchedCb || untouchedCb) && !taskCreated) {
        // 附加中断回调，使用带参数的版本
        touchAttachInterruptArg(io, touchInterruptCallback, this, threshold);
        
        // 创建任务来检测状态变化
        static char name[] = {"touchX"};
        name[5] = io + '0';
        xTaskCreatePinnedToCore(taskLoop, name, 2048, this, 1, NULL, ARDUINO_RUNNING_CORE);
        taskCreated = true;
    }
}

void TouchPad::setUnTouchedCallback(CBFunc cb)
{
    untouchedCb = cb;
    if ((touchedCb || untouchedCb) && !taskCreated) {
        // 附加中断回调，使用带参数的版本
        touchAttachInterruptArg(io, touchInterruptCallback, this, threshold);
        
        // 创建任务来检测状态变化
        static char name[] = {"touchX"};
        name[5] = io + '0';
        xTaskCreatePinnedToCore(taskLoop, name, 2048, this, 1, NULL, ARDUINO_RUNNING_CORE);
        taskCreated = true;
    }
}

