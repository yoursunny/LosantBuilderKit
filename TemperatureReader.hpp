#ifndef TEMPERATURE_READER_HPP
#define TEMPERATURE_READER_HPP

#include <cinttypes>

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
   * \param alpha effect of new value in moving average is 1/(2^alpha)
   */
  explicit
  TemperatureReader(int alpha = 3);

  void
  loop();

  TemperatureReading
  getMovingAverage() const;

private:
  const int m_alpha;
  uint32_t m_avg;
};

#endif // TEMPERATURE_READER_HPP
