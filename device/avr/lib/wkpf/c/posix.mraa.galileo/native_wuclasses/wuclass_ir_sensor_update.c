#include "config.h"
#if defined(INTEL_GALILEO_GEN1) || defined(INTEL_GALILEO_GEN2) || defined(INTEL_EDISON)
#include "debug.h"
#include "native_wuclasses.h"
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

void wuclass_ir_sensor_setup(wuobject_t *wuobject) {
	system("echo -n 37 > /sys/class/gpio/unexport");
        system("echo -n 37 > /sys/class/gpio/export");
        system("echo -n out > /sys/class/gpio/gpio37/direction");
        system("echo -n 0 > /sys/class/gpio/gpio37/value");

}

void wuclass_ir_sensor_update(wuobject_t *wuobject) {
	
        int16_t fd0=-1;
        char buf0[4] = {'\\','\\','\\','\\'};
	static int16_t dis_buf_0[3] = {0,0,0};

        fd0 = open("/sys/bus/iio/devices/iio:device0/in_voltage0_raw", O_RDONLY | O_NONBLOCK);
        lseek(fd0, 0, SEEK_SET);
        read(fd0, buf0, 4);
        close(fd0);
        int16_t num0=0;
        int16_t ii=0;
//      never use atoi
//      num0 = atoi(buf0);
        for(ii=0;ii<4;ii++){
            if(buf0[ii]>='0' && buf0[ii] <='9')
                num0 = num0*10 + (buf0[ii] - '0');
        }

	num0 /=4; // this devision should be tuned depending on diffirent ir sensor model.
                  // usually divided by 2 or 4

        float volts0 = 10650.08 * pow(num0, -0.935);
        int16_t distance0 = (int16_t)(volts0 - 10);
	if(distance0 > 200)
		distance0 = 200;
        printf("The distance is: %d cm\n",distance0);

	int16_t final_dis_0=0;
	int16_t total_num=0;
	int16_t max_dis, min_dis;
	int16_t max_index = 0, min_index = 0;

	max_dis = distance0;
	min_dis = distance0;
	max_index = min_index = -1;
	final_dis_0 = distance0;
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
			dis_buf_0[ii] = distance0;

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

        printf("The distance after smoothing is: %d cm\n",final_dis_0);
	wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_IR_SENSOR_CURRENT_VALUE, final_dis_0);
}
#endif