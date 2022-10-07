import subprocess
import glob
import uuid
import os
import re
from .generic_test import TestGeneric


class TestNegative(TestGeneric):
    error_pattern = re.compile(r'(?i)(Image contains negative values)')

    def test_positive(self):
        for path in glob.glob(self.SRC_DIR + '/data/positive*.nii'):
            output_path = os.path.join(self.BUILD_DIR, self.output + str(uuid.uuid4()) + ".nii")
            args = [self.bin, "--input=" + path, "--output=" + output_path, "--scaleMin=40",
                    "--factor=1.32", "--nbScales=1"]
            proc = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            outs, errs = proc.communicate(timeout=15)

            # check return value is 0
            assert (proc.returncode == 0)

            # check if file exists
            assert(os.path.exists(output_path))

            # remove generated output image
            os.remove(output_path)

            # check if program generate expected messages
            outs, errs = str(outs), str(errs)
            assert ("computation" in outs)
            assert (self.error_pattern.search(errs) is None)

    def test_negative(self):
        for path in glob.glob(self.SRC_DIR + '/data/negative*.nii'):
            output_path = self.output + str(uuid.uuid4()) + ".nii"
            args = [self.bin, "--input=" + path, "--output=" + output_path, "--scaleMin=40",
                    "--factor=1.32", "--nbScales=1"]
            proc = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            outs, errs = proc.communicate(timeout=15)

            # check return value is 1
            assert (proc.returncode == 1)

            # check if file does not exist
            assert(not os.path.exists(output_path))

            # check if program generate expected messages
            outs, errs = str(outs), str(errs)
            assert ("computation" not in outs)
            assert (self.error_pattern.search(errs) is not None)
