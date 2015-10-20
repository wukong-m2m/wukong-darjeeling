#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"
#include "GENERATEDwuclass_sonos.h"
#include "djtimer.h"

#define COMMAND_DELAY 500000
#define LOOP_DELAY    500

char sonos_playlist_api[5][255]={"http://192.168.4.105:5005/playlist/Pop", "http://192.168.4.105:5005/playlist/Soundtrack", "http://192.168.4.105:5005/playlist/Jazz", "http://192.168.4.105:5005/playlist/NewAge", "http://192.168.4.105:5005/playlist/Classic"};
char sonos_volume_api[1][255]={"http://192.168.4.105:5005/volume/%d"};
char sonos_play_api[1][255]={"http://192.168.4.105:5005/play"};
char sonos_pause_api[1][255]={"http://192.168.4.105:5005/pause"};

void sonos_command_function(char *command){
  char buffer[255]={};
  snprintf(buffer, 255, "curl %s", command);
  DEBUG_LOG(DBG_WKPFUPDATE,"WKPFUPDATE(Sonos): %s\n", buffer);
  system(buffer);
  usleep(COMMAND_DELAY);
}

void wuclass_sonos_setup(wuobject_t *wuobject) {
  sonos_command_function(sonos_playlist_api[0]);
  sonos_command_function(sonos_pause_api[0]);
}

void wuclass_sonos_update(wuobject_t *wuobject) {
  static bool on_off = false, last_on_off = false;
  static int16_t playlist = 1, last_playlist = 1;
  static int16_t volume = 0, last_volume = 0;
  char buf[255]={};           
                     
  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_SONOS_ON_OFF, &on_off);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_SONOS_PLAYLIST, &playlist);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_SONOS_VOLUME, &volume);
  DEBUG_LOG(DBG_WKPFUPDATE,"WKPFUPDATE(Sonos): onoff %d, playlisty %d, volume %d\n",on_off, playlist, volume);

  //static uint32_t currenttime, lasttime;
  //currenttime = dj_timer_getTimeMillis();
  //if (currenttime - lasttime > LOOP_DELAY){
  //}

  if(on_off != last_on_off){
    if(on_off){
      sonos_command_function(sonos_play_api[0]);
      DEBUG_LOG(DBG_WKPFUPDATE,"WKPFUPDATE(Sonos): on_off %d\n",on_off);
    }else{
      sonos_command_function(sonos_pause_api[0]);
      DEBUG_LOG(DBG_WKPFUPDATE,"WKPFUPDATE(Sonos): on_off %d\n",on_off);
    }
    last_on_off = on_off;
  }
  
  if(last_playlist != playlist && playlist >= 1 && playlist <= 5){
    last_playlist = playlist;
    playlist -= 1;
    sonos_command_function(sonos_playlist_api[playlist]);
    DEBUG_LOG(DBG_WKPFUPDATE,"WKPFUPDATE(Sonos): playlist %d\n",playlist);
  }
  int diff = last_volume - volume;
  diff = diff>0 ? diff : (-1)*diff; 
  if(diff > 5 && volume >= 0 && volume <= 255){
    last_volume = volume;
    volume = (int16_t)((volume / 255.0) * 100); 
    sprintf(buf, sonos_volume_api[0], volume);
    sonos_command_function(buf);
    DEBUG_LOG(DBG_WKPFUPDATE,"WKPFUPDATE(Sonos): volume %d\n",volume);
  }

  //lasttime = currenttime;
}
