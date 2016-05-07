#ifndef TEMPERATURE_READER_HPP
#define TEMPERATURE_READER_HPP

struct TemperatureReading
{
  double celsius;
  double fahrenheit;
};

/**
 * \brief read temperature and report to Losant
 *
 * TMP36 sensor is divided down by 2x, and connected on an analog pin.
 */
class TemperatureReader
{
public:
  /**
   * \param pin the analog pin
   * \param alpha the effect of new value in moving average
   */
  TemperatureReader(int pin, double alpha = 0.1);

  void
  loop();

  TemperatureReading
  getMovingAverage() const;

private:
  const int m_pin;
  const double m_alpha;
  double m_avg;
};

#endif // TEMPERATURE_READER_HPP
