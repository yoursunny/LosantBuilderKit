#include "pingpong.hpp"

PingPong::PingPong(LosantDevice& device, int pingInterval, int pongMissThreshold)
  : m_device(device)
  , m_pingInterval(pingInterval)
  , m_pongMissThreshold(pongMissThreshold)
  , m_lastPing(micros())
  , m_hasPong(true)
  , m_nMissedPongs(0)
{
}

void
PingPong::loop()
{
  if (millis() - m_lastPing < m_pingInterval) {
    return;
  }

  if (!m_hasPong) {
    ++m_nMissedPongs;
    Serial.print("Pong missed: ");
    Serial.println(m_nMissedPongs);
    if (m_nMissedPongs >= m_pongMissThreshold) {
      m_nMissedPongs = 0;
      Serial.println("Too many missed pongs, disconnecting");
      m_device.disconnect();
      return;
    }
  }

  Serial.println("Sending ping");
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["act"] = "ping";
  m_device.sendState(root);

  m_lastPing = millis();
  m_hasPong = false;
}

void
PingPong::handlePong(LosantCommand* cmd)
{
  Serial.println("Pong received");
  m_hasPong = true;
}
