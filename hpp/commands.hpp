#ifndef COMMANDS_H
#define COMMANDS_H



#include <gtkmm.h>
void arduino_handshake();
void move_camera(int x, int y, bool b);
void telemetre_distance(int distance);
void fire_signal();
class Myhardware {
  public:
    Myhardware();
  void go_to_bearing(int bearing);
  bool joystick();
  void joystick_connect();
  void joystick_disconnect();
  private:
   sigc::connection joystick_connection;
   sigc::connection joystick_verify;
};
#endif // COMMANDS_H
