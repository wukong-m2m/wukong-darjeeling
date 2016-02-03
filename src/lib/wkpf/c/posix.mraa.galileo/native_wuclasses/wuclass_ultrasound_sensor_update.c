#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/time.h>
#include <ctype.h>
#include "fast_gpio_sc.h"
#include <time.h>
//#include "./arduino/wiring_digital.h"
#include "config.h"
#if defined(INTEL_GALILEO_GEN1) || defined(INTEL_GALILEO_GEN2) || defined(INTEL_EDISON)

float pulseIn()
{
	char buf[2];
	int16_t pulseWidth1 = 0;
	int16_t pulseWidth2 = 0;
	struct timeval t1, t2;
	int fp;
	if((fp = open("/sys/class/gpio/gpio14/value", O_RDONLY | O_NONBLOCK)) < 0)
	{
		printf("No such file\n");
		exit(1);
	}
	//The Theory of Ultrasound Sensor Operation:
        //The model of this ultrasound sensor is HC-SR04.
	//The trig pin sends a 10us pulse first to IC and IC produces a burst signal
        //Once the burst signal is transmitted, the echo pin is set to high level
        //The voltage level of echo pin keeps high until the burst signal reflects back to the ultrasound sensor
        //So, the time period of high level of echo pin is in proportion to the distance
	do
	{
		read(fp, buf, 2);
		lseek(fp, 0, SEEK_SET);
		pulseWidth1++;
	}while((buf[0] == '0') && (pulseWidth1<5000));
	//printf("buf[0]= %c, ",buf[0]);
	//printf("pulseWidth1: %d\n",pulseWidth1);
	gettimeofday(&t1, NULL);
	do
	{
		read(fp, buf, 2);
		lseek(fp, 0, SEEK_SET);
		pulseWidth2++;
	}while((buf[0] == '1') && (pulseWidth2<5000));
	//printf("buf[0]= %c, ",buf[0]);
	//printf("pulseWidth2: %d\n",pulseWidth2);
	gettimeofday(&t2, NULL);
	close(fp);
	
	if(pulseWidth1 == 5000 || pulseWidth2 == 5000)
		return -1.0;

        unsigned long long start_t = 1000000 * t1.tv_sec + t1.tv_usec;
        unsigned long long end_t = 1000000 * t2.tv_sec + t2.tv_usec;
        float diff_t = (float)(end_t - start_t);
        //printf("The start time is: %llu", start_t);
        //printf("The end time is: %llu", end_t);
        //printf("The single run time is: %f.\n", diff_t);
	return (diff_t);
}

void wuclass_ultrasound_sensor_setup(wuobject_t *wuobject) 
{
        fastGpioSCInit();
        //gpio15 corresponds to arduino port IO3 which is connected to Trig pin of Ultrasound sensor
        system("echo -n \"30\" > /sys/class/gpio/unexport");
        system("echo -n \"30\" > /sys/class/gpio/export");
        system("echo -n \"out\" > /sys/class/gpio/gpio30/direction");
        system("echo -n \"0\" > /sys/class/gpio/gpio30/value");
        system("echo -n \"15\" > /sys/class/gpio/unexport");
        system("echo -n \"15\" > /sys/class/gpio/export");
        system("echo -n \"out\" > /sys/class/gpio/gpio15/direction");

        //gpio14 corresponds to arduino port IO2 which is connected to Echo pin of Ultrasound sensor
        system("echo -n \"31\" > /sys/class/gpio/unexport");
        system("echo -n \"31\" > /sys/class/gpio/export");
        system("echo -n \"out\" > /sys/class/gpio/gpio31/direction");
        system("echo -n \"0\" > /sys/class/gpio/gpio31/value");
        system("echo -n \"14\" > /sys/class/gpio/unexport");
        system("echo -n \"14\" > /sys/class/gpio/export");
        system("echo -n \"in\" > /sys/class/gpio/gpio14/direction");

}

void wuclass_ultrasound_sensor_update(wuobject_t *wuobject) {

        struct timespec tim;
        int16_t distance;
        float pulse_time;
	static int16_t dis_buf[1] = {0};
	int16_t final_dis=0;

        fastGpioDigitalWrite(0x80,0);
        tim.tv_sec = 0;
        tim.tv_nsec = 5000;
        if(nanosleep(&tim , (struct timespec *)NULL) < 0 )
        {
                printf("Nano sleep system call failed \n");
        }
        //fastGpioDigitalWriteDestructive(0x80);
        fastGpioDigitalWrite(0x80,1);
        //fastGpioDigitalWriteDestructive(0x80);
        tim.tv_sec = 0;
        tim.tv_nsec = 10000;
        if(nanosleep(&tim , (struct timespec *)NULL) < 0 )
        {
                printf("Nano sleep system call failed \n");
        }
        fastGpioDigitalWrite(0x80,0);
        pulse_time = pulseIn();
	
        distance = (int16_t)((pulse_time/2.0)*0.0343);
	if(distance > 200)
		distance = 200;
        printf("distance: %d\n", distance);

	int16_t ii=0;
	int old_final_dis = final_dis;	
	
        if(pulse_time  < 0){
		printf("Ultrasonic didn't get right value in this turn!!!\n");	
        	for(ii=0;ii<1;ii++){
			final_dis = final_dis + dis_buf[ii];
		}
		final_dis = final_dis/1;
		
		if(old_final_dis != final_dis){
			wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_ULTRASOUND_SENSOR_CURRENT_VALUE, final_dis);
			printf("final_dis: %d", final_dis);
			return;
		}
		else{
			printf("1same final dis: %d", final_dis);
		}
	}


	int16_t total_num = 0;
        int16_t max_dis, min_dis;
        int16_t max_index = 0, min_index = 0;

        max_dis = distance;
        min_dis = distance;
        max_index = min_index = -1;
        final_dis = distance;
        total_num = 1;
        for(ii=0;ii<1;ii++){
                if(dis_buf[ii] != 0){
                        final_dis = final_dis + dis_buf[ii];
                        total_num++;
                }
                if(dis_buf[ii] > max_dis){
                        max_dis = dis_buf[ii];
                        max_index = ii;
                }
                //if(dis_buf[ii] < min_dis){
                //        min_dis = dis_buf[ii];
                //        min_index = ii;
                //}

                if(ii != 0)
                        dis_buf[ii] = dis_buf[ii-1];
                else
                        dis_buf[ii] = distance;

        }
        if(total_num != 1 && max_index != -1){
                final_dis = final_dis - max_dis;
                total_num--;
        }
        if(total_num != 1 && min_index != -1){
                final_dis = final_dis - min_dis;
                total_num--;
        }
        final_dis = final_dis/total_num;
	
	if(old_final_dis != final_dis){
		printf("2final_dis: %d\n", final_dis);
		wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_ULTRASOUND_SENSOR_CURRENT_VALUE, final_dis);
	}
	else{
		printf("2same final_dis: %d\n", final_dis);
	}
}
#endif