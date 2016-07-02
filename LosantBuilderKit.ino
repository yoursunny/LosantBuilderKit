#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <esp8266ndn.h>
#include <Streaming.h>
#include "credentials.hpp"
#include "Button.hpp"
#include "Led.hpp"
#include "WifiConnection.hpp"
#include "DyndnsUpdate.hpp"
#include "LosantDeviceAsync.hpp"
#include "TemperatureReader.hpp"
#include "LosantTemperature.hpp"
#include "NdnPingServer.hpp"
#include "NdnPrefixRegistration.hpp"

Led g_powerLed(0, LOW);
Led g_connectivityLed(g_powerLed);
Button g_button(14, INPUT_PULLUP);
Led g_ledR(12, LOW);
Led g_ledG(13, LOW);
Led g_ledY(15, LOW);
Led g_ledB(5, LOW);
Led g_ledW(4, LOW);

WifiConnection g_wifi(WIFI_NETWORKS, sizeof(WIFI_NETWORKS) / sizeof(WIFI_NETWORKS[0]), 15329);
DyndnsUpdate g_dyndns(DYNDNS_SERVER, DYNDNS_HOST, DYNDNS_AUTH, 1824038, 65651);
LosantDeviceAsync g_losant(LOSANT_DEVICE_ID);

TemperatureReader g_temperatureReader(2.0);
LosantTemperature g_losantTemperature(g_temperatureReader, g_losant, "tempC", "tempF", 17088);

WiFiUDP g_faceUdp;
ndn::UnicastUdpTransport g_faceTransport(g_faceUdp);
bool g_isFaceTransportInitialized = false;
ndn::Face g_face(g_faceTransport);
static ndn_NameComponent g_inPingPrefixComps[8];
static ndn::NameLite g_inPingPrefix(g_inPingPrefixComps, 8);
NdnPingServer g_pingServer(g_face, g_inPingPrefix);
static ndn_NameComponent g_outPingPrefixComps[8];
static ndn::InterestLite g_outPingInterest(g_outPingPrefixComps, 8, nullptr, 0, nullptr, 0);
ndn::PingClient g_pingClient(g_face, g_outPingInterest, 29696, 2000);
NdnPrefixRegistration g_prefixReg(g_face, NDNPREFIXREG_HTTPHOST, NDNPREFIXREG_HTTPPORT, NDNPREFIXREG_HTTPURI, 305112, 62983);

void
controlSingleLed(Led& led, JsonObject& cmdPayload, const char* objKey)
{
  if (!cmdPayload.containsKey(objKey)) {
    return;
  }

  auto value = cmdPayload[objKey];
  if (value.is<bool>()) {
    if (value.as<bool>()) {
      Serial << F("Turn on LED-") << objKey << "\n";
      led.on();
    }
    else {
      Serial << F("Turn off LED-") << objKey << "\n";
      led.off();
    }
  }
  else if (value.is<float>()) {
    Serial << F("Dim LED-") << objKey << " " << value.as<float>() << "\n";
    led.dim(value.as<float>());
  }
  else {
    Serial << F("Unset LED-") << objKey << "\n";
    led.unset();
  }
}

void
handleLedCommand(LosantCommand* cmd)
{
  if (cmd->payload == nullptr) {
    Serial << F("LED command requires a payload\n");
    return;
  }
  controlSingleLed(g_ledR, *cmd->payload, "R");
  controlSingleLed(g_ledG, *cmd->payload, "G");
  controlSingleLed(g_ledY, *cmd->payload, "Y");
  controlSingleLed(g_ledB, *cmd->payload, "B");
  controlSingleLed(g_ledW, *cmd->payload, "W");
}

void
handleLosantCommand(LosantCommand* cmd)
{
  if (strcmp(cmd->name, "led") == 0) {
    handleLedCommand(cmd);
  }
  else {
    Serial << F("Unknown command verb: ") << cmd->name << "\n";
  }
}

void
buttonDown(int, bool)
{
  //StaticJsonBuffer<200> jsonBuffer;
  //JsonObject& root = jsonBuffer.createObject();
  //root["act"] = "button";
  //g_losant.getDevice().sendState(root);
}

void
ndnpingMakePayload(PString& payload)
{
  TemperatureReading reading = g_temperatureReader.getMovingAverage();
  payload << F("yoursunny.com temperature sensor\n")
          << reading.celsius << "C " << reading.fahrenheit << "F\n"
          << WiFi.SSID() << " " << WiFi.localIP() << " " << millis() << "\n";
}

void
processInterest(void*, const ndn::InterestLite& interest, uint64_t)
{
  g_pingServer.processInterest(interest);
}

void
processData(void*, const ndn::DataLite& data, uint64_t)
{
  g_pingClient.processData(data) ||
  g_prefixReg.processData(data);
}

void
setup()
{
  Serial.begin(115200);
  delay(2000);
  Serial.println();
  Serial.println();

  g_powerLed.dim(0.03);
  g_button.onDown(&buttonDown);

  g_losant.onCommand(&handleLosantCommand);
  g_losant.setCredentials(LOSANT_ACCESS_KEY, LOSANT_ACCESS_SECRET);

  ndn::parseNameFromUri(g_inPingPrefix, NDN_INPING_PREFIX);
  g_pingServer.makePayload = &ndnpingMakePayload;
  ndn::parseNameFromUri(g_outPingInterest.getName(), NDN_OUTPING_PREFIX);
  g_face.onInterest(&processInterest, nullptr);
  g_face.onData(&processData, nullptr);
  g_face.setHmacKey(NDN_HMAC_KEY, sizeof(NDN_HMAC_KEY));
}

void
loop()
{
  g_wifi.loop();
  g_dyndns.loop();
  if (!g_wifi.isConnected()) {
    g_connectivityLed.on();
  }
  else if (!g_losant.connected()) {
    g_connectivityLed.dim(0.9);
    g_losant.connect();
  }
  else {
    g_connectivityLed.unset();
  }

  if (!g_isFaceTransportInitialized && g_wifi.isConnected()) {
    IPAddress routerIp;
    if (WiFi.hostByName(NDN_ROUTER_HOST, routerIp)) {
      g_faceTransport.begin(routerIp, NDN_ROUTER_PORT, 6363);
      g_isFaceTransportInitialized = true;
    }
  }

  g_button.loop();
  g_temperatureReader.loop();
  g_losantTemperature.loop();
  g_face.loop();
  g_pingClient.loop();
  g_prefixReg.loop();
  delay(10);
}
