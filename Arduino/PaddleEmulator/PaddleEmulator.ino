#include <EnableInterrupt.h>

// C64 Paddle Emulator for the Arduino Nano

#define PIN_POTX_IN    12
#define PIN_POTY_IN    11
#define PIN_POTX_OUT
#define PIN_POTY_OUT

volatile uint16_t interruptCountX=0; // The count will go back to 0 after hitting 65535.
volatile uint16_t interruptCountY=0; // The count will go back to 0 after hitting 65535.

uint16_t lastCountX=0;
uint16_t lastCountY=0;

void interruptFunctionX() 
{
  interruptCountX++;
}

void interruptFunctionY() 
{
  interruptCountY++;
}

// Attach the interrupt in setup()
void setup() 
{
  Serial.begin(57600);
  
  pinMode(PIN_POTX_IN, INPUT_PULLUP);  // Configure the pin as an input, and turn on the pullup resistor.
  pinMode(PIN_POTY_IN, INPUT_PULLUP);  // Configure the pin as an input, and turn on the pullup resistor.
                                    
  enableInterrupt(PIN_POTX_IN | PINCHANGEINTERRUPT, interruptFunctionX, FALLING );
  enableInterrupt(PIN_POTY_IN | PINCHANGEINTERRUPT, interruptFunctionY, FALLING );
}

// In the loop, we just check to see where the interrupt count is at. The value gets updated by the interrupt routine.
void loop() {
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
