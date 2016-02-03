#ifndef SMARTTHINGS_UTILS_H
#define SMARTTHINGS_UTILS_H
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "cJSON.h"

#define MESSAGE_SIZE 5000
#define BUF_SIZE 150

extern int getStatus (char *message, int msg_len, char *app_id, char *access_token,
  char *dev_id, char *status, int status_len);
#endif