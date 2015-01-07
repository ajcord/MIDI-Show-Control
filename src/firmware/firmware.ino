/**
 * MIDI Show Control
 * Arduino firmware v0.0.1
 *
 * This file contains the code that processes received commands and generally
 * manages the Arduino.
 *
 * Last modified January 6, 2015
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

//Colors
#define BACKLIGHT_OFF               0x000000
#define RED                         0xff0000
#define GREEN                       0x00ff00
#define BLUE                        0x0000ff

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
               COMMAND command,
               bool paused);
void displayCue(long cue);
void displayList(int list);
void displayType(TYPE type);
void displayID(int id);
void displayPacket(char* packet);

void setBacklight(int red, int green, int blue);
void setBacklight(int rgb);
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
 *  Sets up MIDI, the LCD, etc.
 *
 *  Note: This function is called once at power up.
 */
void setup() {
  //Setup the MIDI serial
  MIDI.begin();

  //Setup the LCD
  setupLCD();

  //Setup the rest of the pins
  pinMode(BUTTON_PIN, INPUT);
  pinMode(BUTTON_LED_PIN, OUTPUT);

  //Setup the button pin as an interrupt
  attachInterrupt(0, buttonInterrupt, FALLING);
}

/**
 *  Note: This function is called in a forever loop in main().
 */
void loop() {
  MIDI.read();
}

/**
 *  Sets up the user interface on the LCD
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

  setBacklight(BACKLIGHT_OFF);
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
  displayCue(cue);
  displayList(list);
  displayType(type);
  displayID(id);
  displayPacket(packet);
}

/**
 *  Displays the cue number
 *  @param cue The cue number in the format it was received via MIDI
 */
void displayCue(long cue) {
  //TODO: Convert the cue number to a string
  LCD.setCursor(0, 5);
  LCD.print(cue);
}

/**
 *  Displays the list number
 *  @param list The list number
 */
void displayList(int list) {
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
void displayID(int id) {
  LCD.setCursor(1, 18);
  LCD.print(id, HEX);
}

/**
 *  Displays the packet
 *  @param packet The packet bytes received
 */
void displayPacket(char* packet) {
  for (int i = 0; ; i++) {
    //Print each byte in the packet in hex
    LCD.setCursor(2, i*2);
    LCD.print(packet[i], HEX);

    //Stop when we have printed the stop byte
    if (packet[i] == 0xF7) break;
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

/**
 *  Handles button presses
 */
void buttonInterrupt() {
  paused = !paused;
  if (paused) {
    pauseMIDI();
  } else {
    passMIDI();
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