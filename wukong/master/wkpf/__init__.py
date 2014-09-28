import os, sys
mapper_path = os.path.abspath('../mapper')
service_path = os.path.abspath('../service')
sys.path.append(mapper_path)
sys.path.append(service_path)
from mapper import *
from service import *