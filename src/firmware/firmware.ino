/**
 * MIDI Show Control
 * Arduino firmware v0.0.1
 *
 * This file contains the code that processes received commands and generally
 * manages the Arduino.
 *
 * Last modified January 10, 2015
 *
 * Copyright (C) 2015. All Rights Reserved.
 */

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "Arduino.h"
#include "LiquidCrystal.h"
#include "MIDI.h"
#include "midi_Namespace.h"
#include "msc.h"

/******************************************************************************
 * Internal constants
 ******************************************************************************/

//MIDI
//https://www.sparkfun.com/products/9598
#define MIDI_IN_PIN                 0
#define MIDI_OUT_PIN                1

//LCD
//http://learn.adafruit.com/character-lcds
#define LCD_RED_BACKLIGHT_PIN       3
#define LCD_GREEN_BACKLIGHT_PIN     5
#define LCD_BLUE_BACKLIGHT_PIN      6
#define LCD_CONTROL_PIN             7
#define LCD_ENABLE_PIN              8
#define LCD_DATA_BIT_4_PIN          9
#define LCD_DATA_BIT_5_PIN          10
#define LCD_DATA_BIT_6_PIN          11
#define LCD_DATA_BIT_7_PIN          12
#define LCD_COLUMNS                 20
#define LCD_ROWS                    4
#define SYSEX_FLASH_TIME            500 //milliseconds

//Colors (0xRRGGBB)
#define BACKLIGHT_OFF               0x000000
#define RED                         0xff0000
#define GREEN                       0x00ff00
#define BLUE                        0x0000ff

//Pushbutton
#define BUTTON_LED_PIN              13
#define BUTTON_PIN                  2
#define DEBOUNCE_TIME               200 //milliseconds
#define NORMALLY_OPEN               0
#define NORMALLY_CLOSED             1

//Button configuration (compile-time option)
#define BUTTON_MODE                 NORMALLY_OPEN

#if BUTTON_MODE == NORMALLY_OPEN
#define BUTTON_DOWN                 RISING
#define BUTTON_UP                   FALLING
#else
#define BUTTON_DOWN                 FALLING
#define BUTTON_UP                   RISING
#endif

/******************************************************************************
 * Internal function prototypes
 ******************************************************************************/

void setupLCD();
void updateLCD(MSC packet);
void displayCue(char* cue);
void displayList(byte list);
void displayType(TYPE type);
void displayID(byte id);
void displayPacket(const byte* data, int len);

void setBacklight(int red, int green, int blue);
void setBacklight(int rgb);

void setupButton();
void buttonInterrupt();
void pauseMIDI();
void passMIDI();

/******************************************************************************
 * Internal global variables
 ******************************************************************************/

//Create the global LCD object
LiquidCrystal LCD(LCD_CONTROL_PIN,
                  LCD_ENABLE_PIN,
                  LCD_DATA_BIT_4_PIN,
                  LCD_DATA_BIT_5_PIN,
                  LCD_DATA_BIT_6_PIN,
                  LCD_DATA_BIT_7_PIN);

//Create the global MIDI object
MIDI_CREATE_DEFAULT_INSTANCE();

//Whether MIDI passthrough is paused or not
volatile bool paused = false;

/******************************************************************************
 * Function definitions
 ******************************************************************************/

/**
 *  Sets up MIDI, the LCD, and the button
 */
void setup() {
  MIDI.begin();

  setupLCD();

  setupButton();
}

/**
 *  Checks for new MIDI data, parses it, and updates the LCD accordingly
 */
void loop() {
  static long lastSysExTime = 0;
  static bool lcdIsBlue = false;

  if (MIDI.read()) {
    //Flash the LCD blue
    lastSysExTime = millis();
    setBacklight(BLUE);
    lcdIsBlue = true;

    //Get the MSC data and update the LCD
    MSC parsedData(MIDI.getSysExArray(), MIDI.getSysExArrayLength());
    updateLCD(parsedData);
  }

  //Check whether the display has been blue for long enough
  if (lcdIsBlue && millis() - lastSysExTime > SYSEX_FLASH_TIME) {
    //Decide which color to revert to
    if (paused) {
      setBacklight(RED);
    } else {
      setBacklight(GREEN);
    }
    lcdIsBlue = false;
  }
}



///////////////////////////////   LCD   ////////////////////////////////////////

/**
 *  Sets up the user interface on the LCD
 */
void setupLCD() {
  //Prepare the LCD
  LCD.begin(LCD_COLUMNS, LCD_ROWS);
  LCD.noDisplay();
  LCD.clear();

  //Set LCD options
  LCD.noCursor();
  LCD.noAutoscroll();

  //Display the user interface
  LCD.print("CUE#:        LIST:  ");
  LCD.setCursor(0, 1); //Newline
  LCD.print("TYPE:          ID:  ");

  //Setup the backlight
  pinMode(LCD_RED_BACKLIGHT_PIN, OUTPUT);
  pinMode(LCD_GREEN_BACKLIGHT_PIN, OUTPUT);
  pinMode(LCD_BLUE_BACKLIGHT_PIN, OUTPUT);
  setBacklight(BACKLIGHT_OFF);

  //Turn on the display
  LCD.display();
}

/**
 *  Updates the LCD user interface with new values
 *  @param packet The parsed MSC packet
 */
void updateLCD(MSC packet) {
  displayCue(packet.getCue());
  displayList(packet.getList());
  displayType(packet.getType());
  displayID(packet.getID());
  displayPacket(packet.getData(), packet.getLength());
}

/**
 *  Displays the cue number
 *  @param cue The formatted, ASCII cue number
 */
void displayCue(char* cue) {
  LCD.setCursor(0, 5);
  LCD.print(cue);
}

/**
 *  Displays the list number
 *  @param list The list number
 */
void displayList(byte list) {
  LCD.setCursor(0, 18);
  LCD.print(list, HEX);
}

/**
 *  Displays the type
 *  @param type The type of command that was received
 */
void displayType(TYPE type) {
  LCD.setCursor(1, 5);
  switch(type) {
    case LIGHT:
      LCD.print("LIGHT    ");
      break;
    case SOUND:
      LCD.print("SOUND    ");
      break;
    case FIREWORKS:
      LCD.print("FIREWORKS");
      break;
    case ALL:
      LCD.print("ALL      ");
      break;
  }
}

/**
 *  Displays the ID
 *  @param id The ID
 */
void displayID(byte id) {
  LCD.setCursor(1, 18);
  LCD.print(id, HEX);
}

/**
 *  Displays the packet
 *  @param packet The packet bytes received
 */
void displayPacket(const byte* data, int len) {
  for (int i = 0; i < len && i < LCD_COLUMNS/2; i++) {
    //Print each byte in the packet in hex
    LCD.setCursor(2, i*2);
    LCD.print((char)data[i], HEX);
  }

  //Indicate if the packet is too long to fit on the screen
  if (len > LCD_COLUMNS/2) {
    LCD.setCursor(2, LCD_COLUMNS - 2);
    LCD.print(">>");
  }
}

/**
 *  Sets the backlight color of the LCD
 *
 *  @param red    The red value of the backlight
 *  @param green  The green value of the backlight
 *  @param blue   The blue value of the backlight
 */
void setBacklight(int red, int green, int blue) {
  analogWrite(LCD_RED_BACKLIGHT_PIN, red);
  analogWrite(LCD_GREEN_BACKLIGHT_PIN, green);
  analogWrite(LCD_BLUE_BACKLIGHT_PIN, blue);
}

/**
 *  Sets the backlight color of the LCD
 *
 *  @param rgb    The red, green, and blue values of the backlight
 *                encoded in hex as RRGGBB
 */
void setBacklight(int rgb) {
  setBacklight((rgb >> 16) & 0xff, (rgb >> 8) & 0xff, rgb & 0xff);
}



/////////////////////////////   Button   ///////////////////////////////////////

/**
 *  Sets up the pause button
 */
void setupButton() {
  //Setup the button pins
  pinMode(BUTTON_PIN, INPUT);
  pinMode(BUTTON_LED_PIN, OUTPUT);
  digitalWrite(BUTTON_LED_PIN, HIGH); //Can also use analogWrite for PWM

  //Attach an interrupt to the button pin to listen for presses
  attachInterrupt(0, buttonInterrupt, BUTTON_DOWN);
}

/**
 *  Handles button press events, including debouncing
 */
void buttonInterrupt() {
  //The time in milliseconds of the last button press
  static long lastButtonPress = 0;

  if (millis() - lastButtonPress > DEBOUNCE_TIME) {
    lastButtonPress = millis();
    paused = !paused;
    if (paused) {
      pauseMIDI();
    } else {
      passMIDI();
    }
  }
}

/**
 *  Disables MIDI passthrough
 */
void pauseMIDI() {
  MIDI.turnThruOff();

  LCD.setCursor(2, 8);
  LCD.print("-MSC*PAUSED*");

  setBacklight(RED);
}

/**
 *  Enables MIDI passthrough
 */
void passMIDI() {
  MIDI.turnThruOn();

  LCD.setCursor(2, 8);
  LCD.print("-MSC-PASS >>");

  setBacklight(GREEN);
}
