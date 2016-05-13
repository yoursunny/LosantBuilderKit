#include "LosantPingPong.hpp"
#include "logger.hpp"

#define LOSANT_PINGPONG_DBG(...) DBG(LosantPingPong, __VA_ARGS__)

LosantPingPong::LosantPingPong(LosantDevice& device, int pingInterval, int pongMissThreshold)
  : m_device(device)
  , m_pingInterval(pingInterval)
  , m_pongMissThreshold(pongMissThreshold)
  , m_lastPing(millis())
  , m_hasPong(true)
  , m_nMissedPongs(0)
{
}

void
LosantPingPong::loop()
{
  if (millis() - m_lastPing < m_pingInterval) {
    return;
  }

  if (!m_device.connected()) {
    m_lastPing = millis();
    m_hasPong = true;
    m_nMissedPongs = 0;
    return;
  }

  if (m_hasPong) {
    m_nMissedPongs = 0;
  }
  else {
    ++m_nMissedPongs;
    LOSANT_PINGPONG_DBG(_DEC(m_nMissedPongs) << F(" pong missed"));
    if (m_nMissedPongs >= m_pongMissThreshold) {
      m_nMissedPongs = 0;
      LOSANT_PINGPONG_DBG(F("disconnecting LosantDevice due to too many missed pongs"));
      m_device.disconnect();
      return;
    }
  }

  LOSANT_PINGPONG_DBG(F("sending ping"));
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["act"] = "ping";
  m_device.sendState(root);

  m_lastPing = millis();
  m_hasPong = false;
}

void
LosantPingPong::handlePong(LosantCommand* cmd)
{
  LOSANT_PINGPONG_DBG(F("pong received"));
  m_hasPong = true;
}
