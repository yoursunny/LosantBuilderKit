#include "TemperatureReading.hpp"

TemperatureReading::TemperatureReading(LosantDevice& device, const char* celsiusVar, const char* fahrenheitVar, int reportInterval, int pin)
  : m_device(device)
  , m_celsiusVar(celsiusVar)
  , m_fahrenheitVar(fahrenheitVar)
  , m_pin(pin)
  , m_reportInterval(reportInterval)
  , m_lastReport(millis())
  , m_sum(0)
  , m_count(0)
{
}

void
TemperatureReading::loop()
{
  m_sum += analogRead(m_pin);
  ++m_count;

  if (millis() - m_lastReport < m_reportInterval) {
    return;
  }

  double tempC, tempF;
  std::tie(tempC, tempF) = this->computeTemps();
  this->sendReport(tempC, tempF);
}

std::pair<double, double>
TemperatureReading::computeTemps()
{
  double raw = static_cast<double>(m_sum) / static_cast<double>(m_count);
  double tempC = ((raw / 1024.0 * 2.0) - 0.57) * 100.0;
  double tempF = tempC * 1.8 + 32;

  m_sum = 0;
  m_count = 0;

  return std::make_pair(tempC, tempF);
}

void
TemperatureReading::sendReport(double tempC, double tempF)
{
  Serial.print("Reporting temperature: ");
  Serial.print(tempC);
  Serial.print(" C, ");
  Serial.print(tempF);
  Serial.println(" F");

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  if (m_celsiusVar != nullptr) {
    root[m_celsiusVar] = tempC;
  }
  if (m_fahrenheitVar != nullptr) {
    root[m_fahrenheitVar] = tempF;
  }
  m_device.sendState(root);

  m_lastReport = millis();
}
