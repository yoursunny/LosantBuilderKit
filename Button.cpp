#include "Button.hpp"
#include "logger.hpp"
#include <assert.h>

#define BUTTON_DBG(...) DBG(Button, __VA_ARGS__)

Button::Button(int pin, int mode)
  : m_pin(pin)
  , m_isPullUp(mode == INPUT_PULLUP)
  , m_state(false)
  , m_down(nullptr)
  , m_up(nullptr)
{
  assert(mode == INPUT || mode == INPUT_PULLUP);
  pinMode(pin, mode);
}

void
Button::loop()
{
  int state = digitalRead(m_pin);
  if (state == m_state) {
    return;
  }
  m_state = state;

  if (this->isDown(state)) {
    BUTTON_DBG(m_pin << F(" is down"));
    if (m_down != nullptr) {
      m_down(m_pin, true);
    }
  }
  else {
    BUTTON_DBG(m_pin << F(" is up"));
    if (m_up != nullptr) {
      m_up(m_pin, false);
    }
  }
}
