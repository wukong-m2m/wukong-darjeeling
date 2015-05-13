/*
 * fast_gpio_sc.c
 *
 * Implement a fast GPIO path.
 *
 * An explicit contract exists between this code and the in-kernel driver, since we both 'own'
 * the registers in question - user-space undertakes - never - ever to run concurrent data
 * whilst using the fast GPIO driver - in other words - user-space guarantees to never drive
 * traffic that can conflict with the kernel code.
 *
 * This a write only interface - read is verboten - verboten !
 *
 * Author : Bryan O'Donoghue <bryan.odonoghue@intel.com> 2013
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
/* ia32 port */
//#include <Arduino.h>
//#include <trace.h>

#define __TTYUART_IDX_TX 1
#define __TTYUART_IDX_RX 0

#define MY_TRACE_PREFIX	"fast_sc_gpio"
#define UIO_DEVICE	"/dev/uio0"
#define MAP_SIZE	0x1000			/* PCI bar region size */

/*************************** Static ****************************/

struct fgpio_sc {
	char * regs;
	int uio_handle;
	pthread_mutex_t mutex;
};

static struct fgpio_sc fgpio;

/*************************** Global ****************************/

/**
 * fastGpioSCInit
 *
 * Initialise the fast SC GPIO interface
 */
int fastGpioSCInit(void)
{
	extern int errno;

	fgpio.regs = NULL;
	fgpio.uio_handle = -1;

	/* Get handle to UIO regs */
	fgpio.uio_handle = open(UIO_DEVICE, O_RDWR);
	if (fgpio.uio_handle < 0){
		fprintf(stderr, "unable to open %s O_RDONLY\n", UIO_DEVICE);
		return errno;
	}

	/* mmap */
	fgpio.regs = (char*)mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fgpio.uio_handle, 0);
	if (fgpio.regs == MAP_FAILED){
		fprintf(stderr, "unable to mmap UIO %s @ handle %d", UIO_DEVICE, fgpio.uio_handle);
		close(fgpio.uio_handle);
		fgpio.uio_handle = -1;
	}

//	trace_debug("successfully mapped %s size %d handle %d", UIO_DEVICE, MAP_SIZE, fgpio.uio_handle);
	return 0;
}

/**
 * fastGpioSCInit
 *
 * Tear down the fast SC GPIO interface
 */
void fastGpioSCFini(void)
{
	if(fgpio.regs != NULL){
		munmap(fgpio.regs, MAP_SIZE);
		fgpio.regs = NULL;
	}

	if (fgpio.uio_handle != -1){
		close(fgpio.uio_handle);
		fgpio.uio_handle = -1;
	}
}

/**
 * fastGpioDigitalWrite
 *
 * Do a fast write to the GPIO regs
 * This is safe for input GPIO and completely unsafe for write
 * For write() the contract is either kernel or user-space may write NOT BOTH
 *
 * This version of fast write - does a read/modify/write which ends up approx 1/4 the speed of a destructive
 * write for any given square wave.
 */
void fastGpioDigitalWrite(register uint8_t gpio, register uint8_t val)
{
	if (val){
		*(volatile uint32_t*)fgpio.regs |= gpio;
	}else{
		*(volatile uint32_t*)fgpio.regs &= ~gpio;
	}
}

/**
 * fastGpioDigitalWriteDestructive
 *
 * Do a fast write to the GPIO regs - destructively. We assume you know all the values of the bits in question
 * and don't do a read/modify/write.
 *
 */
void fastGpioDigitalWriteDestructive(register uint8_t gpio)
{
	*(volatile uint32_t*)fgpio.regs = gpio;
}

/**
 * fastGpioDigitalLatch
 *
 * Read the current state of the GPIO registers.
 *
 */
uint32_t fastGpioDigitalLatch(void)
{
    return *(volatile uint32_t*)fgpio.regs;
}
