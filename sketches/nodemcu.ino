#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Veronica";
const char* password = "12345678";
const char* mqtt_server = "172.20.10.2";

WiFiClient espClient;
PubSubClient client(espClient);

// 硬件引脚定义
#define TRIG_PIN D5
#define ECHO_PIN D6
#define LED_PIN D2
#define BUTTON_PIN D3
#define BUZZER_PIN D4

void setup() {
    Serial.begin(115200);

    // 配置引脚
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(BUZZER_PIN, OUTPUT);

    // 连接 WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to WiFi");

    // 连接 MQTT
    client.setServer(mqtt_server, 1883);
    reconnectMQTT();
}

void loop() {
    if (!client.connected()) {
        reconnectMQTT();
    }
    client.loop();

    long distance = measureDistance();
    if (distance > 0 && distance < 30) {  // 物体在 10cm 内
        digitalWrite(LED_PIN, HIGH);
        client.publish("object/detect", "Object detected!");
        Serial.println("Object detected, LED ON");
        delay(1000); // 防止多次触发
    } else {
        digitalWrite(LED_PIN, LOW);
    }

    if (digitalRead(BUTTON_PIN) == LOW) {  // 按钮按下
        digitalWrite(BUZZER_PIN, HIGH);
        Serial.println("Doorbell Ringing!");
        delay(500);
        digitalWrite(BUZZER_PIN, LOW);
    }

    delay(100);
}

// 重新连接 MQTT
void reconnectMQTT() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("NodeMCU_Client")) {
            Serial.println("Connected to MQTT broker");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" retrying in 5 seconds");
            delay(5000);
        }
    }
}

// 测量距离（超声波传感器）
long measureDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH);
    long distance = duration * 0.034 / 2;
    return distance;
}
