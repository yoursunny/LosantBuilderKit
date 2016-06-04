#include "Led.hpp"
#include <assert.h>

Led::Led(int pin, int mode)
  : m_next(nullptr)
  , m_isOverride(false)
  , m_pin(pin)
  , m_isLowMode(mode == LOW)
  , m_state(State::UNSET)
  , m_dim(0.0)
{
  assert(mode == HIGH || mode == LOW);
  pinMode(pin, OUTPUT);
}

Led::Led(Led& next)
  : m_next(&next)
  , m_isOverride(false)
  , m_pin(next.m_pin)
  , m_isLowMode(next.m_isLowMode)
  , m_state(State::UNSET)
  , m_dim(0.0)
{
}

void
Led::display()
{
  if (m_isOverride) {
    // LED controlled by higher priority controller
    return;
  }

  if (m_state == State::UNSET) {
    if (m_next == nullptr) {
      analogWrite(m_pin, 0);
      digitalWrite(m_pin, m_isLowMode ? HIGH : LOW);
    }
    else {
      m_next->m_isOverride = false;
      m_next->display();
    }
    return;
  }

  if (m_next != nullptr) {
    m_next->m_isOverride = true;
  }

  if (m_state == State::OFF) {
    analogWrite(m_pin, 0);
    digitalWrite(m_pin, m_isLowMode ? LOW : HIGH);
  }
  else if (m_state == State::ON) {
    analogWrite(m_pin, 0);
    digitalWrite(m_pin, m_isLowMode ? LOW : HIGH);
  }
  else {
    analogWrite(m_pin, static_cast<int>(PWMRANGE * (m_isLowMode ? (1.0 - m_dim) : m_dim)));
  }
}
