import os, sys
import unittest
from noseOfYeti.tokeniser.support import noy_sup_setUp
from should_dsl import *
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../wkpf'))
from wkpf import *
