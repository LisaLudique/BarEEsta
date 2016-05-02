/*
  Utilizes LiquidCrystal Library

 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.

 This sketch prints "Hello World!" to the LCD
 and shows the time.

  The circuit:
 * LCD RS pin to digital pin 4
 * LCD Enable pin to digital pin 5
 * LCD D4 pin to digital pin 9
 * LCD D5 pin to digital pin 10
 * LCD D6 pin to digital pin 11
 * LCD D7 pin to digital pin 12
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)

 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/LiquidCrystal
 */

// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(4, 5, 9, 10, 11, 12);

// declare I/O Pin constants
const int WATER_PIN = 6;
const int JUICE_PIN = 7;
const int BEER_PIN = 8;
const int X_PIN = 2;
const int Y_PIN = 3;
const int PRESSURE_PIN = 1;
const int PHOTODIODE_PIN = 0;
// declare global constants
const double WATER_COST = 0.00;
const double JUICE_COST = 2.50;
const double BEER_COST = 4.00;
const int ALC_LIMIT = 2;
volatile int drinkPin;
volatile double totalCost = 0.00;
volatile int timeElapsed = 0;
volatile int alc = 0;
volatile bool poured = false;
volatile bool pouredAlc = false;

// variables set before compiling to control program behavior
boolean debug = true; // debugging statements sent to serial port
const int fsmFreq = 1; // freq of FSM in Hz
int curState;
int cycTime;

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Cost: ");
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  curState = 1; // initialize for first state in the program loop
  cycTime = 1000/fsmFreq; // calculate number of ms to wait per cycle
  pinMode(WATER_PIN, OUTPUT);
  pinMode(JUICE_PIN, OUTPUT);
  pinMode(BEER_PIN, OUTPUT);
  pinMode(X_PIN, INPUT);
  pinMode(Y_PIN, INPUT);
  pinMode(PRESSURE_PIN, INPUT);
  pinMode(PHOTODIODE_PIN, INPUT);
  
  if (debug)
  {
    Serial.begin(9600); // This pipes serial print data to the serial monitor
    Serial.println("Initialization complete.");
  }
}


// FINITE STATE MACHINE LOOP
// We use the switch/case statement to execute only the code for the current state.
// Then we figure out what the next state is and continue the main program loop.
void loop() {
  // print the number of seconds since reset:
  // lcd.print(millis() / 1000);

  // first do anything we need to do before every state cycle
  if (debug)
  {
    Serial.print("Current State = ");
    Serial.println(curState);
  }

  switch (curState)
  {
    case 1: // A: waiting for choice of drink, default/reset state
      digitalWrite(WATER_PIN, LOW);
      digitalWrite(JUICE_PIN, LOW);
      digitalWrite(BEER_PIN, LOW);
      totalCost = 0.00;
      alc = 0;
      timeElapsed = 0;
      if (digitalRead(X_PIN) && digitalRead(Y_PIN) && alcMax())
        curState = 0;
      else if (digitalRead(X_PIN) && digitalRead(Y_PIN))
        curState = 4;
      else if (digitalRead(X_PIN) && !digitalRead(Y_PIN))
        curState = 3;
      else if (!digitalRead(X_PIN) && digitalRead(Y_PIN))
        curState = 2;
      else
        curState = 1;
      break;
        
    case 2: // B: water selected
      drinkPin = WATER_PIN;
      refill();
      if (poured)
      {
        totalCost += WATER_COST;
        poured = false;
      }
      if (digitalRead(X_PIN) && digitalRead(Y_PIN) && alcMax())
        curState = 2;
      else if (digitalRead(X_PIN) && digitalRead(Y_PIN))
        curState = 4;
      else if (digitalRead(X_PIN) && !digitalRead(Y_PIN))
        curState = 3;
      else if (!digitalRead(X_PIN) && digitalRead(Y_PIN))
        curState = 2;
      else
        curState = 2;
      break;

    case 3: // C: juice selected
      drinkPin = JUICE_PIN;
      refill();
      if (poured)
      {
        totalCost += JUICE_COST;
        poured = false;
      }
      if (digitalRead(X_PIN) && digitalRead(Y_PIN) && alcMax())
        curState = 3;
      else if (digitalRead(X_PIN) && digitalRead(Y_PIN))
        curState = 4;
      else if (digitalRead(X_PIN) && !digitalRead(Y_PIN))
        curState = 3;
      else if (!digitalRead(X_PIN) && digitalRead(Y_PIN))
        curState = 2;
      else
        curState = 3;
      break;


    case 4: // D: beer selected
      drinkPin = BEER_PIN;
      refill();
      if (poured)
      {
        totalCost += BEER_COST;
        poured = false;
        pouredAlc = true;
      }
      if (digitalRead(X_PIN) && digitalRead(Y_PIN) && alcMax())
        curState = 0;
      else if (digitalRead(X_PIN) && digitalRead(Y_PIN))
        curState = 4;
      else if (digitalRead(X_PIN) && !digitalRead(Y_PIN))
        curState = 3;
      else if (!digitalRead(X_PIN) && digitalRead(Y_PIN))
        curState = 2;
      else
        curState = 4;
      break;

    default:
    {
    } // end of switch statement
  }
  
  if (pouredAlc)
    timeElapsed++;
    
  if (timeElapsed >= 3600)
  {
    timeElapsed = 0;
    pouredAlc = false;
    alc = 0;
  }
  
  lcd.setCursor(0, 1);
  lcd.print(totalCost);
  
  delay(cycTime);

}

void refill() 
{
  while(!digitalRead(PRESSURE_PIN) && !digitalRead(PHOTODIODE_PIN)) // hasn't met high threshold
  {
    digitalWrite(drinkPin, HIGH);
    poured = true;
  }
  digitalWrite(drinkPin, LOW);
}

boolean alcMax()
{
  return (timeElapsed > 59 && alc > ALC_LIMIT);
}