#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "config.h"

#ifdef MRAA_LIBRARY
#include "MOSFET_cjq4435.h"

#define NONE        0
#define START       1
#define NEXT        2
#define PREVIOUS    3
#define PAUSE       4
#define STOP        5
#define RESTART     6


void wuclass_music_led_setup(wuobject_t *wuobject) {
    CJQ4435_Init(5);
    setPeriodMS(10);
    enable(true);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(MUSIC_LED): ready\n");
}

void wuclass_music_led_update(wuobject_t *wuobject) {
    static FILE *fp;
    static char filename[30] = {};
    static int track = 1;
    static bool pause = false;
    static bool playing = false;

    int16_t comm, inst;
    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_MUSIC_LED_COMMAND, &comm);
    wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_MUSIC_LED_INSTRUMENT, &inst);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(MUSIC_LED): start %d\n", comm);
    if (comm == START) {
        if (pause) {
            pause = false;
            playing = true;
        } else if(!playing){
            if (fp != NULL)
                fclose(fp);
            snprintf(filename, sizeof(filename), "../csv/%d_%d.csv", inst, track);
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(MUSIC_LED): file name: %s\n", filename);
            fp = fopen(filename, "r");
            playing = true;
            if (fp == NULL) {
                DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(MUSIC_LED): play error\n");
                playing = false;
            }
        }
    } else if (comm == NEXT) {
        if (fp != NULL)
            fclose(fp);
        track++;
        playing = true;
        pause = false;
        snprintf(filename, sizeof(filename), "../csv/%d_%d.csv", inst, track);
        fp = fopen(filename, "r");
        if (fp == NULL) {
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(MUSIC_LED): next error\n");
            track--;
            playing = false;
        }
    } else if (comm == PREVIOUS) {
        if (fp != NULL)
            fclose(fp);
        track--;
        playing = true;
        pause = false;
        snprintf(filename, sizeof(filename), "../csv/%d_%d.csv", inst, track);
        fp = fopen(filename, "r");
        if (fp == NULL) {
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(MUSIC_LED): pervious error\n");
            track++;
            playing = false;
            return;
        }
    } else if (comm == PAUSE) {
        pause = true;
        playing = false;
    } else if (comm == STOP) {
        if (fp != NULL)
            fclose(fp);
        fp = NULL;
        playing = false;
        pause = false;
    } else if (comm == RESTART) {
        if (fp != NULL)
            fclose(fp);
        track = 1;
        playing = true;
        pause = false;
        snprintf(filename, sizeof(filename), "../csv/%d_%d.csv", inst, track);
        fp = fopen(filename, "r");
        if (fp == NULL) {
            DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(MUSIC_LED): restart error\n");
            playing = false;
        }
    }
    if (comm != NONE)
        wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_MUSIC_LED_COMMAND, NONE);
    if (playing && fp != NULL) {
        int amplitude;
        if (fscanf(fp, "%d", &amplitude) != 1) {
            playing = false;
            fclose(fp);
            fp = NULL;
            amplitude = 0;
        }
        setDutyCycle(amplitude/255.0);
        DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(MUSIC_LED): %d\n", amplitude);
    }
}
#endif