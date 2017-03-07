from globals import *
import gevent
from gevent.event import AsyncResult
import pynvc
import logging

signal_reconfiguration = False
signal_deployment = None

def signal_reconfig():
    global signal_reconfiguration
    logging.info('reconfiguration signal set')
    signal_reconfiguration = True

def signal_deploy(*args):
    global signal_deployment
    logging.info('deploy signal set')
    signal_deployment = args

def signal_handler():
    global signal_reconfiguration
    global signal_deployment
    while 1:
        if not is_master_busy():
            if signal_reconfiguration:
                if len(applications) > 0:
                    active_application().reconfiguration()
                    signal_reconfiguration = False

            if signal_deployment:
                if len(applications) > 0:
                    gevent.sleep(2)
                    active_application().deploy_with_discovery(*signal_deployment)
                    #node_ids = [info.nodeId for info in getComm().getActiveNodeInfos(force=True)]
                    #active_application().deploy(node_ids,*signal_deployment)
                    signal_deployment = None

        gevent.sleep(0.15)
