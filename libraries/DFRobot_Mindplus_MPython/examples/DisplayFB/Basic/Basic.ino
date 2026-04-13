#include <MPython.h>

void setup() {
    mPython.begin();

    mPython.display.fillScreen(GUI_Black);
    mPython.display.drawText(10, 10, "Hello mPython!", GUI_White);

    char buffer[32];
    sprintf(buffer, "Value: %d", 42);
    mPython.display.drawText(10, 30, buffer, GUI_White);

    mPython.display.drawLine(0, 50, 319, 50, GUI_Red);
    mPython.display.drawRect(10, 60, 100, 50, GUI_Green);
    mPython.display.fillRect(120, 60, 100, 50, GUI_Blue);
    mPython.display.drawCircle(160, 150, 30, GUI_Yellow);
    mPython.display.fillCircle(240, 150, 30, GUI_Cyan);

    mPython.display.show();
}

void loop() {
}
