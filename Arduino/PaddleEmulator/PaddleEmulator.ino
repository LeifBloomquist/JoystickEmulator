#include <EnableInterrupt.h>
#include <digitalWriteFast.h>

// C64 Paddle Emulator for the Arduino Nano 3.0  (ATmega328)

#define PIN_POTX_IN    12
#define PIN_POTY_IN    11
#define PIN_POTX_OUT   8
#define PIN_POTY_OUT

volatile uint16_t interruptCountX=0; // The count will go back to 0 after hitting 65535.
volatile uint16_t interruptCountY=0; // The count will go back to 0 after hitting 65535.

volatile uint16_t potXdelay=3;
volatile uint16_t potYdelay=0;

uint16_t lastCountX=0;
uint16_t lastCountY=0;

// The lowest value I've been able to get on the POTX and POTY registers is 2.

void interruptFunctionX() 
{
  // Turn the POTX output off (low).
  digitalWriteFast(PIN_POTX_OUT, LOW);
  
  // Wait while the SID discharges the capacitor.  
  // According to the spec, this should be 320 microseconds.
  // With function overhead and so on, this seems about right.
  delayMicroseconds(235);
  
  // Now, delay the amount required to represent the desired value.
  delayMicroseconds(potXdelay);
  
  digitalWriteFast(PIN_POTX_OUT, HIGH);
}

void interruptFunctionY() 
{
  interruptCountY++;
}


// Helper function to do the math between desired value and delay used in the interrupts.
int PaddleValueToDelay(int value)
{
   int delay = value;   // A 1:1 mapping actually works pretty well.  If tweaking/scaling is required, do it here.
   if (delay > 250) delay = 250;  // Larger values cause the interrupt to take too long 
   if (delay < 3)   delay = 3;    // Smallest permissible value.  See http://www.arduino.cc/en/Reference/DelayMicroseconds
   return delay;
}


// Attach the interrupt in setup()
void setup() 
{
  Serial.begin(57600);
  
  // Configure the outputs that will be used to fake out the paddle signals
  pinModeFast(PIN_POTX_OUT, OUTPUT);
  digitalWriteFast(PIN_POTX_OUT, HIGH);
  
  // COnfigure the input lines that will be used to detect when the SID pulls them low to start the sequence
  
  pinMode(PIN_POTX_IN, INPUT_PULLUP);  // Configure the pin as an input, and turn on the pullup resistor.
  pinMode(PIN_POTY_IN, INPUT_PULLUP);  // Configure the pin as an input, and turn on the pullup resistor.
                                    
  enableInterrupt(PIN_POTX_IN | PINCHANGEINTERRUPT, interruptFunctionX, FALLING );
  enableInterrupt(PIN_POTY_IN | PINCHANGEINTERRUPT, interruptFunctionY, FALLING );
}


void loop() 
{
  potXdelay = PaddleValueToDelay(245);
  
  // Debug code  
  Serial.println("---------------------------------------");
  delay(1000);                          // Every second,
  Serial.print("X Pin was interrupted: ");
  Serial.print(interruptCountX, DEC);      // print the interrupt count.
  Serial.print(" times so far.  ");
  Serial.print("Delta: ");
  Serial.println(interruptCountX-lastCountX);
  lastCountX = interruptCountX;
  
  Serial.print("Y Pin was interrupted: ");
  Serial.print(interruptCountY, DEC);      // print the interrupt count.
  Serial.print(" times so far.  ");
  Serial.print("Delta: ");
  Serial.println(interruptCountY-lastCountY);
  lastCountY = interruptCountY;
}
