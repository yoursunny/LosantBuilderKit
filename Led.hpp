#ifndef LED_HPP
#define LED_HPP

#include <Arduino.h>

/**
 * \brief control LED
 */
class Led
{
public:
  /**
   * \brief constructor
   * \param pin GPIO pin
   * \param mode HIGH: output high turns on LED;
   *             LOW: output low turns on LED
   */
  Led(int pin, int mode);

  /**
   * \brief chained control constructor
   * \param next LED controller with lower priority
   *
   * Multiple Led instances can be chained together.
   * The value set at highest priority instance wins.
   */
  Led(Led& next);

  void
  on()
  {
    m_state = State::ON;
    this->display();
  }

  void
  off()
  {
    m_state = State::OFF;
    this->display();
  }

  void
  dim(float level)
  {
    m_state = State::DIM;
    m_dim = level;
    this->display();
  }

  void
  unset()
  {
    m_state = State::UNSET;
    this->display();
  }

private:
  void
  display();

private:
  Led* m_next;
  bool m_isOverride; ///< true if higher priority instance is controlling LED

  byte m_pin;
  bool m_isLowMode;

  enum class State : byte {
    UNSET,
    OFF,
    ON,
    DIM
  };
  State m_state;
  float m_dim;
};

#endif // LED_HPP
