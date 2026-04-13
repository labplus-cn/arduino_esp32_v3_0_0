#include <MPython.h>

// 按行显示示例
// 行定义：起始x=10，第一行(line=0) y=10，行高=30
// 即：line 0 → y=10, line 1 → y=40, line 2 → y=70 ...

void setup() {
    Serial.begin(115200);
    mPython.begin();

    // 黑色背景（同时设置背景色，clearLine会用到）
    mPython.display.fillScreen(GUI_Black);

    // 按行显示文字
    mPython.display.drawTextCN(0, "第0行：白色", GUI_White);
    mPython.display.drawTextCN(1, "第1行：红色", GUI_Red);
    mPython.display.drawTextCN(2, "第2行：绿色", GUI_Green);
    mPython.display.drawTextCN(3, "第3行：蓝色", GUI_Blue);
    mPython.display.show();
    delay(2000);

    // 擦除第1行，重新写入新内容
    mPython.display.clearLine(1);
    mPython.display.drawTextCN(1, "第1行已更新", GUI_Yellow);
    mPython.display.show();
}

void loop() {
}
