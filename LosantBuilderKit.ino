#include <ESP8266WiFi.h>
#include <Losant.h>
#include "credentials.h"
#include "WifiConnection.hpp"
#include "LosantConnection.hpp"
#include "Button.hpp"
#include "TemperatureReader.hpp"
#include "LosantTemperature.hpp"
#include "LosantPingPong.hpp"

WifiConnection g_wifi(WIFI_SSID, WIFI_PASS);
LosantConnection g_losant(g_wifi, LOSANT_DEVICE_ID, LOSANT_ACCESS_KEY, LOSANT_ACCESS_SECRET);
const int CONNECTIVITY_LED_PIN = 0; // fully lit: disconnected; dim to 3%: connected
Button<14> g_button;
TemperatureReader g_temperatureReader(A0);
LosantTemperature g_losantTemperature(g_temperatureReader, g_losant.getDevice(), "tempC", "tempF");
LosantPingPong g_losantPingPong(g_losant.getDevice());

void
buttonDown(int, bool, unsigned long)
{
  Serial.println("Button pressed.");

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["act"] = "button";
  g_losant.getDevice().sendState(root);
}

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
  g_losant.getDevice().onCommand(&handleCommand);
}

void
loop()
{
  g_wifi.loop();
  g_losant.loop();
  if (g_wifi.isConnected() && g_losant.isConnected()) {
    analogWrite(CONNECTIVITY_LED_PIN, static_cast<int>(PWMRANGE * 0.97));
  }
  else {
    analogWrite(CONNECTIVITY_LED_PIN, 0);
  }

  g_button.loop();
  g_temperatureReader.loop();
  g_losantTemperature.loop();
  g_losantPingPong.loop();
  delay(100);
}
