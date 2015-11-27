#include <stdio.h> /* printf, sprintf */
#include <stdlib.h> /* exit */
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <math.h>
#include <stdbool.h>
#include "cJSON.h"

int get_gamma(uint32_t ip, char *message, int total, char *path, float *x, float *y, int *bri, bool *on);

int socket_send_to(uint32_t ip, char *message, int total, char *response, int res_size);

void HSVtoRGB(float *r, float *g, float *b, float h, float s, float v );

void RGBtoXY(int8_t gamma, uint8_t r, uint8_t g, uint8_t b, float *x, float *y);

void XYbtoRGB(int8_t gamma, float x, float y, float bri, uint8_t *r, uint8_t *g, uint8_t *b);