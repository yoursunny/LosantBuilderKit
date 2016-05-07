#include <ESP8266WiFi.h>
#include <Losant.h>
#include "credentials.h"
#include "Button.hpp"
#include "TemperatureReader.hpp"
#include "LosantTemperature.hpp"
#include "LosantPingPong.hpp"

WiFiClientSecure g_wifiClient;
LosantDevice g_losantDevice(LOSANT_DEVICE_ID);

// fully lit: disconnected; dim to 3%: connected
const int CONNECTIVITY_LED_PIN = 0;

bool
connect()
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
  g_losantDevice.connectSecure(g_wifiClient, LOSANT_ACCESS_KEY, LOSANT_ACCESS_SECRET);

  int losantAttempts = 0;
  while (!g_losantDevice.connected()) {
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

void
ensureConnected()
{
  bool needReconnect = false;
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected");
    needReconnect = true;
  }
  if (!g_losantDevice.connected()) {
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

Button<14> g_button;

void
buttonDown(int, bool, unsigned long)
{
  Serial.println("Button pressed.");

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["act"] = "button";
  g_losantDevice.sendState(root);
}

TemperatureReader g_temperatureReader(A0);
LosantTemperature g_losantTemperature(g_temperatureReader, g_losantDevice, "tempC", "tempF");
LosantPingPong g_losantPingPong(g_losantDevice);

void handleCommand(LosantCommand* cmd) {
  if (strcmp(cmd->name, "pong") == 0) {
    g_losantPingPong.handlePong(cmd);
  }
  else {
    Serial.print("Unknown command verb: ");
    Serial.println(cmd->name);
  }
}

void
setup()
{
  Serial.begin(115200);
  delay(2000);

  Serial.println();

  pinMode(CONNECTIVITY_LED_PIN, OUTPUT);

  g_button.onDown(&buttonDown);

  g_losantDevice.onCommand(&handleCommand);
}

void
loop()
{
  ensureConnected();
  g_losantDevice.loop();
  g_button.loop();
  g_temperatureReader.loop();
  g_losantTemperature.loop();
  g_losantPingPong.loop();
  delay(100);
}
