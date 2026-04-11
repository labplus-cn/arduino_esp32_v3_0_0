#include "TouchPad.h"

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

TouchPad::TouchPad(uint8_t _io)
    : touchedCb(NULL), untouchedCb(NULL), io(_io), valuePrevPrev(80), valuePrev(80), value(80), threshold(35)
{
}

uint32_t TouchPad::read(void)
{
    if ((touchedCb == NULL) && (untouchedCb == NULL))
    {
        return touchRead(io);
    }
    return value;
}

bool TouchPad::isTouched(void)
{
    valuePrevPrev = valuePrev;
    valuePrev = value;
    value = touchRead(io);
    return ((value < threshold) && (valuePrev < threshold) && (valuePrevPrev < threshold));
}

void TouchPad::setTouchedCallback(CBFunc cb)
{
    static char name[] = {"touchX"};
    name[5] = io + '0';
    if ((touchedCb == NULL) && (untouchedCb == NULL))
    {
        xTaskCreatePinnedToCore(TouchPad::taskLoop, name, 2048, this, 1, NULL, ARDUINO_RUNNING_CORE);
    }
    touchedCb = cb;
}

void TouchPad::setUnTouchedCallback(CBFunc cb)
{
    static char name[] = {"touchX"};
    name[5] = io + '0';
    if ((touchedCb == NULL) && (untouchedCb == NULL))
    {
        xTaskCreatePinnedToCore(TouchPad::taskLoop, name, 2048, this, 1, NULL, ARDUINO_RUNNING_CORE);
    }
    untouchedCb = cb;
}

void TouchPad::taskLoop(void *param)
{
    bool prevState = false;
    TouchPad *self = (TouchPad *)param;
    while(1){
        bool isTouched = self->isTouched();
        if(prevState != isTouched){
            if(isTouched && self->touchedCb){
                self->touchedCb();
            }
            if((!isTouched) && self->untouchedCb){
                self->untouchedCb();
            }
            prevState = isTouched;
        }
        delay(10);
    }
}