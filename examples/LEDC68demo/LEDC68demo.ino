/*!
 * TM1651 Example with a Re-Used Gotek LEDC68/TM1651 based 3-digit LED board.
 *
 * Written for the Arduino Uno/Nano/Mega.
 * (c) Ian Neill 2024
 * GPL(v3) Licence
 *
 * Built on work by Derek Cooper. Thank you for the jump start!
 * References:
 *    https://www.instructables.com/Re-use-LEDC86-Old-Gotek-Display/
 */

#include "easiTM1651.h"

#define ON      HIGH
#define OFF     LOW

// Pin definitions for the TM1651 - the interface might look like I2C, but it is not!
#define CLKPIN  2                                         // Clock.
#define DIOPIN  3                                         // Data In/Out.
#define LEDPIN  13                                        // The builtin LED.

// Instantiate a TM1651 display.
TM1651 myDisplay(CLKPIN, DIOPIN, true);                   // Set Clock and data pins and declare that we have an LEDC68 module.

void setup() {
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, OFF);
  Serial.begin(9600);
  myDisplay.begin(3, INTENSITY_TYP);                      // Digits = 3, Brightness = 2, Display cleared (all segments OFF and decimal points OFF).
  Serial.println("\nDisplay brightness and digit test.");
  testDisplay();
  blinkLED(100);
  delay(1000);
}

void loop() {
  unsigned long timeNow;                                  // Used to time the 5 and 10 min delay demos.
  
  // 8-bit hex number count up, 0x00 - 0xFF, 1 count/125ms.
  Serial.println("Demo 1: 8-bit hex count up.");
  countHex8(125);
  blinkLED(100);
  delay(1000);
  
  // 12-bit hex number count up, 0x000 - 0xFFF, 1 count/125ms.
  Serial.println("Demo 2: 12-bit hex count up.");
  countHex12(125);
  blinkLED(100);
  delay(1000);
  
  // Decimal number count up, 0 - 999, 1 count/100ms.
  Serial.println("Demo 3: 999 decimal count up.");
  countUp(999, 100);
  blinkLED(100);
  delay(1000);
  
  // Decimal number count down, 999 - 0, 1 count/500ms.
  Serial.println("Demo 4: 999 decimal count down.");
  countDown(999, 500);
  blinkLED(100);
  delay(1000);
  
  // A 10 minute timer.
  Serial.print("Demo 5: A 5 minute delay() timer: ");
  timeNow = millis();
  countXMins(5);
  Serial.print(millis() - timeNow);
  Serial.println("ms");
  blinkLED(100);
  delay(1000);
  
  // A 10 minute timer with flashing decimal point.
  Serial.print("Demo 6: A 10 minute millis() timer & flashing decimal point: ");
  timeNow = millis();
  countXMinsDP(10);
  Serial.print(millis() - timeNow);
  Serial.println("ms");
  blinkLED(100);
  delay(1000);
}

// Flash the builtin LED to signal the end of a demo stage.
void blinkLED(uint32_t interval) {
  digitalWrite(LEDPIN, ON);
  delay(interval);
  digitalWrite(LEDPIN, OFF);
}

void testDisplay() {
  byte brightness, digit, character;
  // Display brightness test.
  for(brightness = INTENSITY_MIN; brightness <= INTENSITY_MAX; brightness++) {
    myDisplay.displayBrightness(brightness);
    myDisplay.displayInt12(0, (brightness * 111));
    delay(1000);
  }
  // Clear the display and set the brightness to the typical value.
  myDisplay.displayClear();
  myDisplay.displayBrightness(INTENSITY_TYP);
  // Display all characters on each digit.
  for(digit = 0; digit < 3; digit++) {
    // Cycle through each code in the character table, deliberately exceeding the table size by 1 to finish on a default space (0x00).
    for(character = 0; character <= myDisplay.charTableSize; character++) {
      myDisplay.displayChar(digit, character);
      delay(200);
    }
  }
  // Decimal points ON/OFF test.
  myDisplay.displayClear();
  myDisplay.displayDP(true);
  delay(500);
  myDisplay.displayDP(false);
  delay(500);
  }

void countHex8(uint32_t interval) {
  byte counter = 0;
  myDisplay.displayChar(0, 0x12);                         // Print an "h" in the 1st digit.
  do {
    myDisplay.displayInt8(1, counter, false);             // Print the 8-bit count in the 2nd and 3rd digits.
    delay(interval);
  } while(++counter != 0);
}

void countHex12(uint32_t interval) {
  uint16_t counter = 0;
  do {
    myDisplay.displayInt12(0, counter, false);            // Print the 12-bit count in the 1st, 2nd and 3rd digits.
    delay(interval);
  } while(++counter != 0x1000);
}

void countUp(uint16_t number, uint32_t interval) {
  int16_t counter;
  for(counter = 0; counter <= number; counter++) {
    myDisplay.displayInt12(0, counter);                   // Print the 12-bit count in the 1st, 2nd and 3rd digits.
    delay(interval);
  }
}

void countDown(uint16_t number, uint32_t interval) {
  int16_t counter;
  for(counter = number; counter >= 0; counter--) {
    myDisplay.displayInt12(0, counter);                   // Print the 12-bit count in the 1st, 2nd and 3rd digits.
    delay(interval);
  }
}

void countXMins(byte minutesMax) {
  byte minutes, seconds;
  if(minutesMax > 16) {
    minutesMax = 16;
  }
  for(minutes = 0; minutes < minutesMax; minutes++) {
    for(seconds = 0; seconds < 60; seconds++) {
      myDisplay.displayChar(0, minutes);                  // Print the minutes in the 1st digit.
      myDisplay.displayChar(1, seconds / 10);             // Print the seconds (x10) in the 2nd digit.
      myDisplay.displayChar(2, seconds % 10);             // Print the seconds (units) in the 3rd digit.
      delay(1000);
    }
  }
}

void countXMinsDP(byte minutesMax) {
  bool dPoint = true;
  byte minutes = 0, seconds = 0;
  unsigned long timeNow, timeMark;
  timeMark = millis();
  if(minutesMax > 16) {
    minutesMax = 16;
  }
  while (minutes != minutesMax || !dPoint) {
    timeNow = millis();
    if(timeNow - timeMark >= 500) {
      timeMark = timeNow;
      // Update the display decimal point.
      myDisplay.displayDP(dPoint);
      // Update the display and increment the time.
      if(dPoint) {
        myDisplay.displayChar(0, minutes);                // Print the minutes in the 1st digit.
        myDisplay.displayChar(1, seconds / 10);           // Print the seconds (x10) in the 2nd digit.
        myDisplay.displayChar(2, seconds % 10);           // Print the seconds (units) in the 3rd digit.
        if(++seconds == 60) {
          seconds = 0;
          minutes++;
        }
      }
      // Toggle the decimal point flag.
      dPoint = !dPoint;
    }
  };
}

// EOF