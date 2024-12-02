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
 * *  easiTM1651 Header File  *
 * ****************************
*/

#ifndef __TM1651_h
  #define __TM1651_h
  #include <Arduino.h>

  // Command and address definitions for the TM1651.
  #define ADDR_AUTO       0x40
  #define ADDR_FIXED      0x44
  #define STARTADDR       0xc0 

  // Definitions for the decimal point but it's not well implemented in the LEDC68 hardware and is of little use.
  #define DP_OFF          0x00
  #define DP_ON           0x08                            // Controlled via start address +03, segment d.

  // Definitions for the 7 segment display brightness.
  #define INTENSITY_MIN   0x00
  #define INTENSITY_TYP   0x02
  #define INTENSITY_MAX   0x07

  // Hardware related definitions.
  #define DEF_TM_CLK      2
  #define DEF_TM_DIN      3
  #define DEF_DIGITS      3

  class TM1651 {
    public:
      // TM1651 Class instantiation.
      TM1651(uint8_t = DEF_TM_CLK, uint8_t = DEF_TM_DIN, bool = true);
      uint8_t cmdDispCtrl;                                // The current display control command.
      uint8_t charTableSize;                              // The size of the defined character code table.
      // Set up the display and initialise it with default values.
      void begin(uint8_t = DEF_DIGITS, uint8_t = INTENSITY_TYP);
      void displayOff(void);                              // Turn the TM1651 display OFF.
      void displayClear(void);                            // Clear all the digits in the display.
      void displayBrightness(uint8_t = INTENSITY_TYP);    // Set the brightness and turn the TM1651 display ON.
      void displayChar(uint8_t, uint8_t, bool = false);   // Display a character in a specific digit.
      void displayInt8(uint8_t, uint8_t, bool = true);    // Display a decimal integer between 0 - 99, or a hex integer between 0x00 - 0xff, starting at a specific digit.
      void displayInt12(uint8_t, uint16_t, bool = true);  // Display a decimal integer between 0 - 999, or a hex integer between 0x000 - 0xfff, starting at a specific digit.
      void displayInt16(uint8_t, uint16_t, bool = true);  // Display a decimal integer between 0 - 9999, or a hex integer between 0x0000 - 0xffff, starting at a specific digit.
      void displayDP(bool = false);                       // Turn ON/OFF the decimal points.
    private:
      bool _LEDC68;                                       // Flag if we have a Gotek LEDC68 module - affects only the decimal point control.
      uint8_t _clkPin;                                    // The TM1651 clock pin.
      uint8_t _dataPin;                                   // The TM1651 data pin.
      uint8_t _numDigits;                                 // The number of TM1651 digits.
      uint8_t _brightness;                                // The current TM1651 display brightness.
      bool writeByte(uint8_t);                            // Write a byte of data to the TM1651.
      void start(void);                                   // Send a start signal to the TM1651.
      void stop(void);                                    // Send a stop signal to the TM1651.
      void bitDelay(void);                                // Wait for a bit...
  };
#endif

// EOF