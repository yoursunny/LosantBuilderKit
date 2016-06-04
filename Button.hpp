#ifndef BUTTON_HPP
#define BUTTON_HPP

#include <Arduino.h>

/**
 * \brief callback upon button down or up
 * \param pin button pin
 * \param state true=down, false=up
 */
typedef void (*ButtonCallback)(int, bool);

/**
 * \brief detect button state
 */
class Button
{
public:
  /**
   * \brief constructor
   * \param pin GPIO pin
   * \param mode INPUT: input pin is externally pulled down when button is released, and held high when button is pressed;
   *             INPUT_PULLUP: input pin is internally pulled up when button is released, and held low when button is pressed
   */
  Button(int pin, int mode);

  bool
  isDown() const
  {
    return this->isDown(digitalRead(m_pin));
  }

  void
  onDown(ButtonCallback cb)
  {
    m_down = cb;
  }

  void
  onUp(ButtonCallback cb)
  {
    m_up = cb;
  }

  void
  loop();

private:
  bool
  isDown(int state) const
  {
    return m_isPullUp == !state;
  }

private:
  int m_pin;
  bool m_isPullUp;
  int m_state;
  ButtonCallback m_down;
  ButtonCallback m_up;
};

#endif // BUTTON_HPP
