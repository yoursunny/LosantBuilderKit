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
 * \brief how the button is wired
 */
enum class ButtonMode {
  /**
   * \brief input pin is externally pulled down when button is released, and held high when button is pressed
   */
  Normal,
  /**
   * \brief input pin is internally pulled up when button is released, and held low when button is pressed
   */
  PullUp
};

/**
 * \brief detect button state
 */
template<int PIN, ButtonMode MODE>
class Button
{
public:
  Button()
    : m_state(false)
    , m_lastChange(millis())
    , m_down(nullptr)
    , m_up(nullptr)
  {
    pinMode(PIN, MODE == ButtonMode::PullUp ? INPUT_PULLUP : INPUT);
  }

  bool
  isDown() const
  {
    return this->isDown(digitalRead(PIN));
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

    if (this->isDown(state)) {
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
  bool
  isDown(int state) const
  {
    return MODE == ButtonMode::PullUp ? !state : !!state;
  }

private:
  int m_state;
  unsigned long m_lastChange;
  ButtonCallback m_down;
  ButtonCallback m_up;
};

#endif // BUTTON_HPP
