#ifndef TEMPERATURE_READER_HPP
#define TEMPERATURE_READER_HPP

struct TemperatureReading
{
  double celsius;
  double fahrenheit;
};

/**
 * \brief read temperature from TMP36 sensor
 *
 * The sensor voltage is divided down by half.
 */
class TemperatureReader
{
public:
  /**
   * \param pin the analog pin
   * \param alpha the effect of new value in moving average
   */
  explicit
  TemperatureReader(double alpha = 0.1);

  void
  loop();

  TemperatureReading
  getMovingAverage() const;

private:
  const double m_alpha;
  double m_avg;
};

#endif // TEMPERATURE_READER_HPP
