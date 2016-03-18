#ifndef __WUKONG_H
#define __WUKONG_H

extern "C" {
	void wukong_setup(void);
	void wukong_start(void);
	void wukong_loop(void);
#include "wkpf/wkpf.h"
#include "wkpf/wkpf_main.h"
#include "wkpf/wkpf_wuobjects.h"
#include "wkpf/wkpf_wuclasses.h"
#include "wkpf/wkpf_properties.h"
#include "wkpf/native_wuclasses.h"
#include "wkpf/native_wuclasses_privatedatatypes.h"
#include "wkpf/wkcomm.h"
#define WUKONG_HAS_VM
};
void wkpf_dump(wuobject_t *obj)
{

}
/*
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
*/
#include "wkpf/GENERATEDwuclass_arduinoIDE.h"

typedef void (*WKPFCallback)(wuobject_t *);
class Wukong {
	public:
		/*
		static const int BINARY_SENSOR=WKPF_WUCLASS_BINARY_SENSOR;
		static const int BINARY_SENSOR_REFRESH_RATE=WKPF_PROPERTY_BINARY_SENSOR_REFRESH_RATE;
	 	*/
		#include "wkpf/GENERATEDwkpf_wuclass_library_arduinoIDE.h"     
		void begin() {
			wukong_setup();
		}

		void start() {
			wukong_start();
		}

		void loop() {
			wukong_loop();
		}
		bool send(int addr, char *payload,int len) {
			return wkcomm_send_raw(addr, (uint8_t *)payload, len)==WKPF_OK;
		}
		bool enableWuClass(int clsID, WKPFCallback func,bool can_create=false) {
			int i;

			i = 0;
			while(classes[i].wuclass_id) {
				if (classes[i].wuclass_id == clsID) {
					classes[i].update = func;
					if (can_create)
						classes[i].flags = WKPF_WUCLASS_FLAG_APP_CAN_CREATE_INSTANCE;
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
		void setPropertyBool(int port, unsigned char property,bool value) {
			wuobject_t *obj;
		       	if (wkpf_get_wuobject_by_port(port,&obj)==WKPF_OK) {
				wkpf_internal_write_property_boolean(obj, property, value);
			}
		}
		void setPropertyShort(int port, unsigned char property,short value) {
			wuobject_t *obj;
		       	if (wkpf_get_wuobject_by_port(port,&obj)==WKPF_OK) {
				wkpf_internal_write_property_int16(obj, property, value);
			}
		}
                void setPropertyRefreshRate(int port, unsigned char property,short value) {
			wuobject_t *obj;
		       	if (wkpf_get_wuobject_by_port(port,&obj)==WKPF_OK) {
				wkpf_internal_write_property_refresh_rate(obj, property, value);
			}
		}
		void getPropertyBool(int port, unsigned char property,bool *value) {
			wuobject_t *obj;
		       	if (wkpf_get_wuobject_by_port(port,&obj)==WKPF_OK) {
				wkpf_internal_read_property_boolean(obj, property, value);
			}
		}
		void getPropertyShort(int port, unsigned char property,int *value) {
			wuobject_t *obj;
		       	if (wkpf_get_wuobject_by_port(port,&obj)==WKPF_OK) {
				wkpf_internal_read_property_int16(obj, property, value);
			}
		}
		void getPropertyRefreshRate(int port, unsigned char property,int *value) {
			wuobject_t *obj;
		       	if (wkpf_get_wuobject_by_port(port,&obj)==WKPF_OK) {
				wkpf_internal_read_property_refresh_rate(obj, property, value);
			}
		}
   
 	        void setPropertyShort(wuobject_t *obj, unsigned char property,short value) {
         		wkpf_internal_write_property_int16(obj, property, value);
		}
        	void setPropertyRefreshRate(wuobject_t *obj, unsigned char property,short value) {
            		wkpf_internal_write_property_refresh_rate(obj, property, value);
		}
		void setPropertyBool(wuobject_t *obj, unsigned char property,bool value) {
            		wkpf_internal_write_property_boolean(obj, property, value);
		}
		void getPropertyBool(wuobject_t *obj, unsigned char property,bool *value) {
	            	wkpf_internal_read_property_boolean(obj, property, value);
		}
		void getPropertyShort(wuobject_t *obj, unsigned char property,int *value) {
            		wkpf_internal_read_property_int16(obj, property, value);
		}
		void getPropertyRefreshRate(wuobject_t *obj, unsigned char property,int *value) {
            		wkpf_internal_read_property_refresh_rate(obj, property, value);
		}
};
#endif
