#include "temperature.hpp"

Temperature::Temperature(LosantDevice& device, int reportInterval, int pin)
  : m_device(device)
  , m_pin(pin)
  , m_reportInterval(reportInterval)
  , m_lastReport(millis())
  , m_sum(0)
  , m_count(0)
{
}

void
Temperature::loop()
{
  m_sum += analogRead(m_pin);
  ++m_count;

  if (millis() - m_lastReport < m_reportInterval) {
    return;
  }

  double raw = static_cast<double>(m_sum) / static_cast<double>(m_count);
  double tempC = ((raw / 1024.0 * 2.0) - 0.57) * 100.0;

  Serial.print("Reporting temperature: ");
  Serial.print(tempC);
  Serial.println(" C");

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["tempC"] = tempC;
  m_device.sendState(root);

  m_sum = 0;
  m_count = 0;
  m_lastReport = millis();
}
