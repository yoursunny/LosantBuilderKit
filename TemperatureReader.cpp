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
  double voltage = m_avg / 1024.0;
  voltage *= 2.0; // voltage divider
  TemperatureReading reading;
  reading.celsius = 100.0 * voltage - 50.0; // https://www.adafruit.com/product/165
  reading.fahrenheit = reading.celsius * 1.8 + 32;
  return reading;
}
