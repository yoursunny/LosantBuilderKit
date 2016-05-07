#include "TemperatureReader.hpp"
#include <Arduino.h>

TemperatureReader::TemperatureReader(int pin, double alpha)
  : m_pin(pin)
  , m_alpha(alpha)
  , m_avg(-1.0)
{
}

void
TemperatureReader::loop()
{
  double v = static_cast<double>(analogRead(m_pin));

  if (m_avg < 0.0) {
    m_avg = v;
  }
  else {
    m_avg = m_alpha * v + (1 - m_alpha) * m_avg;
  }
}

TemperatureReading
TemperatureReader::getMovingAverage() const
{
  TemperatureReading reading;
  reading.celsius = ((m_avg / 1024.0 * 2.0) - 0.57) * 100.0;
  reading.fahrenheit = reading.celsius * 1.8 + 32;
  return reading;
}
