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

  ///////////////////////////  Cue string ///////////////////////////
  byte* p = (byte*)data + 6; //Move past the start bytes
  byte* cue_ptr = p;

  //Walk along until we get to the end of the string
  while (*p && *p != SYSEX_END_BYTE) {
    p++;
  }

  int cue_len = p - cue_ptr;
  if (cue_len > MAX_CUE_LEN) {
    cue_len = MAX_CUE_LEN; //Cap the length of the string
  }

  memset(cue, ' ', MAX_CUE_LEN); //Clear out previous contents

  //Figure out whether the cue is a string or a hex number
  if (cue_ptr[0] >= ' ' && cue_ptr[0] <= '~') {
    //Probably a string
    memcpy(cue, cue_ptr, cue_len); //Copy cue string
  } else {
    //Probably a number
    cue_len *= 2; //Each hex digit takes up two character slots
    if (cue_len > MAX_CUE_LEN) {
      cue_len = MAX_CUE_LEN; //Cap the length of the string
    }
    for (int i = 0; i < cue_len; i += 2) {
      char c = cue_ptr[i/2];
      //Convert the hex digit to ASCII
      char lo = c & 0x0f;
      char hi = (c & 0xf0) >> 4;
      cue[i] = hi + (hi < 0x0a ? '0' : 'A' - 0x0a);
      cue[i+1] = lo + (lo < 0x0a ? '0' : 'A' - 0x0a);
    }
  }
  cue[MAX_CUE_LEN] = 0; //Terminate

  ///////////////////////////  List string ///////////////////////////
  if (!*p) {
    p++; //Move past the delimiter
  }
  byte* list_ptr = p;

  //Walk along until we get to the end of the string
  while (*p && *p != SYSEX_END_BYTE) {
    p++;
  }

  int list_len = p - list_ptr;
  if (list_len > MAX_LIST_LEN) {
    list_len = MAX_LIST_LEN; //Cap the length of the string
  }

  memset(list, ' ', MAX_LIST_LEN); //Clear out previous contents
  memcpy(list, list_ptr, list_len); //Copy list string
  list[MAX_LIST_LEN] = 0; //Terminate

  if (cue_len == 0) {
    //There was no cue. Display NEXT instead.
    memcpy(cue, "NEXT", 4);
  }

  if (list_len == 0) {
    //There was no list. Display NONE instead.
    memcpy(list, "NONE", 4);
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
