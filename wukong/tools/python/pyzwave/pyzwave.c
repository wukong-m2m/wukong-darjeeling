// vim: ts=2 sw=2
#include <regex.h>
#include <Python.h>

#include "pyzwave-testrtt.h"

#define ADD_NODE_ANY		0x1
#define ADD_NODE_CONTROLLER 	0x2
#define ADD_NODE_SLAVE		0x3
#define ADD_NODE_EXISTING	0x4
#define ADD_NODE_STOP		0x5
#define ADD_NODE_STOP_FAILED	0x6
#define ADD_NODE_OPTION_HIGH_POWER	0x80

int initialised = 0;

extern char init_data_buf[256];
extern unsigned int zwave_my_address;


//#define DEBUGF(...)  fprintf(stderr, __VA_ARGS__)
#define DEBUGF(...)


static PyObject* pyzwave_receive(PyObject *self, PyObject *args) {
  int wait_msec, len;

  if (!initialised) {
    PyErr_SetString(PyExc_IOError, "Call pyzwave.init first.");
    return NULL;
  }
  
  if (!PyArg_ParseTuple( args, "i", &wait_msec))
    return NULL;
  
  len = PyZwave_receive(wait_msec);
  if (len == 0) {
    PyObject* return_value_list = PyList_New(0);
    PyList_Append(return_value_list, Py_None);
    PyList_Append(return_value_list, Py_None);
    return return_value_list;
  } else {
    int i;

    DEBUGF("PYZWAVE: Received %i bytes: ", len);
    for (i=0; i<len; i++)
      DEBUGF("[%x] ", PyZwave_messagebuffer[i]);
    DEBUGF("\n");

    PyObject* message_list = PyList_New(0);
    for (i=0; i<len; i++) {
      PyList_Append(message_list, PyInt_FromLong((long)PyZwave_messagebuffer[i] & 0xFF));
    }
    PyObject* return_value_list = PyList_New(0);
    PyList_Append(return_value_list, PyInt_FromLong((long)PyZwave_src));
    PyList_Append(return_value_list, message_list);
    return return_value_list;
  }
}

static PyObject* pyzwave_init(PyObject *self, PyObject *args) {
  char *host_or_dev_name;
  regex_t regex;
  int regi;

  regi = regcomp(&regex, "^[[:digit:]]{1,3}\.[[:digit:]]{1,3}\.[[:digit:]]{1,3}\.[[:digit:]]{1,3}$", REG_EXTENDED | REG_NOSUB | REG_ICASE);
  if (regi) { fprintf(stderr, "Could not compile regex\n"); return NULL; }
  
  printf("pyzwave_init\n");
  if (!PyArg_ParseTuple( args, "s", &host_or_dev_name)) return NULL;
  printf("PyArg_ParseTuple\n");
 
  regi = regexec(&regex, host_or_dev_name, 0, NULL, 0);
  printf("%s\n", host_or_dev_name);
  printf("%d\n", regi);
  if (!regi) {
    printf("is IP\n");
    if (PyZwave_init(host_or_dev_name) < 0) {
      PyErr_SetString(PyExc_IOError, "Call to zwave_init failed.");
      return NULL;
    }
    printf("PyZwave_init\n");
  } else if (regi == REG_NOMATCH) {
    printf("Not IP\n");
    if (PyZwave_init_usb(host_or_dev_name) < 0) {
      PyErr_SetString(PyExc_IOError, "Call to zwave_init failed.");
      return NULL;
    }
    printf("PyZwave_init_usb\n");
  } else {
    printf("Match failed\n");
  }

  regfree(&regex);

  initialised = 1;
  Py_RETURN_NONE;
}

static PyObject* pyzwave_send(PyObject *self, PyObject *args) {
  int dest_address, i, length;
  PyObject *data;
  uint8_t buf[256];

  if (!initialised) {
    PyErr_SetString(PyExc_IOError, "Call pyzwave.init first.");
    return NULL;
  }
  
  if (!PyArg_ParseTuple( args, "iO!", &dest_address, &PyList_Type, &data))
    return NULL;

  length = (int)PyList_Size(data);
  if (length < 0) {
    PyErr_SetString(PyExc_TypeError, "Second argument must be a list of bytes");
    return NULL;
  }
  if (length > 256) {
    PyErr_SetString(PyExc_ValueError, "Payload cannot be longer than 256 bytes");
    return NULL;
  }
  
  for (i=0; i<length; i++) {
    PyObject *byteAsObject = PyList_GetItem(data, i); /* Can't fail */
    long byteAsLong = PyInt_AsLong(byteAsObject);
    if (byteAsLong > 255 || byteAsLong < 0) {
      PyErr_SetString(PyExc_ValueError, "Data must consist of single bytes");
      return NULL;
    }
    buf[i] = (uint8_t)byteAsLong;
  }

  DEBUGF("PYZWAVE: Sending %i bytes to %i: ", length, dest_address);
  for (i=0; i<length; i++) {
    DEBUGF("[%x] ", buf[i]);
  }
  if(PyZwave_send(dest_address, buf, length) == 0) {
    DEBUGF("\nPYZWAVE: Done.\n");
    Py_RETURN_NONE;
  } else {
    DEBUGF("pyzwave.c pyzwave_send: Call to ZW_senddata failed.\n");
    PyErr_SetString(PyExc_IOError, "Call to ZW_senddata failed.");
    return NULL;
  }
}

static PyObject* pyzwave_setdebug(PyObject *self, PyObject *args) {
  int print_debug_info;
  if (!PyArg_ParseTuple( args, "i", &print_debug_info))
    return NULL;
  PyZwave_print_debug_info = print_debug_info;
  Py_RETURN_NONE;
}

static PyObject* pyzwave_poll(PyObject *self, PyObject *args) {

  while (1) {

    fd_set rs;
    struct timeval to;
    char c;
    int n;
    int interval = 500;
    int zwavefd = PyZwave_zwavefd();

    to.tv_sec = interval/1000;
    to.tv_usec = (interval%1000)*1000;

    FD_ZERO(&rs);
    FD_SET(zwavefd, &rs);
#ifdef _WIN32	
    FD_SET(sfd_commu, &rs);
#else //_WIN32	
    FD_SET(STDIN_FILENO, &rs);
#endif //_WIN32	

    n = select(FD_SETSIZE,&rs,NULL,NULL, &to);
    if (n < 0) {
      printf("Z-Wave device file is closed !!!\n");
      return Py_BuildValue("");
    }
    else if (n == 0) {	// timeout
      printf("select timeout\n");
      /*return Py_BuildValue("");*/
      break;
    }

    if (FD_ISSET(zwavefd,&rs)) {
      int len=read(zwavefd,&c,1);
      if (len > 0) {
        zwave_check_state(c);
      }
    }

  }

  char ret[1024];
  strcpy(ret, PyZwave_status());
  PyZwave_clearstatus();
  return PyString_FromString(ret);
}

static PyObject* pyzwave_add(PyObject *self, PyObject *args) {
  if (!initialised) {
    PyErr_SetString(PyExc_IOError, "Call pyzwave.init first.");
    return NULL;
  }

  if (ZW_AddNodeToNetwork(ADD_NODE_ANY|ADD_NODE_OPTION_HIGH_POWER) < 0) {
    return NULL;
  }

  Py_RETURN_NONE;
}

static PyObject* pyzwave_delete(PyObject *self, PyObject *args) {
  if (!initialised) {
    PyErr_SetString(PyExc_IOError, "Call pyzwave.init first.");
    return NULL;
  }
  if (ZW_RemoveNodeFromNetwork(ADD_NODE_ANY|ADD_NODE_OPTION_HIGH_POWER) < 0) {
    return NULL;
  }

  Py_RETURN_NONE;
}

static PyObject* pyzwave_stop(PyObject *self, PyObject *args) {
  if (!initialised) {
    PyErr_SetString(PyExc_IOError, "Call pyzwave.init first.");
    return NULL;
  }

  if (ZW_AddNodeToNetwork(ADD_NODE_STOP) < 0) {
    return NULL;
  }

  Py_RETURN_NONE;
}

static PyObject* pyzwave_discover(PyObject *self, PyObject *args) {
	int i;
	PyObject* message_list;
	if (!initialised) {
    	PyErr_SetString(PyExc_IOError, "Call pyzwave.init first.");
    	return NULL;
  	}
	PyZwave_discover();
 	message_list = PyList_New(0);
	PyList_Append(message_list, PyInt_FromLong((long)zwave_my_address & 0xFF));
    for (i=0; i<init_data_buf[0]+1; i++) {
      PyList_Append(message_list, PyInt_FromLong((long)init_data_buf[i] & 0xFF));

    }
    return message_list;
}

static PyObject* pyzwave_routing(PyObject *self, PyObject *args) {
  int i;
  int node_id;
  PyObject* neighbor_list;

  if (!PyArg_ParseTuple( args, "i", &node_id))
    return NULL;

  if (!initialised) {
    PyErr_SetString(PyExc_IOError, "Call pyzwave.init first.");
    return NULL;
  }

  PyZwave_routing((unsigned)node_id);

  neighbor_list = PyList_New(0);
  for (i=1; i<init_data_buf[0]+1; i++) {
    PyList_Append(neighbor_list, PyInt_FromLong((long)init_data_buf[i] & 0xFF));
  }
  return neighbor_list;
}
static PyObject* pyzwave_getDeviceType(PyObject *self, PyObject *args) {
  int i;
  int node_id;
  PyObject* device_type;

  if (!PyArg_ParseTuple( args, "i", &node_id))
    return NULL;

  if (!initialised) {
    PyErr_SetString(PyExc_IOError, "Call pyzwave.init first.");
    return NULL;
  }

  PyZwave_getDeviceType((unsigned)node_id);

  device_type = PyList_New(0);
  PyList_Append(device_type, PyInt_FromLong((long)pyzwave_basic));
  PyList_Append(device_type, PyInt_FromLong((long)pyzwave_generic));
  PyList_Append(device_type, PyInt_FromLong((long)pyzwave_specific));
  return device_type;
}

static PyObject* pyzwave_getID(PyObject *self, PyObject *args) {
  int i;
  int node_id;

  if (!initialised) {
    PyErr_SetString(PyExc_IOError, "Call pyzwave.init first.");
    return NULL;
  }
  return PyInt_FromLong(PyZwave_get_id());
}

PyMethodDef methods[] = {
  {"init", pyzwave_init, METH_VARARGS, "Sets the IP address to connect to"},
  {"send", pyzwave_send, METH_VARARGS, "Sends a list of bytes to a node"},
  {"add", pyzwave_add, METH_VARARGS, "Goes into add mode"},
  {"delete", pyzwave_delete, METH_VARARGS, "Goes into delete mode"},
  {"stop", pyzwave_stop, METH_VARARGS, "Stop adding/deleting nodes"},
  {"poll", pyzwave_poll, METH_VARARGS, "Polling current status"},
  {"receive", pyzwave_receive, METH_VARARGS, "Receive data"},
  {"setdebug", pyzwave_setdebug, METH_VARARGS, "Turn debug info on or off"},
  {"discover", pyzwave_discover, METH_VARARGS, "discover nodes"},
  {"routing", pyzwave_routing, METH_VARARGS, "node neighbors"},
  {"getDeviceType", pyzwave_getDeviceType, METH_VARARGS, "device type"},
  {"getID", pyzwave_getID, METH_VARARGS, "get dongle ID"},
  {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC 
initpyzwave() {
    (void) Py_InitModule("pyzwave", methods); 
}
