#include <MPython.h>

void setup() {
    mPython.begin();

    // 白色背景
    mPython.display.fillScreen(GUI_White);

    // 在(10,10)处显示二维码，每个模块3像素
    mPython.display.drawQRCode(10, 10, "https://www.dfrobot.com", 3);

    mPython.display.show();
}

void loop() {
}
