import subprocess
import glob
import uuid
import os
import re
import numpy as np
from .generic_test import TestGeneric
import nibabel as nib


class TestNormalizeOption(TestGeneric):
    pattern = re.compile(r'(?i)(converting output image intensity\s*:\s*([0-9]*\s*-?\s*)*to \[0,1\])')

    def test_normalize(self):
        for path in glob.glob(self.SRC_DIR + '/data/positive*.nii'):
            output_path = os.path.join(self.BUILD_DIR, self.output + str(uuid.uuid4()) + ".nii")
            args = [self.bin, "--input=" + path, "--output=" + output_path, "--scaleMin=40",
                    "--factor=1.32", "--nbScales=1", "--verbose", '--normalize']
            proc = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            outs, errs = proc.communicate(timeout=15)

            # check return value is 0
            assert (proc.returncode == 0)

            # check if file exists
            assert(os.path.exists(output_path))

            # check type of output image is double
            img = nib.load(output_path)
            assert(img.header.get_data_dtype() == np.dtype(np.float64))

            # check that output image is normalized
            data = img.get_fdata()
            assert(data.min() >= 0)
            assert(data.max() <= 1)

            # remove generated output image
            os.remove(output_path)

            # check if program generate expected messages
            assert (proc.returncode == 0)
            assert (self.pattern.search(str(outs)) is not None)
