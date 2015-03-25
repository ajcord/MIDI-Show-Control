/**
 * MIDI Show Control
 * MSC routines implementation
 *
 * This file contains the code for interpreting MSC commands.
 *
 * Last modified March 25, 2015
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

    while (*p && *p != SYSEX_END_BYTE) {
      p++;
    }
    int cue_len = p - cue_ptr;
    if (cue_len > MAX_CUE_LEN) {
      cue_len = MAX_CUE_LEN;
    }

    memset(cue, ' ', MAX_CUE_LEN);
    memcpy(cue, cue_ptr, cue_len);
    cue[MAX_CUE_LEN] = 0;

    if (!*p) {
      //There must be additional data: the list. Read on.
      p++; //Move past the delimiter
      byte* list_ptr = p;
    
      while (*p && *p != SYSEX_END_BYTE) {
        p++;
      }
      int list_len = p - list_ptr;
      if (list_len > MAX_LIST_LEN) {
        list_len = MAX_LIST_LEN;
      }

      memset(list, ' ', MAX_LIST_LEN);
      memcpy(list, list_ptr, list_len);
      list[MAX_LIST_LEN] = 0;
    } else {
      memset(list, ' ', MAX_LIST_LEN);
      strcpy(list, "NONE");
    }
  } else {
    memset(cue, ' ', MAX_CUE_LEN);
    strcpy(cue, "NEXT");
    memset(list, ' ', MAX_LIST_LEN);
    strcpy(list, "NONE");
  }
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
