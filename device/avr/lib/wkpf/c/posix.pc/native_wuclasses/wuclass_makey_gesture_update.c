#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#define STDIN 0

int getch(void)
{
	struct termios oldattr, newattr;
	struct timeval tv;
	fd_set readfds;
	int ch;

	tv.tv_sec = 0;
	tv.tv_usec = 100000;
	FD_ZERO(&readfds);
	FD_SET(STDIN, &readfds);
	tcgetattr( STDIN_FILENO, &oldattr );
	newattr = oldattr;
	newattr.c_lflag &= ~( ICANON | ECHO );
	tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
	select(STDIN+1, &readfds, NULL, NULL, &tv);
	if (FD_ISSET(STDIN, &readfds))
		ch = getchar();
	else
		ch = -2;
	tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
	return ch;
}

void wuclass_makey_gesture_setup(wuobject_t *wuobject) {
}

void wuclass_makey_gesture_update(wuobject_t *wuobject) {
	static int sent = 0;
	if (sent == 1) {
		sent = false;
        	wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_MAKEY_GESTURE_NUMBER, 7);
        	wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_MAKEY_GESTURE_MODE, false);
		return;
	}
	if (sent) sent--;
	int c = getch();
	if (c != -2)
		sent = 5;
	printf("WKPFUPDATE(makey): input value: %d\n", c);
	if (c == 'w') {
        	wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_MAKEY_GESTURE_NUMBER, 3);
	} else if (c == 's') {
        	wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_MAKEY_GESTURE_NUMBER, 4);
	} else if (c == 'a') {
        	wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_MAKEY_GESTURE_NUMBER, 1);
	} else if (c == 'd') {
        	wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_MAKEY_GESTURE_NUMBER, 2);
	} else if (c == 'g') {
        	wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_MAKEY_GESTURE_NUMBER, 5);
	} else if (c == 'f') {
        	wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_MAKEY_GESTURE_NUMBER, 6);
	} else if (c == ' ') {
        	wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_MAKEY_GESTURE_MODE, true);
	} 
}
