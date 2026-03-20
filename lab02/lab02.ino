// 定义LED引脚（ESP32板载LED通常为GPIO2）
const int ledPin = 2;

// 定义闪烁参数：1Hz = 周期1000ms，亮/灭各500ms
const unsigned long blinkInterval = 500;  

// 记录上一次LED状态切换的时间
unsigned long previousMillis = 0;  

// 记录当前LED状态（初始为灭）
bool ledState = LOW;  

void setup() {
  // 初始化LED引脚为输出模式
  pinMode(ledPin, OUTPUT);  
}

void loop() {
  // 读取当前系统运行的毫秒数
  unsigned long currentMillis = millis();  

  // 检查是否达到切换时间（当前时间 - 上次切换时间 ≥ 间隔时间）
  if (currentMillis - previousMillis >= blinkInterval) {
    // 更新上次切换时间为当前时间（核心：重置计时起点）
    previousMillis = currentMillis;  

    // 翻转LED状态（亮→灭，灭→亮）
    ledState = !ledState;  

    // 应用新状态到LED引脚
    digitalWrite(ledPin, ledState);  
  }

  // 这里可以加其他代码（比如读取传感器），不会被闪烁逻辑阻塞
  // 示例：串口打印当前毫秒数（可选）
  // Serial.print("Current millis: ");
  // Serial.println(currentMillis);
}