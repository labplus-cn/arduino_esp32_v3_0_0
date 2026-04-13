#include <MPython.h>

void setup() {
    Serial.begin(115200);
    mPython.begin();

    mPython.display.fillScreen(GUI_Gray);
    mPython.display.drawTextCN(10, 10, "你好，世界！", GUI_White);
    mPython.display.drawText(10, 30, "Hello, World!", GUI_White);
    mPython.display.drawTextCN(10, 58, "红色文字", GUI_Red);
    mPython.display.drawTextCN(100, 58, "绿色文字", GUI_Green);
    mPython.display.drawTextCN(200, 58, "蓝色文字", GUI_Blue);
    mPython.display.drawTextCN(10, 88, "这是中文字符显示示例,红色文字，显示不换行", GUI_Red, false);
    mPython.display.drawTextCN(10, 118, "这是中文字符显示示例,hello,world.蓝色文字，显示换行", GUI_Blue);

    mPython.display.show();
}

void loop() {
}
