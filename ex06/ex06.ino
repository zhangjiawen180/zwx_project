// 定义两个LED引脚（ESP32任意支持PWM的GPIO都可以）
#define LED_A 2    // 第一个LED（可接板载LED）
#define LED_B 4    // 第二个LED（必须外接LED+限流电阻）

// 呼吸灯参数
int brightnessA = 0;   // LEDA 亮度
int brightnessB = 255; // LEDB 亮度（初始反向）
int step = 3;          // 渐变步长（数值越小越平滑）
bool fadeUp = true;    // 方向标志

void setup() {
  Serial.begin(115200);
  
  // 设置两个引脚为输出模式
  pinMode(LED_A, OUTPUT);
  pinMode(LED_B, OUTPUT);

  Serial.println("=== 双通道 PWM 反相警车双闪灯 ===");
}

void loop() {
  // ==============================
  // 核心逻辑：两个灯 反向同步渐变
  // ==============================
  if (fadeUp) {
    // LEDA 变亮，LEDB 变暗
    brightnessA += step;
    brightnessB -= step;

    // 到达最亮 → 切换方向
    if (brightnessA >= 255) {
      brightnessA = 255;
      brightnessB = 0;
      fadeUp = false;
    }
  } else {
    // LEDA 变暗，LEDB 变亮
    brightnessA -= step;
    brightnessB += step;

    // 到达最暗 → 切换方向
    if (brightnessA <= 0) {
      brightnessA = 0;
      brightnessB = 255;
      fadeUp = true;
    }
  }

  // 输出 PWM 亮度
  analogWrite(LED_A, brightnessA);
  analogWrite(LED_B, brightnessB);

  // 控制平滑度（越小越丝滑）
  delay(12);
}