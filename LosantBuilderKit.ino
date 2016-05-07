#include <ESP8266WiFi.h>
#include <Losant.h>
#include "credentials.h"
#include "TemperatureReading.hpp"
#include "LosantPingPong.hpp"

WiFiClientSecure wifiClient;
LosantDevice device(LOSANT_DEVICE_ID);

// fully lit: disconnected; dim to 3%: connected
const int CONNECTIVITY_LED_PIN = 0;

bool connect()
{
  Serial.println();
  Serial.println();
  Serial.print("Connecting to WiFi ");
  Serial.println(WIFI_SSID);

  int wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print(".");
    delay(500);
    if (++wifiAttempts > 120) {
      Serial.println();
      Serial.println("WiFi not connected after 60 seconds");
      return false;
    }
  }

  Serial.println();
  Serial.print("WiFi connected, local IP is ");
  Serial.println(WiFi.localIP());

  Serial.println();
  Serial.println("Connecting to Losant");
  device.connectSecure(wifiClient, LOSANT_ACCESS_KEY, LOSANT_ACCESS_SECRET);

  int losantAttempts = 0;
  while (!device.connected()) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println();
      Serial.println("WiFi connection lost");
      return false;
    }
    Serial.print(".");
    delay(500);
    if (++losantAttempts > 120) {
      Serial.println();
      Serial.println("Losant not connected after 60 seconds");
      return false;
    }
  }

  Serial.println();
  Serial.println("Connected to Losant");
  Serial.println();
  Serial.println();

  analogWrite(CONNECTIVITY_LED_PIN, static_cast<int>(PWMRANGE * 0.97));
  return true;
}

void ensureConnected()
{
  bool needReconnect = false;
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected");
    needReconnect = true;
  }
  if (!device.connected()) {
    Serial.println("Losant disconnected");
    needReconnect = true;
  }
  while (needReconnect) {
    analogWrite(CONNECTIVITY_LED_PIN, 0);
    digitalWrite(CONNECTIVITY_LED_PIN, LOW);
    WiFi.disconnect();
    delay(4000);
    needReconnect = !connect();
  }
}

const int BUTTON_PIN = 14;
int buttonState = 0;

void buttonPressed()
{
  Serial.println("Button pressed.");
  Serial.println();

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["act"] = "button";
  device.sendState(root);
}

void readButton()
{
  int currentRead = digitalRead(BUTTON_PIN);
  if (currentRead != buttonState) {
    buttonState = currentRead;
    if (buttonState) {
      buttonPressed();
    }
  }
}

TemperatureReading temperature(device, "tempC", "tempF");
LosantPingPong pingPong(device);

void handleCommand(LosantCommand* cmd) {
  if (strcmp(cmd->name, "pong") == 0) {
    pingPong.handlePong(cmd);
  }
  else {
    Serial.print("Unknown command verb: ");
    Serial.println(cmd->name);
  }
}

void setup()
{
  Serial.begin(115200);
  delay(2000);

  Serial.println();

  pinMode(BUTTON_PIN, INPUT);
  pinMode(CONNECTIVITY_LED_PIN, OUTPUT);

  device.onCommand(&handleCommand);
}

void loop()
{
  ensureConnected();
  device.loop();

  readButton();

  temperature.loop();
  pingPong.loop();
  delay(100);
}
