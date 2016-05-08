#include "LosantTemperature.hpp"

#define LOSANT_TEMPERATURE_DBG Serial.print

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

  LOSANT_TEMPERATURE_DBG("[LosantTemperature] reporting ");
  LOSANT_TEMPERATURE_DBG(reading.celsius);
  LOSANT_TEMPERATURE_DBG("C, ");
  LOSANT_TEMPERATURE_DBG(reading.fahrenheit);
  LOSANT_TEMPERATURE_DBG("F\n");

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
