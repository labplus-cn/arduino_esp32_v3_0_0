#include "Arduino.h"
#include "Button.h"

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

Button::Button(uint8_t _io)
    : pressedCb(NULL), unpressedCb(NULL), io(_io), io1(-1), io2(-1), pinNum(1)
{
    pinMode(io, INPUT | PULLUP);
    prevState = isPressed(); // 初始化prevState为按钮的实际状态
    threshold = 35;
}

Button::Button(uint8_t _io1, uint8_t _io2)
    : pressedCb(NULL), unpressedCb(NULL), io(-1), io1(_io1), io2(_io2), pinNum(2)
{
    pinMode(io1, INPUT | PULLUP);
    pinMode(io2, INPUT | PULLUP);
    prevState = isPressed(); // 初始化prevState为按钮的实际状态
    threshold = 35;
}

bool Button::isPressed(void)
{
    int retry = 10;
    if (pinNum == 1)
    {
        if (digitalRead(io) == 0)
        {
            while (retry--)
            {
                if (digitalRead(io))
                    return false;
                delay(5);
            }
            if (digitalRead(io) == 0)
                return true;
        }
        return false;
    }
    else
    {
        if ((digitalRead(io1) == 0) && (digitalRead(io2) == 0))
        {
            while (retry--)
            {
                if (digitalRead(io1) || digitalRead(io2))
                    return false;
                delay(5);
            }
            if ((digitalRead(io1) == 0) && (digitalRead(io2) == 0))
                return true;
        }
        return false;
    }
}

void Button::setPressedCallback(CBFunc cb)
{
    static char name[] = {"buttonX"};
    if (pinNum == 1)
        name[6] = io + '0';
    else
        name[6] = io1 + io2 + '0';
    if (pressedCb == NULL && unpressedCb == NULL)
    {
        xTaskCreatePinnedToCore(Button::taskLoop, name, 2048, this, 1, NULL, ARDUINO_RUNNING_CORE);
    }
    pressedCb = cb;
}

void Button::setUnPressedCallback(CBFunc cb)
{
    static char name[] = {"buttonX"};
    if (pinNum == 1)
        name[6] = io + '0';
    else
        name[6] = io1 + io2 + '0';
    if (unpressedCb == NULL && pressedCb == NULL)
    {
        xTaskCreatePinnedToCore(Button::taskLoop, name, 2048, this, 1, NULL, ARDUINO_RUNNING_CORE);
    }
    unpressedCb = cb;
}

void Button::taskLoop(void *param){
    Button *self = (Button *)param;
    while(1){
        bool isPressed = self->isPressed();
        if(self->prevState != isPressed){
            if(isPressed && self->pressedCb){
                self->pressedCb();
            }
            if((!isPressed) && self->unpressedCb){
                self->unpressedCb();
            }
            self->prevState = isPressed;
        }
        yield();
    }
}