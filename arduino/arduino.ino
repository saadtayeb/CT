#include <HCSR04.h>
#include<Servo.h>
#include<math.h>
#include <PID_v1.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

#define NUM_JOY 2
#define MIN_VAL 0
#define MAX_VAL 1023
#define DELAY_MAX 500
#define DELAY_MIN 10
#define DELAY_MOY 1
#define SERVO_X_LIM_MIN 0
#define SERVO_X_LIM_MAX 180
#define SERVO_Y_LIM_MIN 60
#define SERVO_Y_LIM_MAX 160
#define SERVO_X_LIM_MS_MIN 550
#define SERVO_X_LIM_MS_MAX 2400
#define SERVO_Y_LIM_MS_MIN 1300
#define SERVO_Y_LIM_MS_MAX 2200
#define DISTANCE_TRESHOLD 5
int delay_tir = 3000;
bool tir_state =false;
byte triggerPin = 11;
byte echoPin = 12;
float distance;
float ecart_x = 25;
float ecart_y = 8;
const int laser = 4;
const int fire_pin = 8;
int ms_step = 4;
double Setpoint_x, Input_x, Output_x;
double Setpoint_y, Input_y, Output_y;
Servo servo_x;
Servo servo_y;
Servo servo_arme_x;
Servo servo_arme_y;
const double Kpx = 0.0271 , Kix = 0.00109, Kdx = 0.000587 ;
const double Kpy = 0.018 , Kiy = 0.0030, Kdy = 0.000106 ;

const int joyPin [2] = {A9, A8};
int c;

float temp_distance;
String msg = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
char  inChar;
int x , y;
int gisement;
int joy_x,joy_y;
int servo_gisement, servo_site;
int servo_x_ms,servo_arme_x_ms,servo_y_ms,servo_arme_y_ms;
int delay_x,delay_y,tx1,tx2,ty1,ty2;
int t1,t2;
int calibration_joystick_value_x,calibration_joystick_value_y; 
int joy_error = 2;
int reference[2] = {90,90};
int jeu_x = -6;
int jeu_y = 6;
bool telemetre_state = true;
bool joytick_state = false;
bool gisement_search_state  = false;
bool tracking_state = false; 
bool couplage_state = false;
PID PID_x(&Input_x, &Output_x, &Setpoint_x, Kpx, Kix, Kdx, REVERSE);
PID PID_y(&Input_y, &Output_y, &Setpoint_y, Kpy, Kiy, Kdy, REVERSE);
HCSR04 hc(triggerPin, echoPin);
float distance_msg = 0;
// temp 
int site;
float projection;
char terminator = '\n';

void setup() {
  pinMode(fire_pin,OUTPUT);
  lcd.init();      
  lcd.backlight(); 
  lcd.setCursor(1,0);
  lcd.print("initializing");
  Serial.begin(115200);
  servo_x.attach(2);
  servo_y.attach(3);
  servo_x.write(reference[0]);
  servo_y.write(reference[1]);
  servo_arme_x.attach(5);
  servo_arme_y.attach(6);
  servo_arme_x.write(reference[0]);
  servo_arme_y.write(reference[1]);
   for (int i = 0; i < NUM_JOY; i++) pinMode(joyPin[i], INPUT);
  msg.reserve(200);
  Setpoint_x = 0;
  Setpoint_y = 0;
  PID_x.SetMode(AUTOMATIC);
  PID_y.SetMode(AUTOMATIC);  
  pinMode(laser,OUTPUT);
  digitalWrite(laser,LOW);
  digitalWrite(fire_pin,LOW);
  calibration_joystick_value_x = analogRead(joyPin[0]);
  calibration_joystick_value_y = analogRead(joyPin[1]);
  Serial.println(calibration_joystick_value_x);
  Serial.println(calibration_joystick_value_y);
  delay(100);
  initialisation();
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
Serial.println("ready");
lcd.clear();
}

void loop() {
  if (gisement_search_state)
  {
    gisement = msg.substring(1).toInt();
    gisement_search(gisement);
  }
  if (joytick_state)
  {
    joystick_control();    
  }
  //Serial.print("x    ");Serial.print(servo_x.read());
  //Serial.print("        y    ");Serial.println(servo_y.read());
  if (tir_state)fire();
  couplage_state = false;
  
}


void serialEvent() {
  if (Serial.available()) {
    msg = Serial.readStringUntil(terminator);
    inChar = msg[0];
    if (inChar  == 'b')
    {
      gisement_search_state = true;
      joytick_state = false;
      tracking_state = false;
      tx1 = millis();
      digitalWrite(laser,LOW);

    }
    else if (inChar  == 'j')
    {
      gisement_search_state = false;
      joytick_state = (msg[1] == '1') ? true : false;
      tracking_state = false;
    }
    else if (inChar == 'x')
    {
      gisement_search_state = false;
      joytick_state = false;
      tracking_state = true;
      decode_x_y(msg);
      //tracking();
      tracking_pid();
    }
    else if (inChar == 'd')
    {
      get_distance(msg);     
    }
   else if (inChar == 'f')
    {
      t1 = millis();
      digitalWrite(fire_pin,HIGH);
      tir_state = true;
    } 
  }
}

void gisement_search( int g)
{   
    //Serial.print("gisement    ");Serial.println(g);
    site = reference[1];
    servo_gisement = servo_x.read();
    servo_site = servo_y.read();
    tx2 = millis(); 
    //Serial.println(tx2 - tx1);
    if (tx2 - tx1 >  DELAY_MOY)
    {
      if (servo_gisement < g)
       {
          servo_x.writeMicroseconds(servo_x.readMicroseconds()+1);
       }
       else if (servo_gisement > g)
       {
         servo_x.writeMicroseconds(servo_x.readMicroseconds()-1);
       }
       if (servo_site < site)
       {
          servo_y.writeMicroseconds(servo_y.readMicroseconds()+1);
       }
       else if (servo_site > site)
       {
         servo_y.writeMicroseconds(servo_y.readMicroseconds()-1);
       }
      if  (servo_site == site &&  gisement == servo_x.read())
      {
        gisement_search_state = false;
        return ;
      }
     tx1 = millis();       
  }
}

void joystick_control()
{
    joy_x = map(analogRead(joyPin[0]),MIN_VAL,2*calibration_joystick_value_x,-90,90);
    joy_y = map(analogRead(joyPin[1]),MIN_VAL,2*calibration_joystick_value_y,-90,90);
    delay_x = map(abs(joy_x),90,0,DELAY_MIN,DELAY_MAX);
    delay_y = map(abs(joy_y),90,0,DELAY_MIN,DELAY_MAX);
    tx2 = millis();
    ty2 = millis();
    if (tx2 - tx1 > delay_x)
    {
      if(joy_x>+joy_error)
      {
        servo_go_to('X',servo_x.readMicroseconds()+ms_step);
      }
      else if (joy_x<-joy_error)
      {
        servo_go_to('X',servo_x.readMicroseconds()-ms_step);
      } 
     tx1 = millis(); 
    }
    if (ty2 - ty1 > delay_y)
    {
      if(joy_y> joy_error)
      {
        servo_go_to('Y',servo_y.readMicroseconds()-ms_step);
      }
      else if (joy_y< -joy_error)
      {
        servo_go_to('Y',servo_y.readMicroseconds()+ms_step);
      }  
      ty1 = millis();
    }
}


void servo_go_to(char type,int angle)
{
  
  Serial.println(angle);
 if (type == 'X')
 {
     if (angle < SERVO_X_LIM_MS_MIN)
     {
       servo_x.writeMicroseconds(SERVO_X_LIM_MS_MIN);  
     } 
     else if (angle > SERVO_X_LIM_MS_MAX)
     {
       servo_x.writeMicroseconds(SERVO_X_LIM_MS_MAX);
     }
     else 
     {
        servo_x.writeMicroseconds(angle);
     }
     return;
 }
 else if (type == 'Y')
 {
     if (angle < SERVO_Y_LIM_MS_MIN)
     {
       servo_y.writeMicroseconds(SERVO_Y_LIM_MS_MIN);  
     } 
     else if (angle > SERVO_Y_LIM_MS_MAX)
     {
       servo_y.writeMicroseconds(SERVO_Y_LIM_MS_MAX);
     }
     else
     {
       servo_y.writeMicroseconds(angle);
     }
   return;
 }   
}

void decode_x_y(String myString)
{
  x = myString.substring(1,myString.indexOf('/')).toInt();
  y = myString.substring(myString.indexOf('/')+1).toInt();
  couplage_state = myString.substring(myString.indexOf('#')+1).toInt();
  Input_x = abs(x);
  Input_y = abs(y);
  
  //Serial.print("x    ");Serial.print(x);
  //Serial.print("    y    ");Serial.println(y);
} 

void tracking()
{
   if (x > 0)
  {
     //servo_go_to('X',servo_x.read()-1);
  } 
  else if (x < 0)
  {
     //servo_go_to('X',servo_x.read()+1);
  }
  if (y > 0)
  {
      //servo_go_to('Y',servo_y.read()+1);
  } 
  else if (y < 0)
  {
      //servo_go_to('Y',servo_y.read()-1);
  }
} 

void tracking_pid()
{/*
  servo_x_ms = servo_x.readMicroseconds() - x*Kp;
  servo_y_ms = servo_y.readMicroseconds() + y*Kp/2;
  if (servo_x_ms < SERVO_X_LIM_MS_MIN)
  {
     servo_x.write(SERVO_X_LIM_MS_MIN);  
  } 
  else if (servo_x_ms > SERVO_X_LIM_MS_MAX)
  {
     servo_x.write(SERVO_X_LIM_MS_MAX);
  }
 else 
  {
    servo_x.writeMicroseconds(servo_x_ms);
  }
  if (servo_y_ms < SERVO_Y_LIM_MS_MIN)
  {
    servo_x.write(SERVO_Y_LIM_MS_MIN);  
  } 
  else if (servo_y_ms > SERVO_Y_LIM_MS_MAX)
  {
    servo_y.write(SERVO_Y_LIM_MS_MAX);
  }
  else 
  {
    servo_y.writeMicroseconds(servo_y_ms);
  } 
  */ 
  PID_x.Compute();
  PID_y.Compute();
  //Serial.print("input_x    ");Serial.println(Input_x);
  //Serial.print("output_x    ");Serial.println(Output_x);
  //Serial.print("input_y    ");Serial.println(Input_y);
  //Serial.print("output_y    ");Serial.println(Output_y);
  servo_x.writeMicroseconds(servo_x.readMicroseconds() - (x/abs(x))*Output_x);
  servo_y.writeMicroseconds(servo_y.readMicroseconds() + (y/abs(y))*Output_y);
  if (telemetre_state == true)
  {
    distance = hc.dist();
    if (distance < DISTANCE_TRESHOLD)
    {
       return;
    } 
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Target distance");
    lcd.setCursor(5,1);
    lcd.print(distance);
  } 
  digitalWrite(laser,couplage_state);
  if (couplage_state)
      {
         servo_x_ms =map(parallaxe(servo_x.read(),distance,ecart_x),0,180,455,2400);
         servo_arme_x_ms = servo_arme_x.readMicroseconds();
         servo_y_ms = map(parallaxe(servo_y.read(),distance,ecart_y),0,180,2400,455); 
         servo_arme_y_ms = servo_arme_y.readMicroseconds();
         //Serial.print(servo_x_ms);
         //Serial.print("       ");
         //Serial.println(servo_arme_x_ms);
         /*if (servo_x_ms > servo_arme_x_ms)
          {
            servo_arme_x.writeMicroseconds(servo_arme_x_ms + ms_step);
          }
          else if (servo_x_ms < servo_arme_x_ms)
          {
            servo_arme_x.writeMicroseconds(servo_arme_x_ms - ms_step);   
          }
          if (servo_y_ms > servo_arme_y_ms)
          {
            servo_arme_y.writeMicroseconds(servo_arme_y_ms + ms_step);
          }
          else if (servo_y_ms < servo_arme_y_ms)
          {
            servo_arme_y.writeMicroseconds(servo_arme_y_ms - ms_step);   
          }        
          */ 
         servo_arme_x.write(parallaxe(servo_x.read(),distance,ecart_x)+jeu_x);
         servo_arme_y.write(parallaxe(servo_y.read(),distance,ecart_y)+jeu_y);
         //Serial.println(distance);

         delay(10);
      } 
}

void get_distance(String msg)
{
  distance_msg  =  msg.substring(1).toInt();
  distance = distance_msg;
  if (distance_msg > 0)
  {
     lcd.clear();  
     lcd.setCursor(0,0);
     lcd.print("Target distance");
     lcd.setCursor(5,1);
     lcd.print(distance);
     telemetre_state = false;    
  }
  else
  {
     telemetre_state = true;    
  }  
}  

float parallaxe(float angle,float distance,float ecart)
{
  float angle_rad  = angle * PI /180;  
  projection = ecart - distance*cos(angle_rad);
  if (projection > 0) return 180-atan((distance* sin(angle_rad))/(projection))*180/PI;
  else if (projection < 0) return atan((distance* sin(angle_rad))/(-projection))*180/PI;  
} 


void fire()
{
  t2= millis();
  if (t2 - t1 > delay_tir)
  {
      digitalWrite(fire_pin,LOW);  
      tir_state = false;
  } 
}

void initialisation()
{
  servo_x.write(0);
  servo_arme_x.write(0);
  delay(500);
  servo_x.write(reference[0]);
  servo_arme_x.write(reference[0]);
  delay(500);
  servo_y.write(180);
  servo_arme_y.write(180);
  delay(500);
  servo_y.write(reference[1]);
  servo_arme_y.write(reference[1]);  
  delay(500);

}  
