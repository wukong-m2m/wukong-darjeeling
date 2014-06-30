#ifndef WKPF_H
#define WKPF_H

#include "types.h"

typedef int16_t   wkpf_refresh_rate_t; // TODONR: should be changed later when I have a better implementation of the property store

#define WKPF_PROPERTY_TYPE_SHORT         0
#define WKPF_PROPERTY_TYPE_BOOLEAN       1
#define WKPF_PROPERTY_TYPE_REFRESH_RATE  2
#define WKPF_PROPERTY_ACCESS_READONLY    (1 << 7)
#define WKPF_PROPERTY_ACCESS_WRITEONLY   (1 << 6)
#define WKPF_PROPERTY_ACCESS_READWRITE   (WKPF_PROPERTY_ACCESS_READONLY+WKPF_PROPERTY_ACCESS_WRITEONLY)

#define WKPF_IS_READONLY_PROPERTY(x)     ((~x) & WKPF_PROPERTY_ACCESS_WRITEONLY)
#define WKPF_IS_WRITEONLY_PROPERTY(x)    ((~x) & WKPF_PROPERTY_ACCESS_READONLY)
#define WKPF_GET_PROPERTY_DATATYPE(x)    ((x) & ~WKPF_PROPERTY_ACCESS_READWRITE)

#define WKPF_OK                                               0
#define WKPF_ERR_WUOBJECT_NOT_FOUND                           1
#define WKPF_ERR_PROPERTY_NOT_FOUND                           2
#define WKPF_ERR_WUCLASS_NOT_FOUND                            3
#define WKPF_ERR_READ_ONLY                                    4
#define WKPF_ERR_WRITE_ONLY                                   5
#define WKPF_ERR_PORT_IN_USE                                  6
#define WKPF_ERR_WUCLASS_ID_IN_USE                            7
#define WKPF_ERR_OUT_OF_MEMORY                                8
#define WKPF_ERR_WRONG_DATATYPE                               9
#define WKPF_ERR_WUOBJECT_ALREADY_ALLOCATED                  10
#define WKPF_ERR_NEED_VIRTUAL_WUCLASS_INSTANCE               11
#define WKPF_ERR_NVMCOMM_SEND_ERROR                          12
#define WKPF_ERR_NVMCOMM_NO_REPLY                            13
#define WKPF_ERR_REMOTE_PROPERTY_FROM_JAVASET_NOT_SUPPORTED  14
#define WKPF_ERR_COMPONENT_NOT_FOUND                         15
#define WKPF_ERR_LOCATION_TOO_LONG                           16
#define WKPF_ERR_UNKNOWN_FEATURE                             17
#define WKPF_ERR_LINK_NOT_FOUND                              18
#define WKPF_ERR_ENDPOINT_NOT_FOUND                          19
#define WKPF_ERR_CANT_CREATE_INSTANCE_OF_WUCLASS			 20
#define WKPF_ERR_SHOULDNT_HAPPEN                           0xFF

// Need to make sure these codes don't overlap with other libs or the definitions in panic.h
#define WKPF_PANIC_ERROR_CREATING_LOCAL_OBJECTS             110
#define WKPF_PANIC_MISSING_BINARY_FILE                      111
#define WKPF_PANIC_PIN_NOT_SUPPORTED                        112

#endif // WKPF_H
