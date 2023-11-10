// import libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// initiate pins and values for lcd display
LiquidCrystal_I2C lcd(0x38,16,2); 
int row=0;
int col=0;
byte Square[8] = 
{
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111  
};

// declare variables for joystick controls
int x_axis,y_axis,x_val,y_val;   
int button_Joy_val,button_L_val,button_R_val;  

// initiate variables for battery indicator circuit
double voltage; 
double original_volt=10.4;
int percent;

// initiate variables for radio data transmission
RF24 radio(7,8); 
const byte address[][6] = {"3rr0r","Error"};  
int Array[5];

// define pins
int led=3; 
int x=A2;
int y=A3;
int butt_joy=2;
int buttonL=6;
int buttonR=4;

// modify these constants
const int x_normal=5;  
const int y_normal=4;
const int range_constant=20;

// one time code
void setup()
{
  Serial.begin(9600);
  radioInitiateTransmitter();
  startLCD();
  pinMode(x,INPUT);
  pinMode(y,INPUT);
  pinMode(butt_joy,INPUT_PULLUP);
  pinMode(buttonL,INPUT_PULLUP);
  pinMode(buttonR,INPUT_PULLUP);
  pinMode(led,OUTPUT);
}

// loop code
void loop()
{
  getJoystick();
  getButtons();
  displayLCD();
  sendJoystickButtons();
  radioListenVolt();
}

////////////////////
// a function to prepare radio data transmission through a radio module
void radioInitiateTransmitter()
{
  radio.begin();
  radio.openWritingPipe(address[0]);
  radio.openReadingPipe(1,address[1]);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}
/////////////////////
// a function to display initial messages on LCD
void startLCD()
{
  lcd.init();
  lcd.backlight();
  lcd.createChar(1,Square);  //create customization
  lcd.home();
  lcd.print("Initialized...");
  for (int i=0; i<16 ;i++)  //loading block style
  {   
    lcd.setCursor(col,1);      
    lcd.write((byte)1);        
    delay(100);                
    col++;                     
  }
  delay(125);  // 2s for 16 squares
  lcd.clear();
}
/////////////////////
// a function for joystick data to be transmitted
void getJoystick()
{     
  x_axis=analogRead(x);
  y_axis=analogRead(y);
  button_Joy_val=!digitalRead(butt_joy);
  x_val=map(x_axis,0,1023,0,10);
  y_val=map(y_axis,0,1023,0,10);
  Serial.print(x_val); //x=5, y=4 at normal value
  Serial.print("\t");
  Serial.print(y_val);
  Serial.print("\t");
  Serial.print(button_Joy_val);
  Serial.print("\t");
}
/////////////////////
// a function to store and show button states
void getButtons()
{
  button_L_val=!digitalRead(buttonL);
  button_R_val=!digitalRead(buttonR);
  Serial.print(button_L_val);
  Serial.print("\t");
  Serial.println(button_R_val);
}
////////////////////
// a function to display joystick data on LCD
void displayLCD()
{
      if(x_val==x_normal and y_val==y_normal and button_Joy_val==0 and button_L_val==0 and button_R_val==0) 
      {
        digitalWrite(led,LOW); 
        lcd.setCursor(0,0); 
        lcd.print("Drive Mode      "); 
        lcd.setCursor(0,1); 
        lcd.print("STOP            ");
      }
      
      else if (x_val==x_normal and y_val>y_normal and button_Joy_val==0 and button_L_val==0 and button_R_val==0)  {lcd.setCursor(0,0); lcd.print("Drive Mode      "); lcd.setCursor(0,1); lcd.print("DIR - FORWARD   ");}
      else if (x_val==x_normal and y_val<y_normal and button_Joy_val==0 and button_L_val==0 and button_R_val==0)  {lcd.setCursor(0,0); lcd.print("Drive Mode      "); lcd.setCursor(0,1); lcd.print("DIR - BACKWARD  ");}
      else if (x_val<x_normal and y_val==y_normal and button_Joy_val==0 and button_L_val==0 and button_R_val==0)  {lcd.setCursor(0,0); lcd.print("Drive Mode      "); lcd.setCursor(0,1); lcd.print("DIR - LEFT      ");}
      else if (x_val>x_normal and y_val==y_normal and button_Joy_val==0 and button_L_val==0 and button_R_val==0)  {lcd.setCursor(0,0); lcd.print("Drive Mode      "); lcd.setCursor(0,1); lcd.print("DIR - RIGHT     ");}  

      else if (x_axis>x_normal+range_constant and y_axis>y_normal+range_constant and button_Joy_val==0 and button_L_val==0 and button_R_val==0) {lcd.setCursor(0,0); lcd.print("Drive Mode      "); lcd.setCursor(0,1); lcd.print("DIR - F_RIGHT   ");}
      else if (x_axis<x_normal-range_constant and y_axis>y_normal+range_constant and button_Joy_val==0 and button_L_val==0 and button_R_val==0) {lcd.setCursor(0,0); lcd.print("Drive Mode      "); lcd.setCursor(0,1); lcd.print("DIR - F_LEFT    ");}
      else if (x_axis<x_normal-range_constant and y_axis<y_normal-range_constant and button_Joy_val==0 and button_L_val==0 and button_R_val==0) {lcd.setCursor(0,0); lcd.print("Drive Mode      "); lcd.setCursor(0,1); lcd.print("DIR - B_LEFT    ");}
      else if (x_axis>x_normal+range_constant and y_axis<y_normal-range_constant and button_Joy_val==0 and button_L_val==0 and button_R_val==0) {lcd.setCursor(0,0); lcd.print("Drive Mode      "); lcd.setCursor(0,1); lcd.print("DIR _ B_RIGHT   ");}
        
       
      else if (x_val==x_normal and y_val==y_normal and button_Joy_val==1 and button_L_val==0 and button_R_val==0)
      {
        digitalWrite(led,HIGH);
        lcd.setCursor(0,0);
        lcd.print("Battery -    ");
        lcd.setCursor(10,0);
        lcd.print(percent);
        lcd.setCursor(14,0);
        lcd.print("%");
        lcd.setCursor(0,1);
        lcd.print("Voltage - ");
        lcd.setCursor(10,1);
        lcd.print(voltage);
        lcd.setCursor(15,1);
        lcd.print("V");
      }

      else if (x_val==x_normal and y_val==y_normal and button_Joy_val==0 and button_L_val==1 and button_R_val==0){lcd.setCursor(0,0); lcd.print("Refilling Left   "); lcd.setCursor(0,1); lcd.print("                ");}
      else if (x_val==x_normal and y_val==y_normal and button_Joy_val==0 and button_L_val==0 and button_R_val==1){lcd.setCursor(0,0); lcd.print("Refilling Right  "); lcd.setCursor(0,1); lcd.print("                ");}
}
/////////////////////
// a function to transmit data
void sendJoystickButtons()
{ 
  radio.stopListening();
  Array[0]=x_val;
  Array[1]=y_val;
  Array[2]=button_Joy_val;
  Array[3]=button_L_val;
  Array[4]=button_R_val;
  radio.write(&Array,sizeof(Array));
  delay(40);
}
////////////////////
// a function to receive data about voltage
void radioListenVolt()
{
  radio.startListening();
  if(radio.available()){radio.read(&voltage,sizeof(voltage));}
  voltPercent();
}
///////////////////
// a function to calculate voltage percent
void voltPercent()
{
  percent=(voltage/original_volt)*100;
}
