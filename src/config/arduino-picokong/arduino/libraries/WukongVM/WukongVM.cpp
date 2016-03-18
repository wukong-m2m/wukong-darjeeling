#include <Arduino.h>
extern "C" {
	typedef unsigned char uint8;
	typedef unsigned long uint32;
	unsigned long avr_millis();
	void avr_delay(unsigned long ms);
	void avr_serialPrint(char * str);
	void avr_serialPrint(char * str);
	void uart_write_byte(uint8 port,char c);
	int uart_available(uint8 port);
	char uart_read_byte(uint8 port);
	void uart_inituart(uint8 port,uint32 rate);
unsigned long avr_millis()
{
	millis();
}	
void avr_delay(unsigned long ms)
{
	delay(ms);
}
void avr_serialPrintf(char * str)
{
	Serial.print(str);
}
void avr_serialWrite(uint8 value)
{
	Serial.print(value);
}
void uart_write_byte(uint8 port, char c)
{
	if (port == 0) {
		Serial.write(c);
	} else if (port == 1) {
		Serial1.write(c);
	} else if (port == 2) {
		Serial2.write(c);
	} else if (port == 3) {
		Serial3.write(c);
	}
}

char uart_read_byte(uint8 port)
{
	if (port == 0) {
		return Serial.read();
	} else if (port == 1) {
		return Serial1.read();
	} else if (port == 2) {
		return Serial2.read();
	} else if (port == 3) {
		return Serial3.read();
	}
}

int uart_available(uint8 port) 
{
	if (port == 0) {
		return Serial.available();
	} else if (port == 1) {
		return Serial1.available();
	} else if (port == 2) {
		return Serial2.available();
	} else if (port == 3) {
		return Serial3.available();
	}
}
void uart_inituart(uint8 port,uint32 rate)
{
	Serial.print("Init port ");
	Serial.print(port);
	Serial.print(" rate ");
	Serial.println(rate);

	if (port == 0) {
		Serial.begin(rate);
	} else if (port == 1) {
		Serial1.begin(rate);
	} else if (port == 2) {
		Serial2.begin(rate);
	} else if (port == 3) {
		Serial3.begin(rate);
	}
}

};
