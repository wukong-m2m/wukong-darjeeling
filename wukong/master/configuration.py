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
