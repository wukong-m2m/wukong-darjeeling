#include "debug.h"
#include "native_wuclasses.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"

void wuclass_ics_demo_floorlamp_pr_class_setup(wuobject_t *wuobject) {}
void wuclass_ics_demo_floorlamp_pr_class_update(wuobject_t *wuobject) {
    bool on;
    int16_t bri, hue;
	wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_ICS_DEMO_FLOORLAMP_PR_CLASS_ON_OFF, &on);
	wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_ICS_DEMO_FLOORLAMP_PR_CLASS_BRIGHTNESS, &bri);
	wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_ICS_DEMO_FLOORLAMP_PR_CLASS_HUE, &hue);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(floor: value: %d %d %d\n", on, bri, hue);
}
void wuclass_ics_demo_bloom_pr_class_setup(wuobject_t *wuobject) {}
void wuclass_ics_demo_bloom_pr_class_update(wuobject_t *wuobject) {}
void wuclass_ics_demo_go_pr_class_setup(wuobject_t *wuobject) {}
void wuclass_ics_demo_go_pr_class_update(wuobject_t *wuobject) {}
void wuclass_ics_demo_strip_pr_class_setup(wuobject_t *wuobject) {}
void wuclass_ics_demo_strip_pr_class_update(wuobject_t *wuobject) {}
void wuclass_ics_demo_fan_pr_class_setup(wuobject_t *wuobject) {}
void wuclass_ics_demo_fan_pr_class_update(wuobject_t *wuobject) {}
void wuclass_ics_demo_aroma_pr_class_setup(wuobject_t *wuobject) {}
void wuclass_ics_demo_aroma_pr_class_update(wuobject_t *wuobject) {}
void wuclass_ics_demo_music_pr_class_setup(wuobject_t *wuobject) {}
void wuclass_ics_demo_music_pr_class_update(wuobject_t *wuobject) {}
void wuclass_ics_demo_tv_pr_class_setup(wuobject_t *wuobject) {}
void wuclass_ics_demo_tv_pr_class_update(wuobject_t *wuobject) {}
void wuclass_ics_demo_q_pr_class_setup(wuobject_t *wuobject) {}
void wuclass_ics_demo_q_pr_class_update(wuobject_t *wuobject) {}
