/**
 * MIDI Show Control
 * Arduino firmware v0.0.1
 *
 * This file contains the code that processes received commands and generally
 * manages the Arduino.
 *
 * Last modified January 4, 2015
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


//#include "firmware.h"

/******************************************************************************
 * Internal constants
 ******************************************************************************/

//MIDI
//https://www.sparkfun.com/products/9598
#define MIDI_IN_PIN                 0
#define MIDI_OUT_PIN                1
//#define MIDI_SERIAL_SPEED           31250

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

//Pushbutton
#define BUTTON_LED_PIN              13
#define BUTTON_PIN                  2 //TBD

//Custom types
typedef enum TYPE_TAG {
  LIGHT = 0x01,
  SOUND = 0x10,
  FIREWORKS = 0x61,
  ALL = 0x7f
} TYPE;

typedef enum STATUS_TAG {
  PASS,
  PAUSED
} STATUS;

typedef enum COMMAND_TAG {
  GO = 0x01,
  STOP,
  RESUME,
  TIMED_GO,
  LOAD,
  SET,
  FIRE,
  ALL_OFF,
  RESTORE,
  RESET,
  GO_OFF,
  STANDBY_PLUS = 0x11,
  STANDBY_MINUS,
  SEQUENCE_PLUS,
  SEQUENCE_MINUS
} COMMAND;

/******************************************************************************
 * Internal function prototypes
 ******************************************************************************/

void setupLCD();
void updateLCD(long cue,
               int list,
               TYPE type,
               int id,
               char* packet,
               COMMAND command);
void setBacklight(int red, int green, int blue);
void setBacklight(int rgb);

/******************************************************************************
 * Internal global variables
 ******************************************************************************/

LiquidCrystal LCD(LCD_CONTROL_PIN,
                  LCD_ENABLE_PIN,
                  LCD_DATA_BIT_4_PIN,
                  LCD_DATA_BIT_5_PIN,
                  LCD_DATA_BIT_6_PIN,
                  LCD_DATA_BIT_7_PIN);


/******************************************************************************
 * Function definitions
 ******************************************************************************/

/**
 *  Sets up MIDI, the LCD, etc.
 *
 *  Note: This function is called once at power up.
 */
void setup() {
  //FIXME: Setup the MIDI serial
  //MIDI.begin();

  //Setup the LCD
  setupLCD();

  //Setup the rest of the pins
  pinMode(BUTTON_PIN, INPUT);
  pinMode(BUTTON_LED_PIN, OUTPUT);
}

/**
 *  Note: This function is called in a forever loop in main().
 */
void loop() {

}

/**
 *  Sets up the LCD
 */
void setupLCD() {
  LCD.begin(LCD_COLUMNS, LCD_ROWS);
  
  //Prepare the LCD
  LCD.noDisplay();
  LCD.clear();

  //Set LCD options
  LCD.noCursor();
  LCD.noAutoscroll();

  //Display the user interface
  LCD.print("CUE#:        LIST:  ");
  LCD.setCursor(0, 1); //Newline
  LCD.print("TYPE:          ID:  ");

  //Display the interface
  LCD.display();

  //Setup the backlight
  pinMode(LCD_RED_BACKLIGHT_PIN, OUTPUT);
  pinMode(LCD_GREEN_BACKLIGHT_PIN, OUTPUT);
  pinMode(LCD_BLUE_BACKLIGHT_PIN, OUTPUT);
}

/**
 *  Updates the LCD user interface with new values
 *
 *  TODO: Figure out what types the cue and packet should be
 */
void updateLCD(long cue,
               int list,
               TYPE type,
               int id,
               char* packet,
               COMMAND command) {
  //Convert the cue number to a string
  LCD.setCursor(0, 5);
  LCD.print(cue);
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
