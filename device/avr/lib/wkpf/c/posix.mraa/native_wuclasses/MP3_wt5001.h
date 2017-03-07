#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>


#define WT5001_DEFAULT_UART 0
#define WT5001_MAX_VOLUME   31

// protocol start and end codes
#define WT5001_START        0x7e
#define WT5001_END          0x7e

// WT5001 opcodes
typedef enum {     NONE             = 0x00,
		   PLAY_SD          = 0xa0,
		   PLAY_SPI         = 0xa1,
		   PLAY_UDISK       = 0xa2,
		   PAUSE            = 0xa3,
		   STOP             = 0xa4,
		   NEXT             = 0xa5,
		   PREVIOUS         = 0xa6,
		   SET_VOLUME       = 0xa7,
		   QUEUE            = 0xa8,
		   PLAY_MODE        = 0xa9,
		   COPY_SD2FLASH    = 0xaa, // not implemented
		   COPY_UDISK2FLASH = 0xab, // not implemented
		   INSERT_SONG      = 0xac,
		   SET_DATE         = 0xb1,
		   SET_TIME         = 0xb2,
		   SET_ALARM        = 0xb3,
		   SET_ALARM_DUR    = 0xb4, // not implemented
		   CLEAR_ALARM      = 0xb5,
		   CLEAR_ALARM_DUR  = 0xb6, // not implemented
		   READ_VOLUME      = 0xc1,
		   READ_PLAY_STATE  = 0xc2,
		   READ_SPI_NUMF    = 0xc3,
		   READ_SD_NUMF     = 0xc4,
		   READ_UDISK_NUMF  = 0xc5,
		   READ_CUR_FNAME   = 0xc6,
		   READ_CF_CHAR     = 0xc7, // not implemented
		   READ_DATE        = 0xd1,
		   READ_TIME        = 0xd2
} WT5001_OPCODE_T;

// play modes
typedef enum { 	   NORMAL           = 0x00,
                   SINGLE_REPEAT    = 0x01,
                   ALL_REPEAT       = 0x02,
                   RANDOM           = 0x03
} WT5001_PLAYMODE_T;

// music source
typedef enum {     SD,
                   SPI,
                   UDISK
} WT5001_PLAYSOURCE_T;

void _WT5001(int uart);

bool _dataAvailable(unsigned int millis);

int _readData(char *buffer, size_t len);

int _writeData(char *buffer, size_t len);

bool _setupTty(speed_t baud);

bool _checkResponse(WT5001_OPCODE_T opcode);

bool _play(WT5001_PLAYSOURCE_T psrc, uint16_t index);

bool _stop();

bool _pause();

bool _next();

bool _previous();

/**
* set the volume. Range is between 0-31.  0 means mute.
*
* @return true if successful
*/
bool _setVolume(uint8_t vol);

/**
* queue a track to play next, when current song is finished
*
* @param index file number to queue
* @return true if successful
*/
bool _queue(uint16_t index);

/**
* set the playback mode
*
* @param pm play mode to enable
* @return true if successful
*/
bool _setPlayMode(WT5001_PLAYMODE_T pm);

/**
* insert a track to play immediately, interrupting the current
* track.  When the inserted track is finished playing, the
* interrupted track will resume where it was interrupted.
*
* @param index file number to insert
* @return true if successful
*/
bool _insert(uint16_t index);

/**
* set the date of the internal clock
*
* @param year 4 digit year
* @param month the month
* @param day the day
* @return true if successful
*/
bool _setDate(uint16_t year, uint8_t month, uint8_t day);

/**
* set the time of the internal clock
*
* @param hour hour
* @param minute minute
* @param second second
* @return true if successful
*/
bool _setTime(uint8_t hour, uint8_t minute, uint8_t second);

/**
* set the alarm
*
* @param hour hour
* @param minute minute
* @param second second
* @return true if successful
*/
bool _setAlarm(uint8_t hour, uint8_t minute, uint8_t second);

/**
* clear any alarm that has been set
*
* @return true if successful
*/
bool _clearAlarm();

/**
* get the current volume
*
* @param vol the returned volume
* @return true if successful
*/
bool _getVolume(uint8_t *vol);

/**
* get the current play state. 1 = playing, 2 = stopped, 3 = paused
*
* @param ps the returned play state
* @return true if successful
*/
bool _getPlayState(uint8_t *ps);

/**
* get the number of files present on the source device
*
* @param psrc the storage source
* @param numf the returned number of files
* @return true if successful
*/
bool _getNumFiles(WT5001_PLAYSOURCE_T psrc, uint16_t *numf);

/**
* get the index of the current file
*
* @param curf the index of the current file
* @return true if successful
*/
bool _getCurrentFile(uint16_t *curf);

/**
* get the device date
*
* @param year returned 4 digit year
* @param month returned month
* @param day returned day
* @return true if successful
*/
bool _getDate(uint16_t *year, uint8_t *month, uint8_t *day);

/**
* get the device time
*
* @param hour returned hour
* @param minute returned minute
* @param second returned second
* @return true if successful
*/
bool _getTime(uint8_t *hour, uint8_t *minute, uint8_t *second);
