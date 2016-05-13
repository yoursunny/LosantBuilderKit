#include <ESP8266WiFi.h>
#include <Streaming.h>
#include "credentials.hpp"
#include "WifiConnection.hpp"
#include "LosantConnection.hpp"
#include "Button.hpp"
#include "TemperatureReader.hpp"
#include "LosantTemperature.hpp"
#include "LosantPingPong.hpp"
#include "NdnFace.hpp"
#include "NdnPingServer.hpp"
#include "NdnPingClient.hpp"
#include "NdnPrefixRegistration.hpp"

WifiConnection g_wifi(WIFI_NETWORKS, sizeof(WIFI_NETWORKS) / sizeof(WIFI_NETWORKS[0]), 15000);
LosantConnection g_losant(g_wifi, LOSANT_DEVICE_ID, LOSANT_ACCESS_KEY, LOSANT_ACCESS_SECRET);
const int CONNECTIVITY_LED_PIN = 0; // fully lit: disconnected; dim to 3%: connected
Button<14, ButtonMode::PullUp> g_button;
TemperatureReader g_temperatureReader(A0);
LosantTemperature g_losantTemperature(g_temperatureReader, g_losant.getDevice(), "tempC", "tempF");
LosantPingPong g_losantPingPong(g_losant.getDevice(), 20000, 4);

static uint8_t g_pktbuf[1500];
NdnFace g_face(NDN_ROUTER_HOST, NDN_ROUTER_PORT, 6363, g_pktbuf, sizeof(g_pktbuf));
static ndn_NameComponent g_inPingPrefixComps[8];
static ndn::NameLite g_inPingPrefix(g_inPingPrefixComps, 8);
NdnPingServer g_pingServer(g_face, g_inPingPrefix);
const int NDNPING_LED_PIN = 2;
static ndn_NameComponent g_outPingPrefixComps[8];
static ndn::InterestLite g_outPingInterest(g_outPingPrefixComps, 8, nullptr, 0, nullptr, 0);
NdnPingClient g_pingClient(g_face, g_outPingInterest, 30000, NDNPING_LED_PIN);
NdnPrefixRegistration g_prefixReg(g_face, NDNPREFIXREG_HTTPHOST, NDNPREFIXREG_HTTPPORT, NDNPREFIXREG_HTTPURI);

void
handleLosantCommand(LosantCommand* cmd) {
  if (strcmp(cmd->name, "pong") == 0) {
    g_losantPingPong.handlePong(cmd);
  }
  else {
    Serial.print("Unknown command verb: ");
    Serial.println(cmd->name);
  }
}

void
buttonDown(int, bool, unsigned long)
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
  payload << "yoursunny.com temperature sensor\n"
          << reading.celsius << "C " << reading.fahrenheit << "F\n"
          << WiFi.SSID() << " " << WiFi.localIP() << " " << millis() << "\n";
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

  g_button.onDown(&buttonDown);

  pinMode(CONNECTIVITY_LED_PIN, OUTPUT);
  g_losant.getDevice().onCommand(&handleLosantCommand);

  ndn_parseName(g_inPingPrefix, NDN_INPING_PREFIX);
  g_pingServer.makePayload = &ndnpingMakePayload;
  ndn_parseName(g_outPingInterest.getName(), NDN_OUTPING_PREFIX);
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
    analogWrite(CONNECTIVITY_LED_PIN, static_cast<int>(PWMRANGE * 0.97));
  }
  else {
    analogWrite(CONNECTIVITY_LED_PIN, 0);
  }

  g_button.loop();
  g_temperatureReader.loop();
  g_losantTemperature.loop();
  g_losantPingPong.loop();
  g_face.loop(2);
  g_pingClient.loop();
  g_prefixReg.loop();
  delay(10);
}
