#include "LosantTemperature.hpp"

LosantTemperature::LosantTemperature(TemperatureReader& reader, LosantDevice& device, const char* celsiusVar, const char* fahrenheitVar, int reportInterval)
  : m_reader(reader)
  , m_device(device)
  , m_celsiusVar(celsiusVar)
  , m_fahrenheitVar(fahrenheitVar)
  , m_reportInterval(reportInterval)
  , m_lastReport(millis())
{
}

void
LosantTemperature::loop()
{
  if (!m_device.connected() || millis() - m_lastReport < m_reportInterval) {
    return;
  }

  TemperatureReading reading = m_reader.getMovingAverage();

  Serial.print("Reporting temperature: ");
  Serial.print(reading.celsius);
  Serial.print(" C, ");
  Serial.print(reading.fahrenheit);
  Serial.println(" F");

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  if (m_celsiusVar != nullptr) {
    root[m_celsiusVar] = reading.celsius;
  }
  if (m_fahrenheitVar != nullptr) {
    root[m_fahrenheitVar] = reading.fahrenheit;
  }
  m_device.sendState(root);

  m_lastReport = millis();
}
