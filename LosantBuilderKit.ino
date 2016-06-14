#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <esp8266ndn.h>
#include <Streaming.h>
extern "C" {
#include "user_interface.h"
}
#include "credentials.hpp"
#include "Button.hpp"
#include "Led.hpp"
#include "WifiConnection.hpp"
#include "LosantConnection.hpp"
#include "TemperatureReader.hpp"
#include "LosantTemperature.hpp"
#include "NdnPingServer.hpp"
#include "NdnPingClient.hpp"
#include "NdnPrefixRegistration.hpp"

Led g_powerLed(0, LOW);
Led g_pingLed(g_powerLed);
Led g_connectivityLed(g_pingLed);
Button g_button(14, INPUT_PULLUP);
Led g_ledR(12, LOW);
Led g_ledG(13, LOW);
Led g_ledY(15, LOW);
Led g_ledB(5, LOW);
Led g_ledW(4, LOW);

WifiConnection g_wifi(WIFI_NETWORKS, sizeof(WIFI_NETWORKS) / sizeof(WIFI_NETWORKS[0]), 15329);
LosantConnection g_losant(g_wifi, LOSANT_DEVICE_ID, LOSANT_ACCESS_KEY, LOSANT_ACCESS_SECRET);

TemperatureReader g_temperatureReader(A0);
LosantTemperature g_losantTemperature(g_temperatureReader, g_losant.getDevice(), "tempC", "tempF", 17088);

WiFiUDP g_faceUdp;
ndn::UnicastUdpTransport g_faceTransport(g_faceUdp);
bool g_isFaceTransportInitialized = false;
ndn::Face g_face(g_faceTransport);
static ndn_NameComponent g_inPingPrefixComps[8];
static ndn::NameLite g_inPingPrefix(g_inPingPrefixComps, 8);
NdnPingServer g_pingServer(g_face, g_inPingPrefix);
static ndn_NameComponent g_outPingPrefixComps[8];
static ndn::InterestLite g_outPingInterest(g_outPingPrefixComps, 8, nullptr, 0, nullptr, 0);
NdnPingClient g_pingClient(g_face, g_outPingInterest, 29696);
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

  g_pingClient.ping();
}

void
ndn_parseName(ndn::NameLite& name, char* uri)
{
  name.clear();
  char* token = strtok(uri, "/");
  while (token != nullptr) {
    name.append(token);
    token = strtok(nullptr, "/");
  }
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
ndnpingEvent(NdnPingEvent evt, uint64_t seq)
{
  switch (evt) {
    case NdnPingEvent::PROBE:
      g_pingLed.dim(0.60);
      break;
    default:
      g_pingLed.unset();
      break;
  }
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

  uint8_t mac[6];
  mac[0] = 0x02;
  for (int i = 1; i < 6; ++i) {
    mac[i] = random(0x00, 0xFF);
  }
  wifi_set_macaddr(STATION_IF, mac);

  g_powerLed.dim(0.03);
  g_button.onDown(&buttonDown);

  g_losant.getDevice().onCommand(&handleLosantCommand);

  ndn_parseName(g_inPingPrefix, NDN_INPING_PREFIX);
  g_pingServer.makePayload = &ndnpingMakePayload;
  ndn_parseName(g_outPingInterest.getName(), NDN_OUTPING_PREFIX);
  g_pingClient.onEvent(&ndnpingEvent);
  g_face.onInterest(&processInterest, nullptr);
  g_face.onData(&processData, nullptr);
  g_face.setHmacKey(NDN_HMAC_KEY, sizeof(NDN_HMAC_KEY));
}

void
loop()
{
  g_wifi.loop();
  g_losant.loop();
  if (g_wifi.isConnected() && g_losant.isConnected()) {
    g_connectivityLed.unset();
  }
  else {
    g_connectivityLed.on();
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
