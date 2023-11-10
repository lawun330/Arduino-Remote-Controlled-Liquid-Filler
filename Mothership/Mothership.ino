// import libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// define characters for car movements
#define FORWARD       'F'
#define BACKWARD      'B'
#define LEFT          'L'
#define RIGHT         'R'
#define FORWARDLEFT   '2'
#define FORWARDRIGHT  '1'
#define BACKLEFT      '3'
#define BACKRIGHT     '4'
#define STOP          's'

// define dispenser characters
#define LEFT_FILL     'l'
#define RIGHT_FILL    'r'

// define dispenser pins
#define DISPENSER_PIN   42
#define DIS_POWERPIN    43

// define car pins
#define LEFTSPEED_PIN   2
#define RIGHTSPEED_PIN  9
#define LEFT_RED        4
#define LEFT_BLACK      3
#define RIGHT_RED       11
#define RIGHT_BLACK     10
#define EN1_PIN         26
#define EN2_PIN         27

// initiate variables for battery indicator circuit
int analogInput = A0; 
float vout = 0.0;
float vin = 0.0;
float R1 = 100000;
float R2 = 10000;
int value = 0;
double voltage;

// initiate radio data transfer
RF24 radio(7,8); 
const byte address[][6] = {"3rr0r","Error"};  
int Received_array[5];

// initiate car control variables
int x_axis,y_axis; 
int button_joy,button_L,button_R; 
int SpeedF,SpeedB,SpeedL,SpeedR;
char data;

// set dispenser initial state
int DIS_STATE = LOW; 

// button debounce
unsigned long lastDebounceTime=0;
unsigned long debounceDelay=50;
int button_state;
int button_last=LOW;

const int x_normal=50; //modifiable constant
const int y_normal=49;

// one time code
void setup()
{
  Serial.begin(9600);
  radioReceived();
  pinMode(EN1_PIN,OUTPUT);
  pinMode(EN2_PIN,OUTPUT);
  pinMode(analogInput,INPUT);
  pinMode(LEFTSPEED_PIN,OUTPUT);
  pinMode(RIGHTSPEED_PIN,OUTPUT);  
  pinMode(LEFT_RED,OUTPUT);
  pinMode(LEFT_BLACK,OUTPUT);
  pinMode(RIGHT_RED,OUTPUT);
  pinMode(RIGHT_BLACK,OUTPUT);
  pinMode(DISPENSER_PIN,OUTPUT);
  pinMode(DIS_POWERPIN,OUTPUT);
  // Enable pins from a motor driver
  digitalWrite(EN1_PIN,HIGH);
  digitalWrite(EN2_PIN,HIGH);
  // Dispenser pin
  digitalWrite(DISPENSER_PIN,DIS_STATE);
  digitalWrite(DIS_POWERPIN,HIGH);
}

// loop code
void loop()
{
  joystickInput();
  data=keys();
  Speed();
//  print_val();
  carResponse(data);
  refill();
}

///////////////////////
// a function to control car movements based on the radio command
void carResponse (char command)
{   
  switch (command)
  {
    case (FORWARD):
    {
      digitalWrite(LEFT_RED, HIGH);
      digitalWrite(LEFT_BLACK, LOW);
      digitalWrite(RIGHT_RED, HIGH);
      digitalWrite(RIGHT_BLACK, LOW);
      mainSpeed(SpeedF);
    }
      break;

    case (BACKWARD):
    {
      digitalWrite(LEFT_RED, LOW);
      digitalWrite(LEFT_BLACK, HIGH);
      digitalWrite(RIGHT_RED, LOW);
      digitalWrite(RIGHT_BLACK, HIGH);
      mainSpeed(SpeedB);
    }
      break;

    case (LEFT):
    {
      digitalWrite(LEFT_RED, HIGH);
      digitalWrite(LEFT_BLACK, LOW);
      digitalWrite(RIGHT_RED, LOW);
      digitalWrite(RIGHT_BLACK, HIGH);
      mainSpeed(SpeedL);
    }
      break;

    case (RIGHT):
    {
      digitalWrite(LEFT_RED, LOW);
      digitalWrite(LEFT_BLACK, HIGH);
      digitalWrite(RIGHT_RED, HIGH);
      digitalWrite(RIGHT_BLACK, LOW);
      mainSpeed(SpeedR);
    }
      break;

    case (FORWARDLEFT):
    {
      digitalWrite(LEFT_RED, HIGH);
      digitalWrite(LEFT_BLACK, LOW);
      digitalWrite(RIGHT_RED, HIGH);
      digitalWrite(RIGHT_BLACK, LOW);
      leftSpeedUP(SpeedL);
    }
      break;

    case (FORWARDRIGHT):
    {
      digitalWrite(LEFT_RED, HIGH);
      digitalWrite(LEFT_BLACK, LOW);
      digitalWrite(RIGHT_RED, HIGH);
      digitalWrite(RIGHT_BLACK, LOW);
      rightSpeedUP(SpeedR);
    }
      break;

    case (BACKLEFT):
    {
      digitalWrite(LEFT_RED, LOW);
      digitalWrite(LEFT_BLACK, HIGH);
      digitalWrite(RIGHT_RED, LOW);
      digitalWrite(RIGHT_BLACK, HIGH);
      leftSpeedUP(SpeedL);
    }
      break;

    case (BACKRIGHT):
    {
      digitalWrite(LEFT_RED, LOW);
      digitalWrite(LEFT_BLACK, HIGH);
      digitalWrite(RIGHT_RED, LOW);
      digitalWrite(RIGHT_BLACK, HIGH);
      rightSpeedUP(SpeedR);
    }
      break;

    case (STOP):
    {      
      digitalWrite(LEFT_RED, LOW);
      digitalWrite(LEFT_BLACK, LOW);
      digitalWrite(RIGHT_RED, LOW);
      digitalWrite(RIGHT_BLACK, LOW);
      mainSpeed(0);
    }
      break;
  }
}
////////////////////////
// a function for acceleration
void leftSpeedUP (int val)
{
  analogWrite(LEFTSPEED_PIN,val);
  analogWrite(RIGHTSPEED_PIN,val/2);
}
////////////////////////
// a function for acceleration
void rightSpeedUP (int val)
{
  analogWrite(LEFTSPEED_PIN,val/2);
  analogWrite(RIGHTSPEED_PIN,val);
}
////////////////////////
// a function for acceleration
void mainSpeed(int val)
{
  analogWrite(LEFTSPEED_PIN,val);
  analogWrite(RIGHTSPEED_PIN,val);
}
////////////////////////
// a function for acceleration
void Speed()
{ 
  if (x_axis==9) x_axis=10;
  if (y_axis==9) y_axis=10;
  SpeedF=map(y_axis,40,100,0,255); //forward
  SpeedB=map(y_axis,40,0,0,255); //backward
  SpeedL=map(x_axis,50,0,0,255); //left
  SpeedR=map(x_axis,50,100,0,255); //right
}
////////////////////////
// a function to listen radio commands
void radioReceived()
{
  radio.begin();
  radio.openReadingPipe(1,address[0]);
  radio.openWritingPipe(address[1]);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}
///////////////////////
// a function for joystick data transmitted from the remote
void joystickInput()
{
  radio.startListening();
  if (radio.available())
    {
      radio.read(&Received_array,sizeof(Received_array));
      x_axis      = Received_array[0];
      y_axis      = Received_array[1];
      button_joy  = Received_array[2];
      button_L    = Received_array[3];
      button_R    = Received_array[4]; 
      delay(40);
    }
}
///////////////////////
// a function to make distinct commands for each combination of inputs
char keys()
{
  if      (x_axis==x_normal and y_axis==y_normal and button_joy==0 and button_L==0 and button_R==0) {return 's';}
  else if (x_axis==x_normal and y_axis>y_normal and button_joy==0 and button_L==0 and button_R==0)  {return 'F';}
  else if (x_axis==x_normal and y_axis<y_normal and button_joy==0 and button_L==0 and button_R==0)  {return 'B';}
  else if (x_axis<x_normal and y_axis==y_normal and button_joy==0 and button_L==0 and button_R==0)  {return 'L';}
  else if (x_axis>x_normal and y_axis==y_normal and button_joy==0 and button_L==0 and button_R==0)  {return 'R';}
  
  else if (x_axis>x_normal and y_axis>y_normal and button_joy==0 and button_L==0 and button_R==0)   {return '1';}
  else if (x_axis<x_normal and y_axis>y_normal and button_joy==0 and button_L==0 and button_R==0)   {return '2';}
  else if (x_axis<x_normal and y_axis<y_normal and button_joy==0 and button_L==0 and button_R==0)   {return '3';}
  else if (x_axis>x_normal and y_axis<y_normal and button_joy==0 and button_L==0 and button_R==0)   {return '4';}  

  else if (x_axis==x_normal and y_axis==y_normal and button_joy==1 and button_L==0 and button_R==0){sendVoltageData(); return 's';}
}
//////////////////////
// a function to calculate battery voltage for battery indicator circuit
float battery()
{
  value = analogRead(analogInput);
  vout = (value * 5.0) / 1024;
  vin = (vout / (R2/(R1+R2))+0.2);
  return vin;
}
//////////////////////
// a function to send data about battery
void sendVoltageData()
{
      voltage=battery();
      Serial.print("Sending:");
      Serial.println(voltage);
      radio.stopListening();
      radio.write(&voltage,sizeof(voltage)); 
}
//////////////////////
// a function to control the dispenser to start and stop filling liquid with a button input
void refill()
{
  if (button_L!=button_last){lastDebounceTime=millis();}
  if ((millis()-lastDebounceTime) > debounceDelay)
  {
    if(button_L!=button_state)
    {
      button_state = button_L;
      if (button_state == HIGH){DIS_STATE = !DIS_STATE;}
    }
  } 
  digitalWrite(DISPENSER_PIN,DIS_STATE);
  button_last = button_L;    
}
//////////////////////
// a function to print values in serial window to check conditions
void print_val()
{ 
  Serial.print(x_axis);
  Serial.print("\t");
  Serial.print(y_axis);
  Serial.print("\t");
  Serial.print(button_joy);
  Serial.print("\t");
  Serial.print(button_L);
  Serial.print("\t");
  Serial.print(button_R);
  Serial.print("\t");
  Serial.print(SpeedF);
  Serial.print("\t");
  Serial.print(SpeedB);
  Serial.print("\t");
  Serial.print(SpeedL);
  Serial.print("\t");
  Serial.print(SpeedR);
  Serial.print("\t");
  Serial.println(data);
}
//////////////////////
/*
//#define LITER1_5      60000
//#define MOTHERTEMP_PIN  4
//#define SMALLTEMP_PIN   5
*/
