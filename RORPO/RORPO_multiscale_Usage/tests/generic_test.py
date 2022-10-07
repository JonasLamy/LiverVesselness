import unittest
import os


class TestGeneric(unittest.TestCase):
    output = "output"
    BUILD_DIR = os.environ.get('BUILD_DIR')
    SRC_DIR = os.environ.get('SRC_DIR')
    bin = BUILD_DIR + "/bin/RORPO_multiscale_usage"
