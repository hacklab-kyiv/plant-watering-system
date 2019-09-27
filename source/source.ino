#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// WiFi and MQTT setup
const char* ssid = "HackLab Guest";
const char* password = "";
const char* mqttServer = "m10.cloudmqtt.com";
const int mqttPort = 16860;
const char* mqttUser = "wahnfosn";
const char* mqttPassword = "8P7oll9SD0l5";

// HW setup
int sensorPowerPin = D2;
int pumpPowerPin = D1;

// Pumping settings
const int wateringPulse = 2000;
const int measurementDelay = 500;

int wateringThreshold = 500;
int smallestTimeBetweenWatering = 50;

char messageBuffer[16];
WiFiClient espClient;
PubSubClient client(espClient);


void setup() {
  pinMode(sensorPowerPin, OUTPUT);
  pinMode(pumpPowerPin, OUTPUT);
  digitalWrite(pumpPowerPin, LOW);
  Serial.begin(9600);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
 
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("WateringSystem", mqttUser, mqttPassword )) {
 
      Serial.println("connected");  
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
 
  client.subscribe("threshold");
  client.subscribe("wateringdelay");
}

void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
 
  Serial.println();
  Serial.println("-----------------------");

  payload[length] = 0;
  if (strcmp(topic, "threshold") == 0) {
    wateringThreshold = atoi((const char*) payload);
    Serial.print("Watering threshold updated: ");
    Serial.println(wateringThreshold);
  } else if (strcmp(topic, "wateringdelay") == 0)
  {
    smallestTimeBetweenWatering = atoi((const char*) payload);
    Serial.print("Delay between watering  updated: ");
    Serial.print(smallestTimeBetweenWatering);
    Serial.println(" seconds");
  }
}

int getHumidity() {
  int humidity;
  digitalWrite(sensorPowerPin, HIGH);
  delay(5);
  humidity = analogRead(A0);
  //delay(1);
  digitalWrite(sensorPowerPin, LOW);
  return 1024 - humidity;
}

int makeWaterPulse() {
  static long int lastWateringTime = 0;
  if ((millis() - lastWateringTime) > 1000 * smallestTimeBetweenWatering) {
    Serial.println("Time to water!");
    digitalWrite(pumpPowerPin, HIGH);
    delay(wateringPulse);
    digitalWrite(pumpPowerPin, LOW);
    lastWateringTime = millis();
  }
}


void loop() {
  int humidity;
  humidity = getHumidity();
  client.loop();
  Serial.println(humidity);
  client.publish("humidity", itoa(humidity, messageBuffer, 10));
  if (humidity < wateringThreshold)
    makeWaterPulse();
  else
    digitalWrite(pumpPowerPin, LOW);
  delay(measurementDelay);
}
