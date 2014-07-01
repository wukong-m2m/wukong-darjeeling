from configuration import *
active_ind = 0
applications = []
location_tree = None
wukong_status = []
connected = (False if SIMULATION == "true" else True)  # whether zwave gateway is connected

MASTER_BUSY = False

def is_not_connected():
  global connected
  connected = False

def is_master_busy():
    global MASTER_BUSY
    return MASTER_BUSY

def master_busy():
    global MASTER_BUSY
    MASTER_BUSY = True

def master_available():
    global MASTER_BUSY
    MASTER_BUSY = False

def active_application():
  global applications
  global active_ind
  try:
    return applications[active_ind]
  except:
    return None

def set_active_application_index(new_index):
  global active_ind
  active_ind = new_index

def get_all_wukong_status():
  global wukong_status
  return wukong_status

def get_wukong_status():
  global wukong_status
  if len(wukong_status) > 0:
    return wukong_status[len(wukong_status)-1]
  else:
    return ""

def set_wukong_status(status):
  global wukong_status
  wukong_status.append(status)
