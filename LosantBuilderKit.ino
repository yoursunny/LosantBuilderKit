#include <ESP8266WiFi.h>
#include <Streaming.h>
#include "credentials.hpp"
#include "Button.hpp"
#include "Led.hpp"
#include "WifiConnection.hpp"
#include "LosantConnection.hpp"
#include "TemperatureReader.hpp"
#include "LosantTemperature.hpp"
#include "NdnFace.hpp"
#include "NdnPingServer.hpp"
#include "NdnPingClient.hpp"
#include "NdnPrefixRegistration.hpp"

Led g_powerLed(0, LOW);
Led g_pingLed(g_powerLed);
Led g_connectivityLed(g_pingLed);
Button g_button(14, INPUT_PULLUP);

WifiConnection g_wifi(WIFI_NETWORKS, sizeof(WIFI_NETWORKS) / sizeof(WIFI_NETWORKS[0]), 15329);
LosantConnection g_losant(g_wifi, LOSANT_DEVICE_ID, LOSANT_ACCESS_KEY, LOSANT_ACCESS_SECRET);

TemperatureReader g_temperatureReader(A0);
LosantTemperature g_losantTemperature(g_temperatureReader, g_losant.getDevice(), "tempC", "tempF", 17088);

static uint8_t g_pktbuf[1500];
NdnFace g_face(NDN_ROUTER_HOST, NDN_ROUTER_PORT, 6363, g_pktbuf, sizeof(g_pktbuf));
static ndn_NameComponent g_inPingPrefixComps[8];
static ndn::NameLite g_inPingPrefix(g_inPingPrefixComps, 8);
NdnPingServer g_pingServer(g_face, g_inPingPrefix);
static ndn_NameComponent g_outPingPrefixComps[8];
static ndn::InterestLite g_outPingInterest(g_outPingPrefixComps, 8, nullptr, 0, nullptr, 0);
NdnPingClient g_pingClient(g_face, g_outPingInterest, 29696);
NdnPrefixRegistration g_prefixReg(g_face, NDNPREFIXREG_HTTPHOST, NDNPREFIXREG_HTTPPORT, NDNPREFIXREG_HTTPURI, 305112, 62983);

void
handleLosantCommand(LosantCommand* cmd) {
  Serial.print("Unknown command verb: ");
  Serial.println(cmd->name);
}

void
buttonDown(int, bool)
{
  Serial.println("Button pressed.");

  //StaticJsonBuffer<200> jsonBuffer;
  //JsonObject& root = jsonBuffer.createObject();
  //root["act"] = "button";
  //g_losant.getDevice().sendState(root);

  g_pingClient.ping();
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
processInterest(const ndn::InterestLite& interest)
{
  g_pingServer.processInterest(interest);
}

void
processData(const ndn::DataLite& data)
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

  WiFi.mode(WIFI_STA);
  WiFi.setSleepMode(WIFI_MODEM_SLEEP);

  g_powerLed.dim(0.03);
  g_button.onDown(&buttonDown);

  g_losant.getDevice().onCommand(&handleLosantCommand);

  ndn_parseName(g_inPingPrefix, NDN_INPING_PREFIX);
  g_pingServer.makePayload = &ndnpingMakePayload;
  ndn_parseName(g_outPingInterest.getName(), NDN_OUTPING_PREFIX);
  g_pingClient.onEvent(&ndnpingEvent);
  g_face.onInterest(&processInterest);
  g_face.onData(&processData);
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

  g_button.loop();
  g_temperatureReader.loop();
  g_losantTemperature.loop();
  g_face.loop(2);
  g_pingClient.loop();
  g_prefixReg.loop();
  delay(10);
}
