#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"
#include "GENERATEDwuclass_grove_mp3.h"
#include "MP3_wt5001.h"

void wuclass_grove_mp3_setup(wuobject_t *wuobject) {
  int16_t pin;
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_GROVE_MP3___PIN, &pin);
  DEBUG_LOG(DBG_WKPFUPDATE,"WKPFUPDATE(Grove_MP3): pin:%d\n", pin);

  _WT5001(0);
  
  if (!_setupTty(B9600))
  {
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Grove_MP3): Failed to setup tty port parameters\n");
  }
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Grove_MP3): setup\n");

  uint8_t vol = 0;
  if (_getVolume(&vol)){
    DEBUG_LOG(DBG_WKPFUPDATE,"WKPFUPDATE(Grove_MP3): The current volume is %d\n", (int)vol);
  }
  uint16_t numf = 0;
  if (_getNumFiles(SD, &numf)){
    DEBUG_LOG(DBG_WKPFUPDATE,"WKPFUPDATE(Grove_MP3): The numbere of files on the SD card is %d\n", (int)numf);
  }
  uint16_t year = 0;
  uint8_t month = 0, day = 0;
  if (_getDate(&year, &month, &day)){
    DEBUG_LOG(DBG_WKPFUPDATE,"WKPFUPDATE(Grove_MP3): The device date is %d/%d/%d\n", (int)month, (int)day, (int)year);
  }

  uint8_t hour = 0, minute = 0, second = 0;
  if (_getTime(&hour, &minute, &second)){
    DEBUG_LOG(DBG_WKPFUPDATE,"WKPFUPDATE(Grove_MP3): The device time is %d:%d:%d\n", (int)hour, (int)minute, (int)second);
  }
  uint16_t curf = 0;
  if (_getCurrentFile(&curf)){
    DEBUG_LOG(DBG_WKPFUPDATE,"WKPFUPDATE(Grove_MP3): The current file is %d\n", (int)curf);
  }

}

void wuclass_grove_mp3_update(wuobject_t *wuobject) {
  _WT5001(0);
  
  if (!_setupTty(B9600))
  {
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Grove_MP3): Failed to setup tty port parameters\n");
  }
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Grove_MP3): setup\n");

  uint8_t vol = 0;
  if (_getVolume(&vol)){
    DEBUG_LOG(DBG_WKPFUPDATE,"WKPFUPDATE(Grove_MP3): The current volume is %d\n", (int)vol);
  }
  uint16_t numf = 0;
  if (_getNumFiles(SD, &numf)){
    DEBUG_LOG(DBG_WKPFUPDATE,"WKPFUPDATE(Grove_MP3): The numbere of files on the SD card is %d\n", (int)numf);
  }
  uint16_t year = 0;
  uint8_t month = 0, day = 0;
  if (_getDate(&year, &month, &day)){
    DEBUG_LOG(DBG_WKPFUPDATE,"WKPFUPDATE(Grove_MP3): The device date is %d/%d/%d\n", (int)month, (int)day, (int)year);
  }

  uint8_t hour = 0, minute = 0, second = 0;
  if (_getTime(&hour, &minute, &second)){
    DEBUG_LOG(DBG_WKPFUPDATE,"WKPFUPDATE(Grove_MP3): The device time is %d:%d:%d:n", (int)hour, (int)minute, (int)second);
  }
  uint16_t curf = 0;
  if (_getCurrentFile(&curf)){
    DEBUG_LOG(DBG_WKPFUPDATE,"WKPFUPDATE(Grove_MP3): The current file is %d\n", (int)curf);
  }

  int16_t track;
  bool on_off;
  uint8_t ps;
  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_GROVE_MP3_ON_OFF, &on_off);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_GROVE_MP3_TRACK, &track);
  if(on_off){
    if(_getPlayState(&ps)){
	  _play(SD, track);
      DEBUG_LOG(DBG_WKPFUPDATE,"WKPFUPDATE(Grove_MP3): playstate %d\n",ps);
    }else{
      DEBUG_LOG(DBG_WKPFUPDATE,"WKPFUPDATE(Grove_MP3): Failed to get playstate\n");
    }
  }
  DEBUG_LOG(DBG_WKPFUPDATE,"WKPFUPDATE(Grove_MP3): on_off:%d track:%d\n", on_off, track);
}
