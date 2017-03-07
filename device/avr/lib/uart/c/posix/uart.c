#include <termios.h>
#include <sys/select.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "uart.h"

extern char* posix_uart_filenames[4]; // Should be filled from main.c
int uart_fd[4] = {0, 0, 0, 0};

void uart_inituart(uint8_t uart, uint32_t baudrate) {
	if(posix_uart_filenames[uart] == NULL) {
		printf("Uart %d not specified\n", uart);
		abort();
	}

	int fd = open(posix_uart_filenames[uart], O_RDWR | O_NOCTTY);
	if (uart_fd[uart] < 0) {
		printf("open %s error\n", posix_uart_filenames[uart]);
		abort();
	}
	uart_fd[uart] = fd;

    struct termios newtio;
	if (tcgetattr(uart_fd[uart], &newtio) < 0) {
		printf("errors:tcgetattr.\n");
		abort();
	}
	cfmakeraw(&newtio);
	cfsetispeed(&newtio, baudrate);
	cfsetospeed(&newtio, baudrate);

	tcflush(uart_fd[uart], TCIFLUSH);
	if (tcsetattr(uart_fd[uart], TCSANOW, &newtio) < 0) {
		printf("errors:tcsetattr.\n");
		abort();
	}
	printf("Opened %s as UART %d ...\n", posix_uart_filenames[uart], uart);
}

void uart_write_byte(uint8_t uart, uint8_t byte) {
	if(uart_fd[uart] == 0) {
		printf("uart_write_byte: uart %d not opened\n", uart);
		abort();
	}
	write(uart_fd[uart], &byte, 1);
}

bool uart_available(uint8_t uart, uint16_t wait_ms) {
	if(uart_fd[uart] == 0) {
		printf("uart_available: uart %d not opened\n", uart);
		abort();
	}

	struct timeval to;
	fd_set rs;
	to.tv_sec = wait_ms / 1000;
	to.tv_usec = (wait_ms % 1000) * 1000;
	FD_ZERO(&rs);
	FD_SET(uart_fd[uart],&rs);

	int n = select(uart_fd[uart]+1, &rs, NULL, NULL, &to) > 0;

	return n > 0;
}

uint8_t uart_read_byte(uint8_t uart) {
	if(uart_fd[uart] == 0) {
		printf("uart_read_byte: uart %d not opened\n", uart);
		abort();
	}

	int n;
	uint8_t b;
	n = read(uart_fd[uart], &b, 1);
	if (n == 1) {
		return b;
	} else {
		printf("read error !!!!!!!!!!!!!! n=%d\n", n);
		abort();
	}
}
