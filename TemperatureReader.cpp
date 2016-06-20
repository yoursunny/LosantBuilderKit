#include "TemperatureReader.hpp"
#include <limits>
#include <Arduino.h>

static const int ANALOG_BITS = 10;
static constexpr int LSHIFT_BITS = std::numeric_limits<uint32_t>::digits - ANALOG_BITS;
#pragma push_macro("max")
#undef max
static const uint32_t NO_READING = std::numeric_limits<uint32_t>::max();
#pragma pop_macro("max")

TemperatureReader::TemperatureReader(float voltageDivider, int alpha)
  : m_voltageDivider(voltageDivider)
  , m_alpha(alpha)
  , m_avg(NO_READING)
{
}

void
TemperatureReader::loop()
{
  uint32_t v = static_cast<uint32_t>(analogRead(A0)) << LSHIFT_BITS;

  if (m_avg == NO_READING) {
    m_avg = v;
  }
  else {
    m_avg -= m_avg >> m_alpha;
    m_avg += v >> m_alpha;
  }
}

TemperatureReading
TemperatureReader::getMovingAverage() const
{
  double voltage = (m_avg >> LSHIFT_BITS) / 1024.0;
  voltage *= m_voltageDivider;
  TemperatureReading reading;
  reading.celsius = 100.0 * voltage - 50.0; // https://www.adafruit.com/product/165
  reading.fahrenheit = reading.celsius * 1.8 + 32;
  return reading;
}
