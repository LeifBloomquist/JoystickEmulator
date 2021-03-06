// C64 Paddle Emulator for the Arduino Nano 3.0  (ATmega328)

/*

PINS!

D0=RX (USB Reserved)

D1=TX (USB Reserved)

D2=Joystick Up (Output)

D3=Joystick Down (Output)

D4=Joystick Left (Output)

D5=Joystick Right (Output)

D6=Joystick Fire (Output)

D7=POTX Sense (Input)

D8=POTX PWM (Output)

D9=POTY PWM (Output)

*/

#include <EnableInterrupt.h>
#include <digitalWriteFast.h>

#define PIN_POTX_IN    7   // Assume POTX and POTY are pulled low simultaneously, use POTX to trigger.
#define PIN_POTX_OUT   8
#define PIN_POTY_OUT   9

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
  // Configure the outputs that will be used to fake out the paddle signals
  pinModeFast(PIN_POTX_OUT, OUTPUT);
  digitalWriteFast(PIN_POTX_OUT, HIGH);
  pinModeFast(PIN_POTY_OUT, OUTPUT);
  digitalWriteFast(PIN_POTY_OUT, HIGH);
  
  // Configure the input lines that will be used to detect when the SID pulls POTX low to start the sequence  
  pinMode(PIN_POTX_IN, INPUT_PULLUP);  // Configure the pin as an input, and turn on the pullup resistor.                                    
  enableInterrupt(PIN_POTX_IN | PINCHANGEINTERRUPT, interruptFunction, FALLING);
}


void loop() 
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
