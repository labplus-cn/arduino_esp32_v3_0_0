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
 * - 唤醒后可选 TTS 应答：先 ttsInit()，再在 speechRecognitionBegin 第一个参数传入 UTF-8 文本。
 */

#include <MPython.h>

namespace {

// 命令 id（请按你的模型与需求调整）
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

  // 可选：唤醒后播放一句 TTS（需 voice_data 分区与 ttsInit）
  bool ttsOk = mPython.audio.ttsInit();
  if (!ttsOk) {
    Serial.println("TTS init skipped (no voice_data or failed). Wakeup reply disabled.");
  }

  // 第二个参数：MultiNet 超时(ms)；第三个：是否从 sdkconfig 加载命令（Arduino 一般为 false）
  const char *wakeupReply = ttsOk ? "在呢" : nullptr;
  if (!mPython.audio.speechRecognitionBegin(wakeupReply, 6000, false)) {
    Serial.println("speechRecognitionBegin failed (check \"model\" partition and SR models).");
    while (true) {
      delay(2000);
    }
  }

  Serial.println("Waiting for MultiNet / command list...");
  if (!mPython.audio.speechRecognitionWaitReady(20000)) {
    Serial.println("speechRecognitionWaitReady timeout.");
    mPython.audio.speechRecognitionEnd();
    while (true) {
      delay(2000);
    }
  }

  mPython.audio.speechRecognitionClearCommands();
  // mn7_cn command grammar uses pinyin-like units more reliably than raw Chinese characters.
  // Speak Chinese naturally, but register commands in pinyin tokens.
  mPython.audio.speechRecognitionAddCommand(CMD_LIGHT_ON, "kai deng");
  mPython.audio.speechRecognitionAddCommand(CMD_LIGHT_OFF, "guan deng");
  mPython.audio.speechRecognitionAddCommand(CMD_FORWARD, "qian jin");
  mPython.audio.speechRecognitionAddCommand(CMD_BACKWARD, "hou tui");

  if (!mPython.audio.speechRecognitionApplyCommands()) {
    Serial.println("speechRecognitionApplyCommands failed (phrases may be invalid for this multinet).");
    mPython.audio.speechRecognitionEnd();
    while (true) {
      delay(2000);
    }
  }

  Serial.println("SR ready: say the wake word, then a command phrase.");
}

void loop() {
  int id = mPython.audio.speechRecognitionCommandId();
  if (id != 0) {
    Serial.printf("Command recognized: id=%d (wakeup_flag=%d)\n", id,
                  mPython.audio.speechRecognitionWakeupFlag());
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
    mPython.audio.speechRecognitionResetCommandId();
  }
  delay(50);
}
