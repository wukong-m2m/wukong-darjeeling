/*
 * Author: Jon Trulson <jtrulson@ics.com>
 * Copyright (c) 2015 Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "MOSFET_cjq4435.h"
#include <mraa.h>

bool m_enabled;
mraa_pwm_context m_pwm;

void CJQ4435_Init(int pin)
{
  if ( !(m_pwm = mraa_pwm_init(pin)) )
  {
    printf("mraa_pwm_init() failed\n");
    return;
  }

  m_enabled = false;
}

void CJQ4435_Release()
{
  if (m_enabled)
    mraa_pwm_enable(m_pwm, 0);

  mraa_pwm_close(m_pwm);
}

void setPeriodUS(int us)
{
  if (mraa_pwm_period_us(m_pwm, us) != MRAA_SUCCESS)
  {
    printf("period specified is not supported\n");
  }
}

void setPeriodMS(int ms)
{
  if (mraa_pwm_period_ms(m_pwm, ms) != MRAA_SUCCESS)
  {
    printf("period specified is not supported\n");
  }
}

void setPeriodSeconds(float seconds)
{
  if (mraa_pwm_period(m_pwm, seconds) != MRAA_SUCCESS)
  {
    printf("period specified is not supported\n");
  }
}

void enable(bool enable)
{
  m_enabled = enable;
  mraa_pwm_enable(m_pwm, ((enable) ? 1 : 0));
}

void setDutyCycle(float dutyCycle)
{
  if (dutyCycle < 0.0)
    dutyCycle = 0.0;

  if (dutyCycle > 1.0)
    dutyCycle = 1.0;

  mraa_pwm_write(m_pwm, dutyCycle);
}

void on()
{
  // set a 1 second period, with 100% duty cycle

  enable(false);
  setPeriodUS(1000);
  setDutyCycle(1.0);
  enable(true);
}

void off()
{
  // set a 1 second period, with 0% duty cycle

  enable(false);
  setPeriodUS(1000);
  setDutyCycle(0.0);
  enable(true);
}