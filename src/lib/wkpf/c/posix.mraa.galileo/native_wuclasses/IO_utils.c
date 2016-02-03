#include "IO_utils.h"

int16_t aio_read(char *buf){
    int16_t num=0;
    int16_t i;
    //use this loop to convert char to int
    //at first, we use atoi (e.g. num=atoi(buf)) but quickly realize that atoi is not reliable
    for(i=0;i<4;i++){
        if(buf[i]>='0' && buf[i] <='9')
            num = num*10 + (buf[i] - '0');
    }
    return (int16_t)(num);
}

int16_t gpio_read(char *this_gpio){
    int value_i;
    char path[100]={0};
    snprintf(path, 100, "/sys/class/gpio/%s/value", this_gpio);
    FILE *fp = NULL;
    while (fp == NULL){
      fp = fopen(path, "r");
    }
    fscanf(fp, "%d", &value_i);
    fclose(fp);
    return (int16_t)value_i;
}

