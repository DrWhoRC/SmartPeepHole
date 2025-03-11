#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>

const char* ssid = "Veronica";
const char* password = "12345678";
const char* mqtt_server = "172.20.10.2";

WiFiClient espClient;
PubSubClient client(espClient);
Servo myServo;

#define TRIG_PIN D5
#define ECHO_PIN D6
#define LED_PIN D2
#define BUTTON_PIN D3
#define BUZZER_PIN D4
#define SERVO_PIN D1

void setup() {
    Serial.begin(115200);

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(BUZZER_PIN, OUTPUT);

    myServo.attach(SERVO_PIN);

    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to WiFi");

    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    reconnectMQTT();
    client.subscribe("servo/control");
}

void loop() {
    if (!client.connected()) {
        reconnectMQTT();
    }
    client.loop();

    long distance = measureDistance();
    if (distance > 0 && distance < 30) {
        digitalWrite(LED_PIN, HIGH);
        client.publish("object/detect", "Object detected!");
        Serial.println("Object detected, LED ON");
        delay(1000);
    } else {
        digitalWrite(LED_PIN, LOW);
    }

    if (digitalRead(BUTTON_PIN) == LOW) {
        digitalWrite(BUZZER_PIN, HIGH);
        Serial.println("Doorbell Ringing!");
        delay(500);
        digitalWrite(BUZZER_PIN, LOW);
    }

    delay(100);
}

void callback(char* topic, byte* payload, unsigned int length) {
    String message = "";
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    Serial.print("Message received [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.println(message);

    if (String(topic) == "servo/control") {
        if (message == "open") {
            myServo.write(180);
            delay(2000);
            Serial.println("door opened");
        } else if (message == "lock") {
            myServo.write(0);
            Serial.println("locked");
        }
    }

    Serial.print("Current servo position: ");
    Serial.println(myServo.read());
}

void reconnectMQTT() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("NodeMCU_Client")) {
            Serial.println("Connected to MQTT broker");
            client.subscribe("servo/control");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" retrying in 5 seconds");
            delay(5000);
        }
    }
}

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
