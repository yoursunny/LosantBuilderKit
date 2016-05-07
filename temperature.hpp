#include <Losant.h>

/**
 * \brief read temperature and report to Losant
 *
 * TMP36 sensor is divided down by 2x, and connected on an analog pin.
 */
class Temperature
{
public:
  /**
   * \param celsiusVar Losant device state variable name for celsius temperature, nullptr to disable reporting
   * \param fahrenheitVar Losant device state variable name for fahrenheit temperature, nullptr to disable reporting
   * \param reportInterval interval between reports, in millis
   * \param pin the analog pin
   */
  Temperature(LosantDevice& device, const char* celsiusVar, const char* fahrenheitVar, int reportInterval = 15000, int pin = A0);

  void
  loop();

private:
  /**
   * \brief compute temperature from averaged readings, and reset readings
   * \return celsius temperature, fahrenheit temperature
   */
  std::pair<double, double>
  computeTemps();

  void
  sendReport(double tempC, double tempF);

private:
  LosantDevice& m_device;
  const char* const m_celsiusVar;
  const char* const m_fahrenheitVar;
  const int m_pin;
  const int m_reportInterval;
  unsigned long m_lastReport;
  unsigned long m_sum;
  unsigned m_count;
};
