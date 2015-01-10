/**
 * MIDI Show Control
 * MSC routines implementation
 *
 * This file contains the code for interpreting MSC commands.
 *
 * Last modified January 9, 2015
 *
 * Copyright (C) 2015. All Rights Reserved.
 */

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "msc.h"

/******************************************************************************
 * Internal constants
 ******************************************************************************/

#define SYSEX_START_BYTE        0xF0
#define MIDI_RTS_BYTE           0x7F
#define MSC_BYTE                0x02
#define SYSEX_END_BYTE          0xF7

/******************************************************************************
 * Function definitions
 ******************************************************************************/

MSC::MSC(const byte* packet, int len) {
  data = packet;
  length = len;

  if (data[0] != SYSEX_START_BYTE) return;
  if (data[1] != MIDI_RTS_BYTE) return;
  id = data[2];
  if (data[3] != MSC_BYTE) return;
  type = (TYPE)data[4];
  command = (COMMAND)data[5];

  //Cue number is optional
  if (len > 7) {
    //Copy the cue string out of the data
    cue = new char[len - 7];
    memcpy(cue, data + 6, len - 6);
    cue[len - 7] = 0;
  }
}

MSC::~MSC() {
  delete cue;
  //Don't delete data[] because it was allocated in MIDI class
}

byte MSC::getID() {
  return id;
}

TYPE MSC::getType() {
  return type;
}

COMMAND MSC::getCommand() {
  return command;
}

char* MSC::getCue() {
  return cue;
}

byte MSC::getList() {
  return list;
}

const byte* MSC::getData() {
  return data;
}

int MSC::getLength() {
  return length;
}
