#include <WiFi.h>
#include <PubSubClient.h>
#include "esp_camera.h"

const char* ssid = "Veronica";
const char* password = "12345678";

const char* mqtt_server = "172.20.10.2";
const char* topic_capture = "camera/capture";
const char* topic_image = "camera/image";

WiFiClient espClient;
PubSubClient client(espClient);

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

void setup_wifi() {
  Serial.print("connecting Wi-Fi ...");
  WiFi.begin(ssid, password);
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    retries++;
    if (retries > 30) {
      Serial.println("\nWi-Fi connection failed，reboot...");
      ESP.restart();
    }
  }
  Serial.println("\nWi-Fi connected！");
}

void reconnect_mqtt() {
  int retries = 0;
  while (!client.connected()) {
    Serial.print("connecting MQTT...");
    if (client.connect("ESP32-CAM")) {
      Serial.println("MQTT connected！");
      client.subscribe(topic_capture);
      return;
    } else {
      Serial.print("failed，status code: ");
      Serial.println(client.state());
      retries++;
      if (retries > 5) {
        Serial.println("MQTT connection failed，reboot ESP32...");
        ESP.restart();
      }
      delay(2000);
    }
  }
}

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
  config.frame_size = FRAMESIZE_UXGA;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("camera initialization failed！");
    return;
  }
}

void sendImageToMQTT(camera_fb_t *fb) {
    if (!client.connected()) {
        reconnect_mqtt();
    }

    Serial.println("sending starting mark of the pic...");
    client.publish(topic_image, "START", false);

    const size_t CHUNK_SIZE = 1000;
    size_t bytes_sent = 0;

    while (bytes_sent < fb->len) {
        size_t chunk_size = min(CHUNK_SIZE, fb->len - bytes_sent);
        client.publish(topic_image, fb->buf + bytes_sent, chunk_size, false);
        bytes_sent += chunk_size;
        delay(10);
    }

    Serial.println("sending ending mark of  the pic ...");
    client.publish(topic_image, "END", false);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message = String((char*)payload).substring(0, length);
  if (message == "capture") {
    Serial.println("received capture command，starting shooting...");

    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("shooting failed！");
      client.publish(topic_image, "failed");
      return;
    }

    Serial.println("shooting succeed，starting sending...");
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