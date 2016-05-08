#include <ESP8266WiFi.h>
#include <Losant.h>
#include "credentials.hpp"
#include "WifiConnection.hpp"
#include "LosantConnection.hpp"
#include "Button.hpp"
#include "TemperatureReader.hpp"
#include "LosantTemperature.hpp"
#include "LosantPingPong.hpp"
#include "NdnFace.hpp"

WifiConnection g_wifi(WIFI_NETWORKS, sizeof(WIFI_NETWORKS) / sizeof(WIFI_NETWORKS[0]), 15000);
LosantConnection g_losant(g_wifi, LOSANT_DEVICE_ID, LOSANT_ACCESS_KEY, LOSANT_ACCESS_SECRET);
const int CONNECTIVITY_LED_PIN = 0; // fully lit: disconnected; dim to 3%: connected
Button<14, ButtonMode::PullUp> g_button;
TemperatureReader g_temperatureReader(A0);
LosantTemperature g_losantTemperature(g_temperatureReader, g_losant.getDevice(), "tempC", "tempF");
LosantPingPong g_losantPingPong(g_losant.getDevice());

const int NDNPING_LED_PIN = 2;
static ndn_NameComponent g_pingPrefixComps[5];
static ndn::NameLite g_pingPrefix(g_pingPrefixComps, sizeof(g_pingPrefixComps) / sizeof(g_pingPrefixComps[0]));
static uint8_t g_pktbuf[1500];
NdnFace g_face(NDN_ROUTER_HOST, NDN_ROUTER_PORT, 6363, g_pktbuf, sizeof(g_pktbuf));

void
handleCommand(LosantCommand* cmd) {
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

  ndn::InterestLite interest(NdnFace::s_nameComps, NDNFACE_NAMECOMPS_MAX, nullptr, 0, nullptr, 0);
  ndn::NameLite& name = interest.getName();
  name.set(g_pingPrefix);
  String seq(millis());
  name.append(seq.c_str());
  interest.setMustBeFresh(true);

  Serial.print("Sending ndnping request ");
  Serial.println(seq);
  g_face.sendInterest(interest);
  digitalWrite(NDNPING_LED_PIN, LOW);
}

void
processData(const ndn::DataLite& data)
{
  const ndn::NameLite& name = data.getName();
  if (g_pingPrefix.match(name) && name.size() == g_pingPrefix.size() + 1) {
    Serial.print("Received ndnping reply ");
    const ndn::BlobLite& seqBlob = name.get(g_pingPrefix.size()).getValue();
    Serial.write(seqBlob.buf(), seqBlob.size());
    Serial.println();
    digitalWrite(NDNPING_LED_PIN, HIGH);
  }
}

void
setup()
{
  Serial.begin(115200);
  delay(2000);
  Serial.println();
  Serial.println();

  pinMode(CONNECTIVITY_LED_PIN, OUTPUT);

  g_button.onDown(&buttonDown);
  g_losant.getDevice().onCommand(&handleCommand);

  pinMode(NDNPING_LED_PIN, OUTPUT);
  g_pingPrefix.clear();
  g_pingPrefix.append("ndn");
  g_pingPrefix.append("edu");
  g_pingPrefix.append("arizona");
  g_pingPrefix.append("ping");
  g_pingPrefix.append("edu");
  g_face.onData(&processData);
}

void
loop()
{
  g_wifi.loop();
  //g_losant.loop();
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
  g_face.loop();
  delay(100);
}
