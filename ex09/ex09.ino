#include <WiFi.h>
#include <WebServer.h>

// ========== 配置 ==========
const char* ap_ssid = "ESP32-Dashboard";
const char* ap_pass = "12345678";

const int LED_PIN = 2;
const int TOUCH_PIN = T0; // 触摸引脚：GPIO4
// ===================================

bool isArmed = false;
bool isAlarming = false;

WebServer server(80);

// 1. 主仪表盘页面
String getDashboardPage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>实时触摸仪表盘</title>
  <style>
    body{text-align:center;font-family:Arial;margin-top:30px;}
    .value{font-size:80px;font-weight:bold;color:#2196F3;margin:40px 0;}
    .status{font-size:24px;margin:20px 0;}
    button{padding:18px 35px;font-size:20px;margin:10px;border:none;border-radius:8px;}
    .arm{background:#0d6efd;color:white;}
    .disarm{background:#dc3545;color:white;}
    .alarm{color:red;font-weight:bold;animation:blink 0.3s infinite;}
    @keyframes blink{50%{opacity:0;}}
  </style>
</head>
<body>
  <h1>📡 ESP32 实时触摸传感器仪表盘</h1>
  
  <div class="status">系统状态：<span id="state">待机</span></div>
  <div class="value" id="touchValue">0</div>
  
  <button class="arm" onclick="arm()">布防 (Arm)</button>
  <button class="disarm" onclick="disarm()">撤防 (Disarm)</button>

  <script>
    // AJAX 获取实时触摸值
    function updateValue() {
      fetch('/data').then(res=>res.text()).then(val=>{
        document.getElementById('touchValue').innerText = val;
      });
    }

    // 更新系统状态
    function updateStatus() {
      fetch('/status').then(res=>res.text()).then(s=>{
        let el = document.getElementById('state');
        el.innerText = s;
        el.className = s.includes("报警") ? "alarm" : "";
      });
    }

    // 定时刷新（100ms 一次）
    setInterval(updateValue, 100);
    setInterval(updateStatus, 300);

    // 布防 / 撤防
    function arm() { fetch('/arm'); }
    function disarm() { fetch('/disarm'); }
  </script>
</body>
</html>
  )rawliteral";
  return html;
}

// 2. 返回触摸传感器数值（AJAX 用）
void handleTouchData() {
  int val = touchRead(TOUCH_PIN);
  server.send(200, "text/plain", String(val));
}

// 3. 返回系统状态（布防/报警）
void handleStatus() {
  String s;
  if (isAlarming) s = "⚠ 报警中！";
  else if (isArmed) s = "✅ 已布防";
  else s = "待机";
  server.send(200, "text/plain", s);
}

// 4. 布防
void handleArm() {
  isArmed = true;
  isAlarming = false;
  digitalWrite(LED_PIN, LOW);
  server.send(200, "text/plain", "OK");
}

// 5. 撤防
void handleDisarm() {
  isArmed = false;
  isAlarming = false;
  digitalWrite(LED_PIN, LOW);
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // AP 热点
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_pass);

  Serial.println("==== 仪表盘启动 ====");
  Serial.print("WiFi: "); Serial.println(ap_ssid);
  Serial.print("访问: http://"); Serial.println(WiFi.softAPIP());

  // 路由
  server.on("/", [](){ server.send(200, "text/html; charset=utf-8", getDashboardPage()); });
  server.on("/data", handleTouchData);   // 实时数据接口
  server.on("/status", handleStatus);    // 状态接口
  server.on("/arm", handleArm);
  server.on("/disarm", handleDisarm);
  
  server.begin();
}

void loop() {
  server.handleClient();

  // 高频报警闪烁（非阻塞）
  static unsigned long lastFlash = 0;
  static bool ledState = false;

  // 布防 → 触摸值低于阈值 → 报警
  if (isArmed && !isAlarming) {
    int val = touchRead(TOUCH_PIN);
    if (val < 40) { // 触摸触发阈值
      isAlarming = true;
    }
  }

  // 报警：高频快闪
  if (isAlarming) {
    if (millis() - lastFlash >= 30) {
      lastFlash = millis();
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
    }
  }
}