// C64 Joystick Emulator for the Arduino Nano

#define PIN_UP    2
#define PIN_DOWN  3
#define PIN_LEFT  4
#define PIN_RIGHT 5
#define PIN_FIRE  6
#define PIN_LED   13

void setup() 
{
  pinMode(PIN_UP,   OUTPUT);
  pinMode(PIN_DOWN, OUTPUT);
  pinMode(PIN_LEFT, OUTPUT);
  pinMode(PIN_RIGHT,OUTPUT);
  pinMode(PIN_FIRE, OUTPUT);
  pinMode(PIN_LED,  OUTPUT);
  
  digitalWrite(PIN_UP,   LOW);
  digitalWrite(PIN_DOWN, LOW);
  digitalWrite(PIN_LEFT, LOW);
  digitalWrite(PIN_RIGHT,LOW);
  digitalWrite(PIN_FIRE, LOW);
  digitalWrite(PIN_LED,  LOW);
  
  Serial.begin(57600);
}

void loop() 
{
   // Wait for a byte
  if (Serial.available()) 
  {
    digitalWrite(PIN_LED, HIGH);
    byte inByte = Serial.read();
    doJoystick(inByte);   
    digitalWrite(PIN_LED, LOW);
  }
  else
  {
    digitalWrite(PIN_LED, LOW);
  }
}

void doJoystick(byte inByte)
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
