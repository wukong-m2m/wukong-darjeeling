#include "execution.h"
#include "array.h"
#include "jlib_bm_heat_detect.h"

extern const uint16_t heat_sensor_data[100][64] PROGMEM;

uint16_t bm_heat_calib_get_heat_sensor_data_frame_number;
dj_int_array *bm_heat_calib_get_heat_sensor_data_frame_buffer;
uint16_t bm_heat_calib_get_heat_sensor_data_returnaddress;

void javax___get_heat_sensor_data() { 
        memcpy_P(bm_heat_calib_get_heat_sensor_data_frame_buffer->data.shorts, &heat_sensor_data[bm_heat_calib_get_heat_sensor_data_frame_number], sizeof(uint16_t[64])); 
}

#ifndef NO_LIGHTWEIGHT_METHODS
void javax_rtcbench_HeatCalib_void_get_heat_sensor_data_short___short() {
    asm volatile("   pop  r18" "\n\r"
                 "   pop  r19" "\n\r"
                 "   sts  bm_heat_calib_get_heat_sensor_data_returnaddress+1, r18" "\n\r"
                 "   sts  bm_heat_calib_get_heat_sensor_data_returnaddress, r19" "\n\r"
                 "   pop  r18" "\n\r"
                 "   pop  r19" "\n\r"
                 "   sts  bm_heat_calib_get_heat_sensor_data_frame_number, r18" "\n\r"
                 "   sts  bm_heat_calib_get_heat_sensor_data_frame_number+1, r19" "\n\r"
                 "   ld   r18, -x" "\n\r"
                 "   ld   r19, -x" "\n\r"
                 "   sts  bm_heat_calib_get_heat_sensor_data_frame_buffer+1, r18" "\n\r"
                 "   sts  bm_heat_calib_get_heat_sensor_data_frame_buffer, r19" "\n\r"
                 "   push r26" "\n\r"
                 "   push r27" "\n\r"
                 "   call javax___get_heat_sensor_data" "\n\r"
                 "   pop  r27" "\n\r"
                 "   pop  r26" "\n\r"
                 "   lds  r19, bm_heat_calib_get_heat_sensor_data_returnaddress" "\n\r"
                 "   lds  r18, bm_heat_calib_get_heat_sensor_data_returnaddress+1" "\n\r"
                 "   push r19" "\n\r"
                 "   push r18" "\n\r"
             :: );
}
#else
void javax_rtcbench_HeatCalib_void_get_heat_sensor_data_short___short() {
    bm_heat_calib_get_heat_sensor_data_frame_number = dj_exec_stackPopShort();
    bm_heat_calib_get_heat_sensor_data_frame_buffer = (dj_int_array *)REF_TO_VOIDP(dj_exec_stackPopRef());
    javax___get_heat_sensor_data();
}
#endif