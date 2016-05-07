#ifndef BUTTON_HPP
#define BUTTON_HPP

#include <Arduino.h>

/**
 * \brief callback upon button down or up
 * \param pin button pin
 * \param state true=down, false=up
 * \param sinceLastChange duration since last state change, in millis
 */
typedef void (*ButtonCallback)(int, bool, unsigned long);

/**
 * \brief detect button state
 */
template<int PIN>
class Button
{
public:
  Button()
    : m_state(false)
    , m_lastChange(millis())
    , m_down(nullptr)
    , m_up(nullptr)
  {
  }

  void
  setup()
  {
    pinMode(PIN, INPUT);
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
  loop()
  {
    int state = digitalRead(PIN);
    if (state == m_state) {
      return;
    }

    unsigned long now = millis();
    unsigned long sinceLastChange = now - m_lastChange;

    if (state) {
      if (m_down != nullptr) {
        m_down(PIN, true, sinceLastChange);
      }
    }
    else {
      if (m_up != nullptr) {
        m_up(PIN, false, sinceLastChange);
      }
    }
    m_state = state;
    m_lastChange = now;
  }

private:
  int m_state;
  unsigned long m_lastChange;
  ButtonCallback m_down;
  ButtonCallback m_up;
};

#endif // BUTTON_HPP
