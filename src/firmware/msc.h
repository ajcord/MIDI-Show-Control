/**
 * MIDI Show Control
 * MSC routines header
 *
 * This file contains the external defines and prototypes for interpreting
 * MSC commands.
 *
 * Last modified January 9, 2015
 *
 * Copyright (C) 2015. All Rights Reserved.
 */

#ifndef MSC_H
#define MSC_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "Arduino.h"

/******************************************************************************
 * External constants
 ******************************************************************************/

/**
 *  Stores the endpoint type of an MSC message
 */
typedef enum TYPE_TAG {
  LIGHT = 0x01,
  SOUND = 0x10,
  FIREWORKS = 0x61,
  ALL = 0x7f
} TYPE;

/**
 *  Stores the command type of an MSC message
 */
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
 * Class definition
 ******************************************************************************/

class MSC {
public:
  MSC(const byte* packet, int len);
  ~MSC();

  byte getID();
  TYPE getType();
  COMMAND getCommand();
  char* getCue();
  byte getList();
  const byte* getData();
  int getLength();

private:
  byte id;
  TYPE type;
  COMMAND command;
  char* cue;
  byte list;
  const byte* data;
  int length;
};

#endif