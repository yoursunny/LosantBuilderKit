#include "LosantTemperature.hpp"
#include "logger.hpp"

#define LOSANT_TEMPERATURE_DBG(...) DBG(LosantTemperature, __VA_ARGS__)

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
  LOSANT_TEMPERATURE_DBG("reporting " << reading.celsius << "C, " << reading.fahrenheit << "F");

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
