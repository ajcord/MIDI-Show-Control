/**
 * MIDI Show Control
 * MSC routines implementation
 *
 * This file contains the code for interpreting MSC commands.
 *
 * Last modified January 10, 2015
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
    byte* cue_ptr = (byte*)data + 6;
    byte* p = cue_ptr;

    while (*p != SYSEX_END_BYTE) {
      p++;
    }
    int cue_len = p - cue_ptr;

    cue = new char[cue_len + 1];
    memcpy(cue, cue_ptr, cue_len);
    cue[cue_len] = 0;

    if (cue_len + 6 < len) {
      //There must be additional data: the list. Read on.
      byte* list_ptr = p;
    
      while (*p && *p != SYSEX_END_BYTE) {
        p++;
      }
      int list_len = p - list_ptr;

      list = new char[list_len + 1];
      memcpy(list, list_ptr, list_len);
      list[list_len] = 0;
    }
  } else {
    cue = "(next)";
    list = 0;
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

char* MSC::getList() {
  return list;
}

const byte* MSC::getData() {
  return data;
}

int MSC::getLength() {
  return length;
}
