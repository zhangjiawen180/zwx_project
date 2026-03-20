// 定义ESP32板载LED引脚（多数开发板为GPIO2，若不亮可改GPIO13/19）
const int ledPin = 2;

// 定义SOS时间参数（基于1Hz基础频率，500ms为1个单位）
const unsigned long shortFlashDuration = 500;   // 短闪点亮时长
const unsigned long longFlashDuration = 1500;    // 长闪点亮时长
const unsigned long gapDuration = 500;           // 闪与闪之间的熄灭间隔
const unsigned long sosPauseDuration = 3000;     // 一次SOS后的长停顿

// 计时变量（必须用unsigned long，避免溢出）
unsigned long previousMillis = 0;

// 状态机枚举：定义SOS执行的不同阶段
enum SOSState {
  SHORT_FLASH,    // 短闪阶段
  LONG_FLASH,     // 长闪阶段
  GAP,            // 闪后间隔阶段
  SOS_PAUSE       // 一次SOS后的长停顿阶段
};
SOSState currentState = SHORT_FLASH;  // 初始状态为短闪

// 计数变量：记录当前闪的次数
int flashCount = 0;          // 短闪/长闪的计数
int sosSequenceStep = 0;     // SOS序列步骤：0=3短闪 1=3长闪 2=3短闪

void setup() {
  pinMode(ledPin, OUTPUT);   // 初始化LED引脚为输出
  digitalWrite(ledPin, LOW); // 初始熄灭LED
}

void loop() {
  unsigned long currentMillis = millis(); // 读取当前系统毫秒数

  // 根据当前状态执行对应逻辑（非阻塞计时核心）
  switch (currentState) {
    // --------------- 短闪阶段 ---------------
    case SHORT_FLASH:
      if (currentMillis - previousMillis >= shortFlashDuration) {
        digitalWrite(ledPin, LOW);          // 熄灭LED
        previousMillis = currentMillis;     // 更新计时起点
        currentState = GAP;                 // 切换到间隔阶段
        flashCount++;                       // 短闪计数+1
      } else {
        digitalWrite(ledPin, HIGH);         // 点亮LED（短闪）
      }
      break;

    // --------------- 长闪阶段 ---------------
    case LONG_FLASH:
      if (currentMillis - previousMillis >= longFlashDuration) {
        digitalWrite(ledPin, LOW);          // 熄灭LED
        previousMillis = currentMillis;     // 更新计时起点
        currentState = GAP;                 // 切换到间隔阶段
        flashCount++;                       // 长闪计数+1
      } else {
        digitalWrite(ledPin, HIGH);         // 点亮LED（长闪）
      }
      break;

    // --------------- 闪后间隔阶段 ---------------
    case GAP:
      if (currentMillis - previousMillis >= gapDuration) {
        previousMillis = currentMillis;     // 更新计时起点
        // 判断当前序列步骤，切换到下一个阶段
        switch (sosSequenceStep) {
          case 0: // 正在执行3短闪
            if (flashCount >= 3) { // 3次短闪完成
              flashCount = 0;      // 重置计数
              sosSequenceStep = 1; // 切换到3长闪步骤
              currentState = LONG_FLASH;
            } else { // 继续短闪
              currentState = SHORT_FLASH;
            }
            break;
          case 1: // 正在执行3长闪
            if (flashCount >= 3) { // 3次长闪完成
              flashCount = 0;      // 重置计数
              sosSequenceStep = 2; // 切换到3短闪步骤
              currentState = SHORT_FLASH;
            } else { // 继续长闪
              currentState = LONG_FLASH;
            }
            break;
          case 2: // 正在执行最后3短闪
            if (flashCount >= 3) { // 整个SOS序列完成
              flashCount = 0;      // 重置计数
              sosSequenceStep = 0; // 重置序列步骤
              currentState = SOS_PAUSE; // 切换到长停顿阶段
            } else { // 继续短闪
              currentState = SHORT_FLASH;
            }
            break;
        }
      }
      break;

    // --------------- SOS序列后长停顿阶段 ---------------
    case SOS_PAUSE:
      if (currentMillis - previousMillis >= sosPauseDuration) {
        previousMillis = currentMillis;     // 更新计时起点
        currentState = SHORT_FLASH;         // 重新开始SOS序列
      } else {
        digitalWrite(ledPin, LOW);          // 保持LED熄灭
      }
      break;
  }

  // 这里可以添加其他代码（如串口打印、传感器读取），不会被阻塞
  // Serial.println(currentMillis); // 可选：打印当前毫秒数调试
}