#include <Losant.h>

/**
 * \brief end-to-end Losant connectivity test
 * \param pingInterval interval between pings, in millis
 * \param pongMissThreshold how many missed pongs causes device.disconnect
 *
 * Usage:
 * 1. Losant device: create state variable `act` of string type
 * 2. Losant workflow: when device reports `{{act}}=='ping'`, send a device command 'pong'
 * 3. sketch globals: declare PingPong instance
 * 4. sketch loop(): invoke PingPong::loop()
 */
class PingPong
{
public:
  PingPong(LosantDevice& device, int pingInterval = 10000, int pongMissThreshold = 6);

  /**
   * \brief send ping request to Losant, and reset if too many missed pongs
   *
   * Every ping should be responded with a pong. If not, it's counted as a missed pong.
   * When the number of missed pongs exceeds a threshold, the LosantDevice is disconnected.
   * Other code is responsible for reconnecting.
   */
  void
  loop();

  void
  handlePong(LosantCommand* cmd);

private:
  LosantDevice& m_device;
  const int m_pingInterval;
  const int m_pongMissThreshold;
  unsigned long m_lastPing;
  bool m_hasPong;
  int m_nMissedPongs;
};
