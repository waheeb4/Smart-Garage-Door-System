#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>


#define PIR_PIN         14
#define REED_PIN        33
#define BUZZER_PIN      25
#define SERVO_PIN       18

const char* ssid       = "sameh";
const char* password   = "00000000";
const char* mqtt_server= "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);
Servo garageServo;

int motionCounter = 0;
unsigned long lastPirTriggerTime = 0;

void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32_GARAGE")) {
      client.subscribe("garage/control/door");
    } else {
      delay(5000);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)message[i];
  }

  if (String(topic) == "garage/control/door") {
    if (msg == "open") {
      garageServo.write(90);
      Serial.println("Open");
    } else if (msg == "close") {
      garageServo.write(0);
      Serial.println("Close");
    }
  }


}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("== Starting setup ==");

  Serial.println("Attaching servo...");
  garageServo.attach(SERVO_PIN);
  garageServo.write(0);

  pinMode(PIR_PIN, INPUT);
  pinMode(REED_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  Serial.println("Connecting to WiFi...");
  setup_wifi();
  Serial.println("WiFi connected!");

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  Serial.println("Setup done.");
}


void loop() {
  if (!client.connected()) reconnect();
  client.loop();




  bool rawPirState = digitalRead(PIR_PIN) == HIGH;
  bool doorClosed = digitalRead(REED_PIN) == LOW;
  
  if (doorClosed) {
    Serial.println("Garage Door Closed");
  } else {
    motionCounter = 0;
    Serial.println("Garage Door Open");
    rawPirState = LOW;
  }

  if (rawPirState) {
    lastPirTriggerTime = millis();
    motionCounter += 1;
    Serial.println(motionCounter);
  }

  if (!rawPirState && (millis() - lastPirTriggerTime > 10000)) {
    motionCounter = 0;
  } else if (motionCounter > 45) {
    Serial.println("Motion detected!");
    digitalWrite(BUZZER_PIN, HIGH);

    client.publish("garage/sensors/buzzer", "1");
    delay(5000);
    digitalWrite(BUZZER_PIN, LOW);
    client.publish("garage/sensors/buzzer", "0");
    motionCounter = 0;
  }
  delay(100);
}
