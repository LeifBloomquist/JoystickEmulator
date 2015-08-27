// C64 Paddle/Joystick/Mouse Emulator for the Arduino Nano 3.0  (ATmega328)

#define PIN_UP        2
#define PIN_DOWN      3
#define PIN_LEFT      4
#define PIN_RIGHT     5
#define PIN_FIRE      6
#define PIN_POTX_IN   7   // Assume POTX and POTY are pulled low simultaneously, use POTX to trigger.
#define PIN_POTX_OUT  8
#define PIN_POTY_OUT  9
#define PIN_LED      13

#include <EnableInterrupt.h>
#include <digitalWriteFast.h>

// Paddle Emulator -----------------------------------------------------------------
#define uS_MIN  3    // Smallest permissible value.  See http://www.arduino.cc/en/Reference/DelayMicroseconds
#define uS_MAX  243  // Larger values cause the interrupt to take too long

volatile bool potXleads=true;        // if false, y leads
volatile uint16_t potXdelay=uS_MIN;
volatile uint16_t potYdelay=uS_MIN;

void interruptFunction() 
{
  // Turn the POTX and POTY outputs off (low).
  digitalWriteFast(PIN_POTX_OUT, LOW);
  digitalWriteFast(PIN_POTY_OUT, LOW);
  
  // Wait while the SID discharges the capacitor.  
  // According to the spec, this should be 320 microseconds.
  // With function overhead and so on, this seems about right.
  delayMicroseconds(235);
  
  // Now, delay the amount required to represent the desired values.
  if (potXleads)
  {
     delayMicroseconds(potXdelay);
     digitalWriteFast(PIN_POTX_OUT, HIGH);
     delayMicroseconds(potYdelay);    // This is a delta!
     digitalWriteFast(PIN_POTY_OUT, HIGH);
  }
  else
  {
     delayMicroseconds(potYdelay);
     digitalWriteFast(PIN_POTY_OUT, HIGH);
     delayMicroseconds(potXdelay);    // This is a delta!
     digitalWriteFast(PIN_POTX_OUT, HIGH);
  }
}

// Helper function to do the math between desired value and delay used in the interrupts.
void PaddleValueToDelay(int valueX, int valueY)
{
   // A 1:1 mapping actually works pretty well.  If tweaking/scaling is required, do it here.
   int delayX = valueX;                   
   int delayY = valueY;
   
   // Now the trick to keep everything in one interrupt.  Use the other delay as a delta!
   if (delayX < delayY)
   {
      potXleads = true;
      delayY -= delayX; 
   }
   else
   {
     potXleads = false;
     delayX -= delayY;
   }
   
   if (delayX > uS_MAX) delayX = uS_MAX;    
   if (delayX < uS_MIN) delayX = uS_MIN;
   if (delayY > uS_MAX) delayY = uS_MAX;
   if (delayY < uS_MIN) delayY = uS_MIN; 
  
   potXdelay = delayX;
   potYdelay = delayY;
}

void setup() 
{
  // Joystick
  
  pinModeFast(PIN_UP,   OUTPUT);
  pinModeFast(PIN_DOWN, OUTPUT);
  pinModeFast(PIN_LEFT, OUTPUT);
  pinModeFast(PIN_RIGHT,OUTPUT);
  pinModeFast(PIN_FIRE, OUTPUT);
  pinModeFast(PIN_LED,  OUTPUT);
  
  digitalWriteFast(PIN_UP,   LOW);
  digitalWriteFast(PIN_DOWN, LOW);
  digitalWriteFast(PIN_LEFT, LOW);
  digitalWriteFast(PIN_RIGHT,LOW);
  digitalWriteFast(PIN_FIRE, LOW);
  digitalWriteFast(PIN_LED,  LOW);
  
  // Paddles / Mouse
  
  pinModeFast(PIN_POTX_OUT, OUTPUT);
  pinModeFast(PIN_POTY_OUT, OUTPUT);
  
  digitalWriteFast(PIN_POTX_OUT, HIGH);
  digitalWriteFast(PIN_POTY_OUT, HIGH);
  
  // Configure the input line that will be used to detect when the SID pulls POTX low to start the sequence
  pinMode(PIN_POTX_IN, INPUT_PULLUP);  // Configure the pin as an input, and turn on the pullup resistor.                                    
  enableInterrupt(PIN_POTX_IN | PINCHANGEINTERRUPT, interruptFunction, FALLING);

  // Serial Link to PC  
  Serial.begin(57600);
  Serial.setTimeout(0);
}

void loop() 
{
  // Wait for a string
  digitalWrite(PIN_LED, HIGH);
  String input = GetInput();
  digitalWrite(PIN_LED, LOW);

  if (input == NULL)  // Timed out
  {
    return;
  }
  
  Serial.print("input=");
  Serial.println(input);

  char c = input.c_str()[0];

  if (c == 'J') doJoystick(input);
  if (c == 'P') doPaddles(input);
  //if (c == 'M') doMouse(input);
}


void PaddleTests() 
{
  for (int t = 0;  t < 255; t++)
  {
    PaddleValueToDelay(t, 255-t);
    delay(10);
  }
  
  for (int t = 255;  t > 0; t--)
  {
    PaddleValueToDelay(t, 255-t);
    delay(10);
  }
}

void doJoystick(String input)
{
  int joyval=input.substring(1).toInt();   // 1=skip past the J
  doJoystickValue((byte)joyval);
}

void doPaddles(String input)
{ 
  int commaIndex = input.indexOf(',');
  int x = input.substring(1, commaIndex).toInt();   // 1=skip past the P
  int y = input.substring(commaIndex+1).toInt();
  
  char temp[100];
  sprintf(temp,"Paddles input=%s  x=%d,y=%d",input.c_str(),x,y);
  Serial.println(temp);
  
  PaddleValueToDelay(x, y); 
}


void doJoystickValue(byte inByte)
{
   bool up    = false;
   bool down  = false;
   bool left  = false;
   bool right = false;
   bool fire  = false;
   
   if ((inByte | 0xFE) == 0xFE) { up   = true; }
   if ((inByte | 0xFD) == 0xFD) { down = true; }
   if ((inByte | 0xFB) == 0xFB) { left = true; }
   if ((inByte | 0xF7) == 0xF7) { right= true; }
   if ((inByte | 0xEF) == 0xEF) { fire = true; }
 
   digitalWrite(PIN_UP,   up);
   digitalWrite(PIN_DOWN, down);
   digitalWrite(PIN_LEFT, left);
   digitalWrite(PIN_RIGHT,right);
   digitalWrite(PIN_FIRE, fire);   
}


String GetInput() 
{
  char in_char[10];
  int max_length=sizeof(in_char);
  
  int i=0; // Input buffer pointer
  char key;
  
  while(true) 
  {
      key=ReadByte(Serial); // Read in one character
           
      if ((int)key != 10)  // Filter out\r
      {
         in_char[i]=key;      
      }
      
      if (((int)key == 13)||(i == max_length-1)) 
      { // The 13 represents enter key.
        in_char[i]=0; // Terminate the string with 0.
        return String(in_char);
      }
      i++;
      if (i<0) i=0;
  }
}

int ReadByte(Stream& in)
{
    while (in.available() == 0) {}   
    return in.read();
}
