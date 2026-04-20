/**
 * SpeechRecognition — 离线语音识别（WakeNet + MultiNet，esp-sr）
 *
 * 前置条件：
 * - 固件分区表中包含 label 为 "model" 的分区，并已烧录与板卡匹配的 esp-sr 模型
 *   （含中文 MultiNet 与唤醒模型）。
 * - 本示例中的「命令词」须改为你所用 MultiNet 模型支持的说法，否则无法识别。
 *
 * 说明：
 * - 与 startRecord() 不能同时使用；识别运行期间串口会打印识别到的 command id。
 * - 唤醒后可选 TTS 应答：先 ttsInit()，再在 srBegin 第一个参数传入 UTF-8 文本。
 */

#include <MPython.h>

namespace {

enum SpeechCommandId : int {
  CMD_LIGHT_ON = 1,
  CMD_LIGHT_OFF = 2,
  CMD_FORWARD = 3,
  CMD_BACKWARD = 4,
};

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("=== SpeechRecognition Example ===");

  mPython.begin();

  bool ttsOk = mPython.audio.ttsInit();
  if (!ttsOk) {
    Serial.println("TTS init skipped (no voice_data or failed). Wakeup reply disabled.");
  }

  if (!mPython.audio.srBegin("在呢", 6000, false)) {
    Serial.println("srBegin failed (check \"model\" partition and SR models).");
    while (true) {
      delay(2000);
    }
  }

  mPython.audio.srClearCommands();
  mPython.audio.srAddCommand(CMD_LIGHT_ON, "kai deng");
  mPython.audio.srAddCommand(CMD_LIGHT_OFF, "guan deng");
  mPython.audio.srAddCommand(CMD_FORWARD, "qian jin");
  mPython.audio.srAddCommand(CMD_BACKWARD, "hou tui");

  if (!mPython.audio.srApplyCommands()) {
    Serial.println("srApplyCommands failed (phrases may be invalid for this multinet).");
    mPython.audio.srEnd();
    while (true) {
      delay(2000);
    }
  }

  Serial.println("SR ready: say the wake word, then a command phrase.");
}

void loop() {
  int id = mPython.audio.srGetCommandId();
  if (id != 0) {
    Serial.printf("Command recognized: id=%d\n", id);
    switch (id) {
      case CMD_LIGHT_ON:
        Serial.println("Action: light on");
        break;
      case CMD_LIGHT_OFF:
        Serial.println("Action: light off");
        break;
      case CMD_FORWARD:
        Serial.println("Action: forward");
        break;
      case CMD_BACKWARD:
        Serial.println("Action: backward");
        break;
      default:
        Serial.println("Action: unknown id");
        break;
    }
  }
  delay(50);
}
