import os
from configobj import ConfigObj

ROOT_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
CONFIG_PATH = os.path.join(ROOT_PATH, 'wukong', 'config', 'master.cfg')
config = ConfigObj(CONFIG_PATH)

ZWAVE_GATEWAY_IP = config.get('ZWAVE_GATEWAY_IP', '')
MASTER_PORT = int(config.get('MASTER_PORT', 80))

LOCATION_ROOT = config.get('LOCATION_ROOT', 'WuKong')

DEPLOY_PLATFORMS = []
DEPLOY_PLATFORMS.append(config.get('DEPLOY_PLATFORM', 'avr_mega2560'))

SIMULATION = config.get('SIMULATION', 'false')
MONITORING = config.get('MONITORING', 'false')

#XML_PATH = os.path.join(ROOT_PATH, 'wukong', 'Applications')
COMPONENTXML_PATH = os.path.join(ROOT_PATH, 'wukong', 'ComponentDefinitions', 'WuKongStandardLibrary.xml')
TEMPLATE_DIR = os.path.join(ROOT_PATH, 'wukong', 'tools', 'xml2java')
JAVA_OUTPUT_DIR = os.path.join(ROOT_PATH, 'src', 'app', 'wkdeploy', 'java')
TESTRTT_PATH = os.path.join(ROOT_PATH, 'wukong', 'tools', 'python', 'pyzwave')
APP_DIR = os.path.join(ROOT_PATH, 'wukong', 'apps')
BASE_DIR = os.path.join(ROOT_PATH, 'wukong', 'master', 'baseapp')
MOCK_XML = os.path.join(ROOT_PATH, 'wukong', 'master', 'mock_discovery.xml')

NETWORKSERVER_ADDRESS = config.get('NETWORKSERVER_ADDRESS', '127.0.0.1')
NETWORKSERVER_PORT = int(config.get('NETWORKSERVER_PORT', 10008))

WKPFCOMM_AGENT = config.get('WKPFCOMM_AGENT', 'ZWAVE')

MONGODB_URL = config.get('MONGODB_URL', '')


DEVICE1 = '/dev/cu.usbserial-A900Z8AI'
DEVICE2 = '/dev/cu.usbserial-A98BV19T'
DEVICE3 = '/dev/cu.usbserial-A9OV7PTP'
VM1 = 'arduino/wutest_with_wuclass.cpp.hex'
TEST_DEVICES = [DEVICE1, DEVICE2, DEVICE3]
HEXFILES = [VM1, VM1, VM1]
APP_PATH = "applications/b5fba9ff24d0045d1377a05a46b32f68/"

TEST_LOCATION_STRENGTH_NUMBER = 100
TEST_LOCATION_ERROR_LENGTH = 50

TEST_DISCOVERY_STRENGTH_NUMBER = 50

TEST_PROPERTY_STRENGTH_NUMBER = 100
