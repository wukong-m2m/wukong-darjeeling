#include "MP3_wt5001.h"

static const int defaultDelay = 100;     // max wait time for read

#include <mraa/uart.h>
mraa_uart_context m_uart;
int m_ttyFd;

void _WT5001(int uart){
  m_ttyFd = -1;

  if ( !(m_uart = mraa_uart_init(uart)) )
  {
      printf("mraa_uart_init() failed\n");
      return;
  }

  // This requires a recent MRAA (1/2015)
  const char *devPath = mraa_uart_get_dev_path(m_uart);

  if (!devPath)
  {
      printf("mraa_uart_get_dev_path() failed\n");
      return;
  }

  // now open the tty
  if ( (m_ttyFd = open(devPath, O_RDWR)) == -1)
  {
      printf("open of %s failed: \n", devPath); 
      //<< strerror(errno) << endl; //unknown
      return;
  }
}

bool _dataAvailable(unsigned int millis){
  if (m_ttyFd == -1){return false;}

  struct timeval timeout;

  // no waiting
  timeout.tv_sec = 0;
  timeout.tv_usec = millis * 1000;

  // int nfds; //unknown
  fd_set readfds;

  FD_ZERO(&readfds);

  FD_SET(m_ttyFd, &readfds);
  
  if (select(m_ttyFd + 1, &readfds, NULL, NULL, &timeout) > 0)
  {
    return true;                // data is ready
  }else{
    return false;
  }
}

int _readData(char *buffer, size_t len){
  if (m_ttyFd == -1){return(-1);}

  if (!_dataAvailable(defaultDelay)){return 0;}// timed out}

  int rv = read(m_ttyFd, buffer, len);

  if (rv < 0)
  {
    printf("read failed: \n");
    //<< strerror(errno) << endl; //unknown
  }
  return rv;
}

int _writeData(char *buffer, size_t len)
{
  if (m_ttyFd == -1){return(-1);}

  // first, flush any pending but unread input
  tcflush(m_ttyFd, TCIFLUSH);

  int rv = write(m_ttyFd, buffer, len);

  if (rv < 0)
    {
      printf("write failed: \n");
      //<< strerror(errno) << endl; //unknown
      return rv;
    }

  tcdrain(m_ttyFd);

  return rv;
}

bool _setupTty(speed_t baud)
{
  if (m_ttyFd == -1){
    return(false);
  }
  
  struct termios termio;

  // get current modes
  tcgetattr(m_ttyFd, &termio);

  // setup for a 'raw' mode.  81N, no echo or special character
  // handling, such as flow control.
  cfmakeraw(&termio);

  // set our baud rates
  cfsetispeed(&termio, baud);
  cfsetospeed(&termio, baud);

  // make it so
  if (tcsetattr(m_ttyFd, TCSAFLUSH, &termio) < 0)
    {
      printf("tcsetattr failed: \n");
      //<< strerror(errno) << endl; //unknown
      return false;
    }

  return true;
}

bool _checkResponse(WT5001_OPCODE_T opcode){
  char resp;
  char fopcode = (char)opcode;

  int rv = _readData(&resp, 1);

  // check for wrong response byte, or timeout
  if ((resp != fopcode) || rv == 0 ){
    return false;
  }

  return true;
}

bool _play(WT5001_PLAYSOURCE_T psrc, uint16_t index){
  char pkt[6];
  WT5001_OPCODE_T opcode = PLAY_SD;

  pkt[0] = WT5001_START;
  pkt[1] = 0x04;                // length

  switch (psrc)                 // src
    {
    case SD:
      opcode = PLAY_SD;
      break;

    case SPI:
      opcode = PLAY_SPI;
      break;

    case UDISK:
      opcode = PLAY_UDISK;
      break;
    }      

  pkt[2] = opcode;
  pkt[3] = (index >> 8) & 0xff; // index hi
  pkt[4] = index & 0xff;        // index lo
  pkt[5] = WT5001_END;

  _writeData(pkt, 6);

  return _checkResponse(opcode);
}

bool _stop(){
  char pkt[4];
  WT5001_OPCODE_T opcode = STOP;

  pkt[0] = WT5001_START;
  pkt[1] = 0x02;                // length
  pkt[2] = opcode;
  pkt[3] = WT5001_END;

  _writeData(pkt, 4);

  return _checkResponse(opcode);
}

bool _pause(){
  char pkt[4];
  WT5001_OPCODE_T opcode = PAUSE;

  pkt[0] = WT5001_START;
  pkt[1] = 0x02;                // length
  pkt[2] = opcode;
  pkt[3] = WT5001_END;

  _writeData(pkt, 4);

  return _checkResponse(opcode);
}

bool _next(){
  char pkt[4];
  WT5001_OPCODE_T opcode = NEXT;

  pkt[0] = WT5001_START;
  pkt[1] = 0x02;                // length
  pkt[2] = opcode;
  pkt[3] = WT5001_END;

  _writeData(pkt, 4);

  return _checkResponse(opcode);
}

bool _previous(){
  char pkt[4];
  WT5001_OPCODE_T opcode = PREVIOUS;

  pkt[0] = WT5001_START;
  pkt[1] = 0x02;                // length
  pkt[2] = opcode;
  pkt[3] = WT5001_END;

  _writeData(pkt, 4);

  return _checkResponse(opcode);
}

/**
* set the volume. Range is between 0-31.  0 means mute.
*
* @return true if successful
*/
bool _setVolume(uint8_t vol){
  if (vol > WT5001_MAX_VOLUME)
  {
    printf("volume must be between 0 and %d\n", WT5001_MAX_VOLUME);
    return false;
  }
  
  char pkt[5];
  WT5001_OPCODE_T opcode = SET_VOLUME;

  pkt[0] = WT5001_START;
  pkt[1] = 0x03;                // length
  pkt[2] = opcode;
  pkt[3] = vol;
  pkt[4] = WT5001_END;

  _writeData(pkt, 5);

  return _checkResponse(opcode);
}

/**
* queue a track to play next, when current song is finished
*
* @param index file number to queue
* @return true if successful
*/
bool _queue(uint16_t index){
  char pkt[6];
  WT5001_OPCODE_T opcode = QUEUE;

  pkt[0] = WT5001_START;
  pkt[1] = 0x04;                // length
  pkt[2] = opcode;
  pkt[3] = (index >> 8) & 0xff; // index hi
  pkt[4] = index & 0xff;        // index lo
  pkt[5] = WT5001_END;

  _writeData(pkt, 6);

  return _checkResponse(opcode);
}

/**
* set the playback mode
*
* @param pm play mode to enable
* @return true if successful
*/
bool _setPlayMode(WT5001_PLAYMODE_T pm){
  char pkt[5];
  WT5001_OPCODE_T opcode = PLAY_MODE;

  pkt[0] = WT5001_START;
  pkt[1] = 0x03;                // length
  pkt[2] = opcode;
  pkt[3] = pm;
  pkt[4] = WT5001_END;

  _writeData(pkt, 5);

  return _checkResponse(opcode);
}

/**
* insert a track to play immediately, interrupting the current
* track.  When the inserted track is finished playing, the
* interrupted track will resume where it was interrupted.
*
* @param index file number to insert
* @return true if successful
*/
bool _insert(uint16_t index){
  char pkt[6];
  WT5001_OPCODE_T opcode = INSERT_SONG;

  pkt[0] = WT5001_START;
  pkt[1] = 0x04;                // length
  pkt[2] = opcode;
  pkt[3] = (index >> 8) & 0xff; // index hi
  pkt[4] = index & 0xff;        // index lo
  pkt[5] = WT5001_END;

  _writeData(pkt, 6);

  return _checkResponse(opcode);
}

/**
* set the date of the internal clock
*
* @param year 4 digit year
* @param month the month
* @param day the day
* @return true if successful
*/
bool _setDate(uint16_t year, uint8_t month, uint8_t day){
  char pkt[8];
  WT5001_OPCODE_T opcode = SET_DATE;

  pkt[0] = WT5001_START;
  pkt[1] = 0x06;                // length
  pkt[2] = opcode;
  pkt[3] = (year >> 8) & 0xff;  // year hi
  pkt[4] = year & 0xff;         // year lo
  pkt[5] = month;               // month
  pkt[6] = day;                 // day
  pkt[7] = WT5001_END;

  _writeData(pkt, 8);

  return _checkResponse(opcode);
}

/**
* set the time of the internal clock
*
* @param hour hour
* @param minute minute
* @param second second
* @return true if successful
*/
bool _setTime(uint8_t hour, uint8_t minute, uint8_t second){
  char pkt[7];
  WT5001_OPCODE_T opcode = SET_TIME;

  pkt[0] = WT5001_START;
  pkt[1] = 0x05;                // length
  pkt[2] = opcode;
  pkt[3] = hour;                // hour
  pkt[4] = minute;              // minute
  pkt[5] = second;              // second
  pkt[6] = WT5001_END;

  _writeData(pkt, 7);

  return _checkResponse(opcode);
}

/**
* set the alarm
*
* @param hour hour
* @param minute minute
* @param second second
* @return true if successful
*/
bool _setAlarm(uint8_t hour, uint8_t minute, uint8_t second){
  char pkt[7];
  WT5001_OPCODE_T opcode = SET_ALARM;

  pkt[0] = WT5001_START;
  pkt[1] = 0x05;                // length
  pkt[2] = opcode;
  pkt[3] = hour;                // hour
  pkt[4] = minute;              // minute
  pkt[5] = second;              // second
  pkt[6] = WT5001_END;

  _writeData(pkt, 7);

  return _checkResponse(opcode);
}

/**
* clear any alarm that has been set
*
* @return true if successful
*/
bool _clearAlarm(){
  char pkt[4];
  WT5001_OPCODE_T opcode = CLEAR_ALARM;

  pkt[0] = WT5001_START;
  pkt[1] = 0x02;                // length
  pkt[2] = opcode;
  pkt[3] = WT5001_END;

  _writeData(pkt, 4);

  return _checkResponse(opcode);
}

/**
* get the current volume
*
* @param vol the returned volume
* @return true if successful
*/
bool _getVolume(uint8_t *vol){
  char pkt[4];
  WT5001_OPCODE_T opcode = READ_VOLUME;

  pkt[0] = WT5001_START;
  pkt[1] = 0x02;                // length
  pkt[2] = opcode;
  pkt[3] = WT5001_END;

  _writeData(pkt, 4);

  if (!_checkResponse(opcode))
    return false;

  // there should be a byte waiting for us, the volume
  int rv = _readData((char *)vol, 1);
  if (rv != 1)
    return false;

  return true;
}

/**
* get the current play state. 1 = playing, 2 = stopped, 3 = paused
*
* @param ps the returned play state
* @return true if successful
*/
bool _getPlayState(uint8_t *ps){
  char pkt[4];
  WT5001_OPCODE_T opcode = READ_PLAY_STATE;

  pkt[0] = WT5001_START;
  pkt[1] = 0x02;                // length
  pkt[2] = opcode;
  pkt[3] = WT5001_END;

  _writeData(pkt, 4);

  if (!_checkResponse(opcode))
    return false;

  // there should be a byte waiting for us, the play state
  int rv = _readData((char *)ps, 1);
  if (rv != 1)
    return false;

  return true;
}

/**
* get the number of files present on the source device
*
* @param psrc the storage source
* @param numf the returned number of files
* @return true if successful
*/
bool _getNumFiles(WT5001_PLAYSOURCE_T psrc, uint16_t *numf){
  char pkt[4];
  WT5001_OPCODE_T opcode;

  pkt[0] = WT5001_START;
  pkt[1] = 0x02;                // length

  switch (psrc)                 // src
    {
    case SD:
      opcode = READ_SD_NUMF;
      break;

    case SPI:
      opcode = READ_SPI_NUMF;
      break;

    case UDISK:
      opcode = READ_UDISK_NUMF;
      break;
    }      

  pkt[2] = opcode;
  pkt[3] = WT5001_END;

  _writeData(pkt, 4);

  if (!_checkResponse(opcode))
    return false;

  // read the two byte response, and encode them
  char buf[2];
  int rv = _readData(buf, 2);
  if (rv != 2)
    return false;

  *numf = (buf[0] << 8) | buf[1];

  return true;
}

/**
* get the index of the current file
*
* @param curf the index of the current file
* @return true if successful
*/
bool _getCurrentFile(uint16_t *curf){
  char pkt[4];
  WT5001_OPCODE_T opcode = READ_CUR_FNAME;

  pkt[0] = WT5001_START;
  pkt[1] = 0x02;                // length
  pkt[2] = opcode;
  pkt[3] = WT5001_END;

  _writeData(pkt, 4);

  if (!_checkResponse(opcode))
    return false;

  // read the two byte response, and encode them
  char buf[2];
  int rv = _readData(buf, 2);
  if (rv != 2)
    return false;

  *curf = (buf[0] << 8) | (buf[1] & 0xff);

  return true;
}

/**
* get the device date
*
* @param year returned 4 digit year
* @param month returned month
* @param day returned day
* @return true if successful
*/
bool _getDate(uint16_t *year, uint8_t *month, uint8_t *day){
  char pkt[4];
  WT5001_OPCODE_T opcode = READ_DATE;

  pkt[0] = WT5001_START;
  pkt[1] = 0x02;                // length
  pkt[2] = opcode;
  pkt[3] = WT5001_END;

  _writeData(pkt, 4);

  if (!_checkResponse(opcode))
    return false;

  // read the 4 byte response
  char buf[4];
  int rv = _readData(buf, 4);
  if (rv != 4)
    return false;

  *year = (buf[0] << 8) | (buf[1] & 0xff);
  *month = buf[2];
  *day = buf[3];
  return true;
}

/**
* get the device time
*
* @param hour returned hour
* @param minute returned minute
* @param second returned second
* @return true if successful
*/
bool _getTime(uint8_t *hour, uint8_t *minute, uint8_t *second){
  char pkt[4];
  WT5001_OPCODE_T opcode = READ_TIME;

  pkt[0] = WT5001_START;
  pkt[1] = 0x02;                // length
  pkt[2] = opcode;
  pkt[3] = WT5001_END;

  _writeData(pkt, 4);

  if (!_checkResponse(opcode))
    return false;

  // read the 3 byte response
  char buf[3];
  int rv = _readData(buf, 3);
  if (rv != 3)
    return false;

  *hour = buf[0];
  *minute = buf[1];
  *second = buf[2];
  return true;
}
