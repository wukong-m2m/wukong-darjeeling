#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "config.h"
#include "MP3_wt5001.h"

#define NONE        0
#define START       1
#define NEXT        2
#define PREVIOUS    3
#define PAUSE       4
#define STOP        5
#define RESTART     6

uint16_t numf = 9;

void wuclass_gesture_mp3_setup(wuobject_t *wuobject) {

    _WT5001(0);

    if (!_setupTty(B9600))
    {
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Gesture_MP3): Failed to setup tty port parameters\n");
    }
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Gesture_MP3): setup\n");

    uint8_t vol = 0;
    if (_getVolume(&vol)){
        DEBUG_LOG(DBG_WKPFUPDATE,"WKPFUPDATE(Gesture_MP3): The current volume is %d\n", (int)vol);
    }
    if (_getNumFiles(SD, &numf)){
        DEBUG_LOG(DBG_WKPFUPDATE,"WKPFUPDATE(Gesture_MP3): The numbere of files on the SD card is %d\n", (int)numf);
    }
    uint16_t year = 0;
    uint8_t month = 0, day = 0;
    if (_getDate(&year, &month, &day)){
        DEBUG_LOG(DBG_WKPFUPDATE,"WKPFUPDATE(Gesture_MP3): The device date is %d/%d/%d\n", (int)month, (int)day, (int)year);
    }

    uint8_t hour = 0, minute = 0, second = 0;
    if (_getTime(&hour, &minute, &second)){
        DEBUG_LOG(DBG_WKPFUPDATE,"WKPFUPDATE(Gesture_MP3): The device time is %d:%d:%d\n", (int)hour, (int)minute, (int)second);
    }
    uint16_t curf = 0;
    if (_getCurrentFile(&curf)){
        DEBUG_LOG(DBG_WKPFUPDATE,"WKPFUPDATE(Gesture_MP3): The current file is %d\n", (int)curf);
    }
}

void wuclass_gesture_mp3_update(wuobject_t *wuobject) {
    static int track = 1;
    static bool pause = false;

    int16_t comm;
    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_GESTURE_MP3_COMMAND, &comm);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Gesture_MP3): start %d\n", comm);
    if (comm == START) {
        uint8_t ps;
        _getPlayState(&ps);
        if (pause) {
            _pause();
            pause = true;
        } else if(ps == 2){
            _play(SD, track);
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Gesture_MP3): playing track: %d\n", track);
        }
    } else if (comm == NEXT) {
        if (track < numf) {
            track++;
            pause = false;
            _play(SD, track);
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Gesture_MP3): playing track: %d\n", track);
        } else {
            _stop();
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Gesture_MP3): No next track.\n"); 
        }
    } else if (comm == PREVIOUS) {
        if (track > 1) {
            track--;
            pause = false;
            _play(SD, track);
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Gesture_MP3): playing track: %d\n", track);
        } else {
            _stop();
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Gesture_MP3): No previous track.\n"); 
        }
    } else if (comm == PAUSE) {
        _pause();
        pause = true;
    } else if (comm == STOP) {
        _stop();
        pause = false;
    } else if (comm == RESTART) {
        _stop();
        pause = false;
        track=1;
        _play(SD, track);
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Gesture_MP3): playing track: %d\n", track);
    }
    if (comm != NONE)
        wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_GESTURE_MP3_COMMAND, NONE);
}
