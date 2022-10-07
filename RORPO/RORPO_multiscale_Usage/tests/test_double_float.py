import subprocess
import glob
import uuid
import os
import re
import numpy as np
from .generic_test import TestGeneric
import nibabel as nib


class TestNegative(TestGeneric):
    double_pattern = re.compile(r'(?i)(Input image type\s*:*\s* double)')
    float_pattern = re.compile(r'(?i)(Input image type\s*:*\s* float)')
    conversion_pattern = re.compile(r'(?i)(Convert image to uint8)')

    def run_test_(self, image_type):
        if image_type == 0:
            paths = glob.glob(self.SRC_DIR + '/data/positive_float*.nii')
            pat = self.float_pattern
        else:
            paths = glob.glob(self.SRC_DIR + '/data/positive_double*.nii')
            pat = self.double_pattern
        for path in paths:
            output_path = os.path.join(self.BUILD_DIR, self.output + str(uuid.uuid4()) + ".nii")
            args = [self.bin, "--input=" + path, "--output=" + output_path, "--scaleMin=40",
                    "--factor=1.32", "--nbScales=1", "--verbose"]
            proc = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            outs, errs = proc.communicate(timeout=20)

            # check return value is 0
            assert (proc.returncode == 0)

            # check if file exists
            assert(os.path.exists(output_path))

            # check type of output image is uint8
            img = nib.load(output_path)
            assert(img.header.get_data_dtype() == np.dtype(np.uint8))

            # remove generated output image
            os.remove(output_path)

            # check if program generate expected messages
            outs = str(outs)
            assert (pat.search(outs) is not None)
            assert (self.conversion_pattern.search(outs) is not None)
            assert ("computation" in outs)

    def test_double(self):
        self.run_test_(1)

    def test_float(self):
        self.run_test_(0)
