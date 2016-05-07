#include <Losant.h>

class Temperature
{
public:
  Temperature(LosantDevice& device, int reportInterval = 15000, int pin = A0);

  void
  loop();

private:
  LosantDevice& m_device;
  const int m_pin;
  const int m_reportInterval;
  unsigned long m_lastReport;
  unsigned long m_sum;
  unsigned m_count;
};
