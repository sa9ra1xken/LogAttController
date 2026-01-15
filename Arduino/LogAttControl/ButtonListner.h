#ifndef BUTTON_LISTNER_H
#define BUTTON_LISTNER_H

class Button {
public:
  Button(int port) : _port(port) {};
  void begin();
  bool wasPressed;
  bool wasReleased;
  int duration;
  void tick();

private:
  int _port;
  bool _state;
  bool _previous_state;
  int _tick_count;
};

#endif
