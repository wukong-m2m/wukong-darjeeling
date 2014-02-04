#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"
#include <avr/io.h>
#define F_CPU 16000000L
#include <util/delay.h>
#include <inttypes.h>
#include <compat/twi.h>
#define SCL_CLOCK  10000L
#define I2C_READ    1

/** defines the data direction (writing to I2C device) in i2c_start(),i2c_rep_start() */
#define I2C_WRITE   0
#define i2c_read(ack)  (ack) ? i2c_readAck() : i2c_readNak(); 

#define BH1750_ADDR (0x23<<1) //device address

//i2c settings
#define BH1750_I2CFLEURYPATH "../i2chw/i2cmaster.h" //define the path to i2c fleury lib
#define BH1750_I2CINIT 1 //init i2c

//resolution modes
#define BH1750_MODEH 0x10 //continuously h-resolution mode, 1lx resolution, 120ms
#define BH1750_MODEH2 0x11 //continuously h-resolution mode, 0.5lx resolution, 120ms
#define BH1750_MODEL 0x13 //continuously l-resolution mode, 4x resolution, 16ms
//define active resolution mode
#define BH1750_MODE BH1750_MODEH

void i2c_init(void)
{
  /* initialize TWI clock: 100 kHz clock, TWPS = 0 => prescaler = 1 */
  
  TWSR = 0;                         /* no prescaler */
  TWBR = (uint8_t)((F_CPU/SCL_CLOCK)-16)/2;  /* must be > 10 for stable operation */

}/* i2c_init */


/*************************************************************************	
  Issues a start condition and sends address and transfer direction.
  return 0 = device accessible, 1= failed to access device
*************************************************************************/
unsigned char i2c_start(unsigned char address)
{
    uint8_t   twst;

	// send START condition
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

	// wait until transmission completed
	while(!(TWCR & (1<<TWINT)));

	// check value of TWI Status Register. Mask prescaler bits.
	twst = TW_STATUS & 0xF8;
	if ( (twst != TW_START) && (twst != TW_REP_START)) return 1;

	// send device address
	TWDR = address;
	TWCR = (1<<TWINT) | (1<<TWEN);

	// wail until transmission completed and ACK/NACK has been received
	while(!(TWCR & (1<<TWINT)));

	// check value of TWI Status Register. Mask prescaler bits.
	twst = TW_STATUS & 0xF8;
	if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) return 1;

	return 0;

}/* i2c_start */


/*************************************************************************
 Issues a start condition and sends address and transfer direction.
 If device is busy, use ack polling to wait until device is ready
 
 Input:   address and transfer direction of I2C device
*************************************************************************/
int i2c_start_wait(unsigned char address)
{
    uint8_t   twst;


	    // send START condition
	    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
    
    	// wait until transmission completed
    	while(!(TWCR & (1<<TWINT)));
    
    	// check value of TWI Status Register. Mask prescaler bits.
    	twst = TW_STATUS & 0xF8;
    	if ( (twst != TW_START) && (twst != TW_REP_START)) return 0;
    
    	// send device address
    	TWDR = address;
    	TWCR = (1<<TWINT) | (1<<TWEN);
    
    	// wail until transmission completed
    	while(!(TWCR & (1<<TWINT)));
    
    	// check value of TWI Status Register. Mask prescaler bits.
    	twst = TW_STATUS & 0xF8;
    	if ( (twst == TW_MT_SLA_NACK )||(twst ==TW_MR_DATA_NACK) ) 
    	{    	    
    	    /* device busy, send stop condition to terminate write operation */
	        TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	        
	        // wait until stop condition is executed and bus released
	        while(TWCR & (1<<TWSTO));
	        
		return 0;
    	}
    	//if( twst != TW_MT_SLA_ACK) return 1;
    	return 1;
}/* i2c_start_wait */


/*************************************************************************
 Issues a repeated start condition and sends address and transfer direction 

 Input:   address and transfer direction of I2C device
 
 Return:  0 device accessible
          1 failed to access device
*************************************************************************/
unsigned char i2c_rep_start(unsigned char address)
{
    return i2c_start( address );

}/* i2c_rep_start */


/*************************************************************************
 Terminates the data transfer and releases the I2C bus
*************************************************************************/
void i2c_stop(void)
{
    /* send stop condition */
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	
	// wait until stop condition is executed and bus released
	while(TWCR & (1<<TWSTO));

}/* i2c_stop */


/*************************************************************************
  Send one byte to I2C device
  
  Input:    byte to be transfered
  Return:   0 write successful 
            1 write failed
*************************************************************************/
unsigned char i2c_write( unsigned char data )
{	
    uint8_t   twst;
    
	// send data to the previously addressed device
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);

	// wait until transmission completed
	while(!(TWCR & (1<<TWINT)));

	// check value of TWI Status Register. Mask prescaler bits
	twst = TW_STATUS & 0xF8;
	if( twst != TW_MT_DATA_ACK) return 1;
	return 0;

}/* i2c_write */


/*************************************************************************
 Read one byte from the I2C device, request more data from device 
 
 Return:  byte read from I2C device
*************************************************************************/
unsigned char i2c_readAck(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	while(!(TWCR & (1<<TWINT)));    

    return TWDR;

}/* i2c_readAck */


/*************************************************************************
 Read one byte from the I2C device, read is followed by a stop condition 
 
 Return:  byte read from I2C device
*************************************************************************/
unsigned char i2c_readNak(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
	
    return TWDR;

}/* i2c_readNak */


/*
 * init bh1750
 */
void bh1750_init() {


	//write config
	
	i2c_write(BH1750_MODE);
	i2c_stop();
}

int bh1750_detect()
{
	return i2c_start_wait(BH1750_ADDR | I2C_WRITE);
}

/*
 * read lux value
 */
int bh1750_getlux() {
	int ret = 0;

	i2c_start_wait(BH1750_ADDR | I2C_READ);
	ret = i2c_readAck();
	ret |= (i2c_readNak()<<8);
	i2c_stop();

	return ret;
}
void wuclass_light_sensor_setup(wuobject_t *wuobject) {}
static int my_bh1750_detect=0;
void wuclass_light_sensor_update(wuobject_t *wuobject) {
  // Pieced together from IntelDemoLightSensorV1.java, Adc.java and native_avr.c
  uint8_t v;
  if (my_bh1750_detect==0) {
	  i2c_init();
	  _delay_ms(10);

	  if (bh1750_detect()) {
		  bh1750_init();
		  my_bh1750_detect = 1;
	  }
  }
  if (my_bh1750_detect==0) {
	  // Adc.setPrescaler(Adc.DIV64);
	  ADCSRA = _BV(ADEN) | (6 & 7);  // set prescaler value

	  // Adc.setReference(Adc.INTERNAL);
	  ADMUX = (3 << 6) & 0xc0;              // set reference value

	  // light_sensor_reading = Adc.getByte(Adc.CHANNEL0);
	  // ADLAR = 1
	  uint8_t channel  = 0; // NOTE: Adc.CHANNEL0 means a value of 0 for the channel variable, but other ADC channels don't map 1-1. For instance channel 15 is selected by setting the channel variable to 39. See below for a list.
	  ADMUX = (ADMUX & 0xc0) | _BV(ADLAR) | (channel & 0x0f);
	  ADCSRB |= (channel & 0x20)>>2;

	  // do conversion
	  ADCSRA |= _BV(ADSC);                  // Start conversion
	  while(!(ADCSRA & _BV(ADIF)));         // wait for conversion complete
	  ADCSRA |= _BV(ADIF);                  // clear ADCIF
	  v = ADCH;
  	  DEBUG_LOG(true, "LS(A): %d\n", v);
  } else {

	int lv = bh1750_getlux();
  	DEBUG_LOG(true, "LS(D): %d\n", lv);
	v = lv>>8;
  }
  
  wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_LIGHT_SENSOR_CURRENT_VALUE, v);
}

// ADC CHANNEL 0,  value for channel variable: 0
// ADC CHANNEL 1,  value for channel variable: 1
// ADC CHANNEL 2,  value for channel variable: 2
// ADC CHANNEL 3,  value for channel variable: 3
// ADC CHANNEL 4,  value for channel variable: 4
// ADC CHANNEL 5,  value for channel variable: 5
// ADC CHANNEL 6,  value for channel variable: 6
// ADC CHANNEL 7,  value for channel variable: 7
// ADC CHANNEL 8,  value for channel variable: 32
// ADC CHANNEL 9,  value for channel variable: 33
// ADC CHANNEL 10, value for channel variable: 34
// ADC CHANNEL 11, value for channel variable: 35
// ADC CHANNEL 12, value for channel variable: 36
// ADC CHANNEL 13, value for channel variable: 37
// ADC CHANNEL 14, value for channel variable: 38
// ADC CHANNEL 15, value for channel variable: 39
