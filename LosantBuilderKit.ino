#include <ESP8266WiFi.h>
#include <Losant.h>
#include "credentials.h"
#include "pingpong.hpp"

const int LOOP_INTERVAL = 100;

WiFiClientSecure wifiClient;
LosantDevice device(LOSANT_DEVICE_ID);

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

const int LED0_PIN = 0;
const int LED1_PIN = 2;
const int LED2_PIN = 12;

void setLed(JsonObject& p)
{
  Serial.println("Setting LED");
  digitalWrite(LED0_PIN, (bool)p["v"][0] ? LOW : HIGH);
  digitalWrite(LED1_PIN, (bool)p["v"][1] ? LOW : HIGH);
  digitalWrite(LED2_PIN, (bool)p["v"][2] ? HIGH : LOW);
}

const int TEMP_REPORT_INTERVAL = 15000;
int tempReportDue = TEMP_REPORT_INTERVAL;
int tempSum = 0;
int tempCount = 0;

void reportTemp(double tempC)
{
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["tempC"] = tempC;
  device.sendState(root);
}

void readTemp()
{
  tempSum += analogRead(A0);
  ++tempCount;
  tempReportDue -= LOOP_INTERVAL;
  if (tempReportDue <= 0) {
    double raw = (double)tempSum / (double)tempCount;
    double tempC = ((raw / 1024.0 * 2.0) - 0.57) * 100.0;
    Serial.print("Reporting temperature: ");
    Serial.print(tempC);
    Serial.println(" C");
    reportTemp(tempC);
    tempSum = tempCount = 0;
    tempReportDue = TEMP_REPORT_INTERVAL;
  }
}

PingPong pingPong(device);

void handleCommand(LosantCommand* cmd) {
  if (strcmp(cmd->name, "led") == 0) {
    setLed(*cmd->payload);
  }
  else if (strcmp(cmd->name, "pong") == 0) {
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
  pinMode(LED0_PIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  digitalWrite(LED0_PIN, HIGH);
  digitalWrite(LED1_PIN, HIGH);
  digitalWrite(LED2_PIN, LOW);

  device.onCommand(&handleCommand);
}

void loop()
{
  ensureConnected();
  device.loop();

  readButton();
  readTemp();
  pingPong.loop();
  delay(LOOP_INTERVAL);
}