#include "debug.h"

#include "native_wuclasses.h"
// #include "../../common/native_wuclasses/native_wuclasses.h"
#include <avr/io.h>
// #include "../../common/native_wuclasses/GENERATEDwuclass_ultrasound_sensor.h"

#include "GENERATEDwuclass_led.h"
#define F_CPU 	16000000 //F_CPU must be defined before <util/delay.h>
#include <util/delay.h>
//#define F_CPU 	16000000

//define TRIG pin
#define US_DDR_OUT DDRH
#define US_PORT PORTH
#define US_POS_OUT PH4 // PH4 is defined to be 4 in /usr/lib/avr/include/avr/io.h
//define ECHO pin
#define US_DDR_IN DDRH
#define US_PIN PINH
#define US_POS_IN PH4

#define AVG_LEN 10

#define US_ERROR_RISE 0xffff
#define US_ERROR_FALL 0xfffe
#define US_NO_OBSTACLE 0xfffd

#define set_output(portdir, pin) portdir |= (1<<pin)
#define set_input(portdir, pin) portdir &= ~(1<<pin)
#define output_high(port, pin) port |= (1<<pin)
#define output_low(port, pin) port &= ~(1<<pin)
#define input_get(port, pin) ((port & (1 << pin)) != 0)

uint16_t getPulseWidth()
{
  uint32_t i, result;

  //wait for the rising edge
  for(i=0;i<600000;i++) //6000 is a reference value for reasonable waiting time
  {
	if(!(input_get(US_PIN, US_POS_IN))) continue; else break;
  }

  if(i==600000)
	return 0xffff;//Indicates time out

  //High Edge Found
  //Setup Timer1
  TCCR1A=0x00;
  TCCR1B=(1<<CS11);//Prescaler = Fcpu/8
  TCNT1=0x00;  	//Init counter

  //Now wait for the falling edge
  for(i=0;i<600000;i++)
  {
	if(input_get(US_PIN, US_POS_IN))
	{
  	if(TCNT1 > 60000) break; else continue;
	}else{
  	break;
	}
  }

  if(i==600000)
	return 0xfffe; //Indicates time out

  //Falling edge found

  result=TCNT1;

  //Stop Timer
  TCCR1B=0x00;

  if(result > 60000){
	return 0xfffd; //No obstacle
  }else{

	return (result>>1);
  }
}

void findMax(uint16_t *maxNum, uint16_t *maxCount, uint16_t dArray[])
{
  *maxNum = dArray[0];
  *maxCount = 0;
  int i=0;
  for(i=AVG_LEN;i>1;i--)
  {
	if(dArray[AVG_LEN-i+1] > *maxNum)
	{
  	*maxNum = dArray[AVG_LEN-i+1];
  	*maxCount = AVG_LEN-i+1;
	}
  }
}

void findMin(uint16_t *minNum, uint16_t *minCount, uint16_t dArray[])
{
  *minNum = dArray[0];
  *minCount = 0;
  int i=0;
  for(i=AVG_LEN;i>1;i--)
  {
	if(dArray[AVG_LEN-i+1] < *minNum)
	{
  	*minNum = dArray[AVG_LEN-i+1];
  	*minCount = AVG_LEN-i+1;
	}
  }
}

void wuclass_ultrasound_sensor_setup(wuobject_t *wuobject) {}

void wuclass_ultrasound_sensor_update(wuobject_t *wuobject) {
  uint16_t r;
  uint16_t avg;
  static uint16_t dArray[AVG_LEN]={0};
  uint16_t dSum;
  uint16_t maxNum, minNum, maxCount, minCount;

  //Set Ultra Sonic Port as out
  set_output(US_DDR_OUT, US_POS_OUT);
  output_low(US_PORT, US_POS_OUT); //Low
  _delay_us(2);

  //Give the US pin a 10us High Pulse according to the spec
  output_high(US_PORT, US_POS_OUT); //High
  _delay_us(10);
  output_low(US_PORT, US_POS_OUT); //Low

  //Now make the pin input
  set_input(US_DDR_IN, US_POS_IN);

  //Measure the width of pulse
  r=getPulseWidth();

  dArray[0]=(int)(r/58.0); //Echo pulse(us)/58 = Distance in cm
  int i;
  for(i=AVG_LEN;i>1;i--)
  {
  dArray[AVG_LEN-i+1] = dArray[AVG_LEN-i]; //shift all old values and update a new value to dArray
  }

  findMax(&maxNum, &maxCount, dArray);
  findMin(&minNum, &minCount, dArray);
  dSum = 0;
  for(i=0;i<AVG_LEN;i++)
  {
  dSum+=dArray[i];
  }
  avg = (dSum - dArray[maxCount] - dArray[minCount])/(AVG_LEN-2);

  //Handle Errors
  if(r==US_ERROR_RISE)
  {
  DEBUG_LOG(DBG_WKPFUPDATE, "Ultrasound Sensor Rise Error!");
  }
  else if(r==US_ERROR_FALL)
  {
  DEBUG_LOG(DBG_WKPFUPDATE, "Ultrasound Sensor Fall Error!");
  }
  else if(r==US_NO_OBSTACLE)
  {
  DEBUG_LOG(DBG_WKPFUPDATE, "Ultrasound Sensor Clear!");
  }
  else
  {
  // reverse the output value
  avg = 300 - avg;
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(UltrasoundSensor): Sensed distance value: %d cm\n", avg);
  wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_ULTRASOUND_SENSOR_CURRENT_VALUE, avg);
  }
}
