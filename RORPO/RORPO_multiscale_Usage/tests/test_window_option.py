import subprocess
import glob
import uuid
import os
import re
from random import randint
from .generic_test import TestGeneric
import nibabel as nib
import numpy as np


class TestWindowOption(TestGeneric):
    range_pattern = re.compile(r'(?i)(Convert image intensity range from)')
    uint8_pattern = re.compile(r'(?i)(Convert image to uint8)')
    max_val = 300

    def test_window(self):
        for path in glob.glob(self.SRC_DIR + '/data/positive*.nii'):
            output_path = os.path.join(self.BUILD_DIR, self.output + str(uuid.uuid4()) + ".nii")
            args = [self.bin, "--input=" + path, "--output=" + output_path, "--scaleMin=40",
                    "--factor=1.32", "--nbScales=1", "--verbose",
                    '--window=%d,%d' % (randint(-self.max_val, self.max_val), randint(-self.max_val, self.max_val))]
            proc = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            outs, errs = proc.communicate(timeout=15)

            # check return value is 0
            assert (proc.returncode == 0)

            # check if file exists
            assert(os.path.exists(output_path))

            # check output type is uint8
            img = nib.load(output_path)
            assert(img.header.get_data_dtype() == np.dtype(np.uint8))

            # remove generated output image
            os.remove(output_path)

            # check if program generate expected messages
            outs = str(outs)
            assert (self.range_pattern.search(outs) is not None)
            assert (self.uint8_pattern.search(outs) is not None)
