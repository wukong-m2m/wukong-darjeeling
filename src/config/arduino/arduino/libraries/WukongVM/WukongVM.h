#ifndef __WUKONG_H
#define __WUKONG_H

extern "C" {
	void wukong_setup(void);
	void wukong_loop(void);
#include "wkpf/wkpf.h"
#include "wkpf/wkpf_wuobjects.h"
#include "wkpf/wkpf_wuclasses.h"
#include "wkpf/wkpf_properties.h"
#include "wkpf/native_wuclasses.h"
#include "wkpf/native_wuclasses_privatedatatypes.h"
#include "wkpf/wkcomm.h"
};
void wkpf_dump(wuobject_t *obj)
{

}
wuclass_t classes[2] = {
	{
              WKPF_WUCLASS_BINARY_SENSOR,
              wkpf_dump,
              NULL,
              4,
              sizeof(short),
              0, // Initialise flags to 0, possibly set WKPF_WUCLASS_FLAG_APP_CAN_CREATE_INSTANCE from native_wuclasses_init
              NULL,
              {
                      WKPF_PROPERTY_TYPE_REFRESH_RATE+WKPF_PROPERTY_ACCESS_READWRITE,
                      WKPF_PROPERTY_TYPE_BOOLEAN+WKPF_PROPERTY_ACCESS_READWRITE,
                      WKPF_PROPERTY_TYPE_BOOLEAN+WKPF_PROPERTY_ACCESS_READWRITE,
                      WKPF_PROPERTY_TYPE_SHORT+WKPF_PROPERTY_ACCESS_READWRITE
              }
	},
	{0,0,0,0,0,0,NULL,{0}}
};
typedef void (*WKPFCallback)(wuobject_t *);
class Wukong {
	public:
		static const int BINARY_SENSOR=WKPF_WUCLASS_BINARY_SENSOR;
		static const int BINARY_SENSOR_REFRESH_RATE=WKPF_PROPERTY_BINARY_SENSOR_REFRESH_RATE;
	      	void begin() {
			wukong_setup();
		}

		void loop() {
			wukong_loop();
		}
		bool send(int addr, char *payload,int len) {
			return wkcomm_send_raw(addr, (uint8_t *)payload, len)==WKPF_OK;
		}
		bool enableWuClass(int clsID, WKPFCallback func) {
			int i;

			i = 0;
			while(classes[i].wuclass_id) {
				if (classes[i].wuclass_id == clsID) {
					classes[i].update = func;
					wkpf_register_wuclass(&classes[i]);
					return true;
				}
				i++;
			}
			return false;
		}
		int addWuObject(int clsID) {
			int port = 1;
			while(1) {
				int ret = wkpf_create_wuobject(clsID, port, 0,true);
				if (ret == WKPF_OK) return port;
				port++;
			}
		}
		void setPropertyRefreshRate(int port, unsigned char property,short value) {
			wuobject_t *obj;
		       	if (wkpf_get_wuobject_by_port(port,&obj)==WKPF_OK) {
				wkpf_internal_write_property_refresh_rate(obj, property, value);
			}
		}
};
#endif
