#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include "DHT.h"

#define DHTPIN 4
#define DHTTYPE DHT22     
#define LED_GREEN 18
#define LED_RED 19
#define SERVO_PIN 23
#define BUZZER_PIN 21

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);
Servo fanServo;

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void reconnect() {
  while (!client.connected()) {
    String clientId = "ESP32_SIC8_Client_";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  fanServo.attach(SERVO_PIN);
  fanServo.write(0);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    delay(2000);
    return;
  }

  String statusKipas = "";
  String statusAlarm = "";

  if (temperature < 30.0) {
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    fanServo.write(0);
    statusKipas = "OFF";
    statusAlarm = "OFF";
  } else {
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    fanServo.write(90);
    statusKipas = "ON";
    statusAlarm = "ON";
  }

  String payload = "{\"temp\":" + String(temperature) + 
                   ",\"humi\":" + String(humidity) + 
                   ",\"fan\":\"" + statusKipas + "\"" +
                   ",\"alarm\":\"" + statusAlarm + "\"}";

  client.publish("sic8/iot/temperature_monitoring", payload.c_str());
  delay(2000);
}
