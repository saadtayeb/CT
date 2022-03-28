#include <iostream>
#include <gtkmm.h>
#include "commands.hpp"
#include "deep-learning.hpp"
#include <string.h>
#include <fcntl.h> // Contains file controls like O_RDWR
#include <unistd.h> // write(), read(), close()
using namespace std;

int serial_port = open("/dev/ttyACM0", O_WRONLY);//O_RDWR



/////////////////////////////////////CONTROL///////////////////////////////////////////////////

Myhardware::Myhardware() {

}
void Myhardware::go_to_bearing(int bearing) {
  cout << "i am going to bearing" << bearing << endl;
  string s = 'b' + to_string(bearing) +'\n';
  unsigned char msg[s.length()];
  for (int i  = 0; i <s.length(); i++)
  {
  	msg[i] = s[i];
  }
  write(serial_port, msg , sizeof(msg));
}

void Myhardware::joystick_connect() {
  cout << "connect" << endl;
  write(serial_port,"j1\n", 3*sizeof(char));
}

void Myhardware::joystick_disconnect() {
 cout << "disconnect" << endl;
write(serial_port,"j0\n", 3*sizeof(char));
}

void move_camera(int x, int y, bool b) {
  string s = "x"+ to_string(x) +'/' + to_string(y)+  '#' +  to_string(b)+'\n' ;
  unsigned char msg[s.length()];
  for (int i  = 0; i <s.length(); i++)
  {
  	msg[i] = s[i];
  }
  //cout<< "MY OUTPUT_X     " << msg <<endl;
  write(serial_port, msg, sizeof(msg));
  
  return;
}

void telemetre_distance(int distance)
{
 string s ="d" + to_string(distance)+'\n';
 unsigned char msg[s.length()];
  for (int i  = 0; i <s.length(); i++)
  {
  	msg[i] = s[i];
  }
  write(serial_port, msg, sizeof(msg));
}
