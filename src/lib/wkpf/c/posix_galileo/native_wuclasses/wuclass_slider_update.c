#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <math.h>

void wuclass_slider_setup(wuobject_t *wuobject) {
	system("echo -n 36 > /sys/class/gpio/unexport");
        system("echo -n 36 > /sys/class/gpio/export");
        system("echo -n out > /sys/class/gpio/gpio36/direction");
        system("echo -n 0 > /sys/class/gpio/gpio36/value");

}

void wuclass_slider_update(wuobject_t *wuobject) {
	
        int16_t fd0=-1;
        char buf0[4] = {'\\','\\','\\','\\'};
	static int16_t dis_buf_0[3] = {0,0,0};

        fd0 = open("/sys/bus/iio/devices/iio:device0/in_voltage1_raw", O_RDONLY | O_NONBLOCK);
        lseek(fd0, 0, SEEK_SET);
        read(fd0, buf0, 4);
        close(fd0);
        int16_t num0=0;
        int16_t ii=0;
        for(ii=0;ii<4;ii++){
            if(buf0[ii]>='0' && buf0[ii] <='9')
                num0 = num0*10 + (buf0[ii] - '0');
        }
//      never use atoi
//      num0 = atoi(buf0);
        printf("Slider value is: %d\n", num0);

	float final_dis_0=0;
	int16_t total_num=0;
	int16_t max_dis, min_dis;
	int16_t max_index = 0, min_index = 0;

	max_dis = num0;
	min_dis = num0;
	max_index = min_index = -1;
	final_dis_0 = num0;
	total_num = 1;
	for(ii=0;ii<3;ii++){
		if(dis_buf_0[ii] != 0){
			final_dis_0 = final_dis_0 + dis_buf_0[ii];
			total_num++;
		}
		if(dis_buf_0[ii] > max_dis){
			max_dis = dis_buf_0[ii];
			max_index = ii;
		}
		if(dis_buf_0[ii] < min_dis){
			min_dis = dis_buf_0[ii];
			min_index = ii;
		}

		if(ii != 2)
			dis_buf_0[ii] = dis_buf_0[ii+1];
		else
			dis_buf_0[ii] = num0;

	}
	if(total_num != 1 && max_index != -1){
		final_dis_0 = final_dis_0 - max_dis;
		total_num--;
	}
	if(total_num != 1 && min_index != -1){
		final_dis_0 = final_dis_0 - min_dis;
		total_num--;
	}
	final_dis_0 = final_dis_0/total_num;

        printf("After smoothing, Slider value is: %f\n", final_dis_0);
	int16_t low;
	int16_t high;
  	wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_SLIDER_LOW_VALUE, &low);
  	wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_SLIDER_HIGH_VALUE, &high);
	int16_t range = high-low;
  	int16_t output = (int16_t)(((final_dis_0)/4095)*range+low);
	wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_SLIDER_OUTPUT, output);
	printf("Slider Sensed %f, low %d, high %d, output %d\n", final_dis_0, low, high, output);
	DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Slider): Sensed %f, low %d, high %d, output %d\n", final_dis_0, low, high, output);

}
