#include <WiFi.h>
#include <PubSubClient.h>
#include "esp_camera.h"

// Wi-Fi 连接信息
const char* ssid = "Veronica";
const char* password = "12345678";

// MQTT 服务器
const char* mqtt_server = "172.20.10.2";
const char* topic_capture = "camera/capture";
const char* topic_image = "camera/image";

// MQTT 客户端
WiFiClient espClient;
PubSubClient client(espClient);

// 摄像头引脚配置
#define PWDN_GPIO_NUM    32
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM     0
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27
#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      21
#define Y4_GPIO_NUM      19
#define Y3_GPIO_NUM      18
#define Y2_GPIO_NUM       5
#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22

// 连接 Wi-Fi
void setup_wifi() {
  Serial.print("📶 连接 Wi-Fi 中...");
  WiFi.begin(ssid, password);
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    retries++;
    if (retries > 30) { // 15秒超时
      Serial.println("\n❌ Wi-Fi 连接失败，重启...");
      ESP.restart();
    }
  }
  Serial.println("\n✅ Wi-Fi 连接成功！");
}

// 连接 MQTT 服务器
void reconnect_mqtt() {
  int retries = 0;
  while (!client.connected()) {
    Serial.print("🔗 连接 MQTT...");
    if (client.connect("ESP32-CAM")) {
      Serial.println("✅ MQTT 连接成功！");
      client.subscribe(topic_capture);
      return;
    } else {
      Serial.print("❌ 失败，状态码: ");
      Serial.println(client.state());
      retries++;
      if (retries > 5) {
        Serial.println("🔄 MQTT 连接失败，重启 ESP32...");
        ESP.restart();
      }
      delay(2000);
    }
  }
}

// 摄像头初始化
void setup_camera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 8;
  config.fb_count = 1;
  config.frame_size = FRAMESIZE_UXGA; // 1280x1024 (更清晰)

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("📷 摄像头初始化失败！");
    return;
  }
}

// 发送图片数据到 MQTT
void sendImageToMQTT(camera_fb_t *fb) {
    if (!client.connected()) {
        reconnect_mqtt();
    }

    Serial.println("📡 发送图片起始标志...");
    client.publish(topic_image, "START", false);

    const size_t CHUNK_SIZE = 1000;  // 每次最多发送 1000 字节
    size_t bytes_sent = 0;

    while (bytes_sent < fb->len) {
        size_t chunk_size = min(CHUNK_SIZE, fb->len - bytes_sent);
        client.publish(topic_image, fb->buf + bytes_sent, chunk_size, false);
        bytes_sent += chunk_size;
        delay(10);  // 确保 MQTT 服务器有时间处理
    }

    Serial.println("📡 发送图片结束标志...");
    client.publish(topic_image, "END", false);
}

// 处理 MQTT 消息
void callback(char* topic, byte* payload, unsigned int length) {
  String message = String((char*)payload).substring(0, length);
  if (message == "capture") {
    Serial.println("📸 收到拍照指令，开始拍照...");

    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("❌ 拍照失败！");
      client.publish(topic_image, "failed");
      return;
    }

    Serial.println("📷 拍照成功，开始分块发送...");
    sendImageToMQTT(fb);
    esp_camera_fb_return(fb);
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  setup_camera();
  reconnect_mqtt();
}

void loop() {
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();
}