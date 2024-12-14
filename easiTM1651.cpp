/*!
 * An Easy TM1651 Arduino Library.
 *  Simple, functional, optimal, and all in a class!
 *
 * The TM1651 is an (up to) 4 digit 7-Segment (no dp) LED display driver used in the Gotek 3 digit LEDC68 display.
 *
 * Written for the Arduino Uno/Nano/Mega.
 * (c) Ian Neill 2024
 * GPL(v3) Licence
 *
 * Built on work by Derek Cooper. Thank you for the jump start!
 * References:
 *    https://www.instructables.com/Re-use-LEDC86-Old-Gotek-Display/
 *    https://github.com/coopzone-dc/GotekLEDC68
 *    https://github.com/mworkfun/TM1650
 *    https://github.com/mworkfun/TM1651
 *
 ***************************************************************
 * LED Segments:         a
 *                     -----
 *                   f|     |b
 *                    |  g  |
 *                     -----
 *                   e|     |c
 *                    |     |
 *                     -----  o dp (controlled separately)
 *                       d         (   if there is one   )
 *   Register bits:
 *      bit:  7  6  5  4  3  2  1  0
 *            X  g  f  e  d  c  b  a
 ***************************************************************
 *
 * ****************************
 * * easiTM1651 C++ Code File *
 * ****************************
*/

#include "easiTM1651.h"

// A table of simple character codes.
const static uint8_t tmCharTable[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, // Numbers : 0-9.
                                      0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71,                         // Numbers : A, b, C, d, E, F.
                                      0x58, 0x6f, 0x74, 0x76, 0x1e, 0x38,                         // Chars1  : c, g, h, H, J, L.
                                      0x54, 0x37, 0x73, 0x50, 0x1c, 0x3e, 0x6e,                   // Chars2  : n, N, P, r, u, U, y.
                                      0x01, 0x40, 0x08, 0x00, 0x63, 0x5c, 0x46, 0x70,             // Specials: uDash, mDash, lDash, Space, uBox, lBox, lBorder, rBorder.
                                      0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40};                  // Segments: SegA, SegB, SegC, SegD, SegE, SegF, SegG.

/**************************/
/* Public Class Functions */
/**************************/

// Class constructor.
TM1651::TM1651(uint8_t clkPin, uint8_t dataPin, bool LEDC68) {
  _clkPin  = clkPin;                                      // Record the TM1651 clock pin.
  _dataPin = dataPin;                                     // Record the TM1651 data pin.
  _LEDC68 = LEDC68;                                       // Record if we have a Gotek LEDC68 module.
  charTableSize = (sizeof(tmCharTable) / sizeof(*tmCharTable));
}

// Set up the display and initialise it with defaults values.
void TM1651::begin(uint8_t numDigits, uint8_t brightness) {
  if(numDigits > 0 && numDigits < 5) {                    // The TM1651 supports between 1 and 4 digits.
    _numDigits = numDigits;
  }
  else {
    _numDigits = 1;
  }
  if(_numDigits != 3) {                                   // If we do not have 3 digits, we cannot have a Gotek LEDC68 module.
    _LEDC68 = false;
  }
  pinMode(_clkPin, OUTPUT);
  pinMode(_dataPin, OUTPUT);
  this->displayClear();
  this->displayBrightness(brightness);
}

// Turn the TM1651 display OFF.
void TM1651::displayOff(void) {
  cmdDispCtrl = 0x80;                                     // 0x80 = display OFF.
  this->start();                                          // Send the start signal to the TM1651.
  this->writeByte(cmdDispCtrl);                           // Turn the display OFF.
  this->stop();                                           // Send the stop signal to the TM1651.
}

// Clear all the digits in the display.
void TM1651::displayClear(void) {
  uint8_t digitCounter;
  for(digitCounter = 0; digitCounter < _numDigits; digitCounter++){
    this->displayChar(digitCounter, 0x00, true);          // Write a zero (all segments OFF) to each digit.
  }
  this->displayDP(false);
}

// Set the brightness (0x00 - 0x07) and turn the TM1651 display ON.
void TM1651::displayBrightness(uint8_t brightness) {
  _brightness = brightness & INTENSITY_MAX;               // Record the TM1651 brightness level.
  cmdDispCtrl = 0x88 + _brightness;                       // 0x88 + 0x00 to 0x07 brightness, 0x88 = display ON.
  this->start();                                          // Send the start signal to the TM1651.
  this->writeByte(cmdDispCtrl);                           // Set the brightness and turn the display ON.
  this->stop();                                           // Send the stop signal to the TM1651.
}

// Display a character in a specific LED digit.
void TM1651::displayChar(uint8_t digit, uint8_t number, bool raw) {
  if(digit > (_numDigits - 1)) {                          // Boundry check the digit number, leftmost digit is #0.
    digit = _numDigits - 1;
  }
  if(raw) {                                               // If this is a raw segment bit number, ensure there are only 7 bits.
    number &= 0x7f;
  }
  else {                                                  // If using the character table, ensure the number is within the character table.
    if(number >= charTableSize) {
      number = 0x20;                                      // This is a 0x00 (space) in the character table.
    }
    number = tmCharTable[number];                         // Get the raw number from the character table.
  }
  this->start();                                          // Send the start signal to the TM1651.
  this->writeByte(ADDR_FIXED);                            // Cmd to set a specific address.
  this->stop();                                           // Send the stop signal to the TM1651.
  this->start();
  this->writeByte(STARTADDR + digit);                     // Set the address for the requested digit.
  this->writeByte(number);                                // Write the number to the digit.
  this->stop();
}

// Display a decimal integer between 0 - 99, or a hex integer between 0x00 - 0xff, starting at a specific digit.
void TM1651::displayInt8(uint8_t digit, uint8_t number, bool useDec) {
  if(_numDigits > 1) {                                    // We need at least 2 digits to display an 8-bit number, leftmost digit is #0.
    if(digit > (_numDigits - 2)) {
      digit = _numDigits - 2;
    }
    if(useDec) {
      if(number > 99) {
        number = 99;
      }
    }
    this->start();                                        // Send the start signal to the TM1651.
    this->writeByte(ADDR_AUTO);                           // Cmd to auto increment the address.
    this->stop();                                         // Send the stop signal to the TM1651.
    this->start();
    this->writeByte(STARTADDR + digit);                   // Start at the first digit address.
    //this->writeByte(tmCharTable[0x12]);                 // Display an "h" in the lefthand digit.
    if(useDec) {
      this->writeByte(tmCharTable[(number / 10) % 10]);
      this->writeByte(tmCharTable[ number       % 10]);
    }
    else {
      this->writeByte(tmCharTable[(number / 16) % 16]);   // Display the most significant nibble.
      this->writeByte(tmCharTable[ number       % 16]);   // Display the least significant nibble.
    }
    this->stop();
  }
}

// Display a decimal integer between 0 - 999, or a hex integer between 0x000 - 0xfff, starting at a specific digit.
void TM1651::displayInt12(uint8_t digit, uint16_t number, bool useDec) {
  if(_numDigits > 2) {                                    // We need at least 3 digits to display an 12-bit number, leftmost digit is #0.
    if(digit > (_numDigits - 3)) {
      digit = _numDigits - 3;
    }
    if(useDec) {
      if(number > 999) {
        number = 999;
      }
    }
    else {
      if(number > 0xfff) {
        number = 0xfff;
      }
    }
    this->start();                                        // Send the start signal to the TM1651.
    this->writeByte(ADDR_AUTO);                           // Cmd to auto increment the address.
    this->stop();                                         // Send the stop signal to the TM1651.
    this->start();
    this->writeByte(STARTADDR + digit);                   // Start at the first digit address.
    if(useDec) {
      this->writeByte(tmCharTable[(number / 100) % 10]);
      this->writeByte(tmCharTable[(number /  10) % 10]);
      this->writeByte(tmCharTable[ number        % 10]);
    }
    else {
      this->writeByte(tmCharTable[(number / 256) % 16]);
      this->writeByte(tmCharTable[(number /  16) % 16]);
      this->writeByte(tmCharTable[ number        % 16]);
    }
    this->stop();
  }
}

// Display a decimal integer between 0 - 9999, or a hex integer between 0x0000 - 0xffff, starting at a specific digit.
void TM1651::displayInt16(uint8_t digit, uint16_t number, bool useDec) {
  if(_numDigits > 3) {                                    // We need at least 4 digits to display an 16-bit number, leftmost digit is #0.
    if(digit > (_numDigits - 4)) {
      digit = _numDigits - 4;
    }
    if(useDec) {
      if(number > 9999) {
        number = 9999;
      }
    }
    this->start();                                        // Send the start signal to the TM1651.
    this->writeByte(ADDR_AUTO);                           // Cmd to auto increment the address.
    this->stop();                                         // Send the stop signal to the TM1651.
    this->start();
    this->writeByte(STARTADDR + digit);                   // Start at the first digit address.
    if(useDec) {
      this->writeByte(tmCharTable[(number / 1000) % 10]);
      this->writeByte(tmCharTable[(number /  100) % 10]);
      this->writeByte(tmCharTable[(number /   10) % 10]);
      this->writeByte(tmCharTable[ number         % 10]);
    }
    else {
      this->writeByte(tmCharTable[(number / 4096) % 16]);
      this->writeByte(tmCharTable[(number /  256) % 16]);
      this->writeByte(tmCharTable[(number /   16) % 16]);
      this->writeByte(tmCharTable[ number         % 16]);
    }
    this->stop();
  }
}

// Turn ON/OFF the decimal points.
void TM1651::displayDP(bool status) {
  if(_LEDC68) {                                           // If we have a Gotek LEDC68 module with DP control.
    uint8_t segData;
    segData = status ? DP_ON : DP_OFF;
    this->start();                                        // Send the start signal to the TM1651.
    this->writeByte(ADDR_FIXED);                          // Cmd to set a specific address.
    this->stop();                                         // Send the stop signal to the TM1651.
    this->start();
    this->writeByte(STARTADDR + 0x03);                    // Start address +0x03 (digit #4), segment d, controls the decimal points on the Gotek LEDC68 module.
    this->writeByte(segData);                             // Write the data to turn the decimal points ON/OFF.
    this->stop();
  }
}

/***************************/
/* Private Class Functions */
/***************************/

// Write a byte of data to the TM1651.
bool TM1651::writeByte(uint8_t data) {
  bool ack;
  uint8_t bit;
  // Send 8 bits of data.
  for(bit = 0; bit < 8; bit++) {
    digitalWrite(_clkPin, LOW);
    if(data & 0x01) {                                     // LSB first.
      digitalWrite(_dataPin, HIGH);
    }
    else {
      digitalWrite(_dataPin, LOW);
    }
    data >>= 1;
    digitalWrite(_clkPin, HIGH);
  }
  // Wait for the ACK.
  digitalWrite(_clkPin, LOW);
  digitalWrite(_dataPin, HIGH);
  digitalWrite(_clkPin, HIGH);
  pinMode(_dataPin, INPUT);
  this->bitDelay(); 
  ack = digitalRead(_dataPin);
  if(ack == LOW) {                                        // ACK = LOW if the transfer was successful.
    pinMode(_dataPin, OUTPUT);
    digitalWrite(_dataPin, LOW);
  }
  this->bitDelay();
  pinMode(_dataPin, OUTPUT);
  this->bitDelay();
  return(ack);                                            // Is this even useful?
}

// Send a start signal to the TM1651.
void TM1651::start(void) {
  digitalWrite(_clkPin, HIGH);
  digitalWrite(_dataPin, HIGH);
  digitalWrite(_dataPin, LOW);
  digitalWrite(_clkPin, LOW);
}

//Send a stop signal to the TM1651.
void TM1651::stop(void) {
  digitalWrite(_clkPin, LOW);
  digitalWrite(_dataPin, LOW);
  digitalWrite(_clkPin, HIGH);
  digitalWrite(_dataPin, HIGH);
}

// Wait for a bit...
void TM1651::bitDelay(void) {
  delayMicroseconds(5);                                   // I think this might go as low as 4us (250KHz).
}

// EOF