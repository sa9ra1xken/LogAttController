#include "ButtonListner.h"
#include <Arduino.h>

void Button::begin(){
  _previous_state = digitalRead(_port);
  _tick_count = 0;
  wasPressed = false;
  wasReleased = false;
}

void Button::tick(){
  _tick_count++;
  _state = digitalRead(_port);
  if (_state != _previous_state) {
    duration = _tick_count;
    _tick_count = 0; 
    if (_state == LOW) wasPressed = true;
    else wasReleased = true;
  }
  _previous_state = _state;
}