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

#ifndef __FAST_GPIO_SC_H__
#define __FAST_GPIO_SC_H__

//#include <Arduino.h>


#ifdef __cplusplus
extern "C" {
#endif

int fastGpioSCInit(void);
void fastGpioSCFini(void);
void fastGpioDigitalWrite(register uint8_t gpio, register uint8_t val);
void fastGpioDigitalWriteDestructive(register uint8_t gpio);
uint32_t fastGpioDigitalLatch(void);

#ifdef __cplusplus
}
#endif

#endif /* __FAST_GPIO_SC_H__ */
