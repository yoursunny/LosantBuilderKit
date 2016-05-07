#ifndef LOSANT_TEMPERATURE_HPP
#define LOSANT_TEMPERATURE_HPP

#include <Losant.h>
#include "TemperatureReader.hpp"

/**
 * \brief report temperature to Losant
 */
class LosantTemperature
{
public:
  /**
   * \param celsiusVar Losant device state variable name for celsius temperature, nullptr to disable reporting
   * \param fahrenheitVar Losant device state variable name for fahrenheit temperature, nullptr to disable reporting
   * \param reportInterval interval between reports, in millis
   */
  LosantTemperature(TemperatureReader& reader, LosantDevice& device, const char* celsiusVar, const char* fahrenheitVar, int reportInterval = 15000);

  void
  loop();

private:
  TemperatureReader& m_reader;
  LosantDevice& m_device;
  const char* const m_celsiusVar;
  const char* const m_fahrenheitVar;
  const int m_reportInterval;
  unsigned long m_lastReport;
};

#endif // LOSANT_TEMPERATURE_HPP
