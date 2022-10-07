# RORPO Multiscale Usage

Compute the RORPO multiscale for blood vessels from a .nii image.

**Accepted type**: int8, uint8, int16, uint16, int32, uint32, float

***An isotropic image resolution is required (cubic voxels)***.
```USAGE:
RORPO_multiscale_usage --input=ImagePath --output=OutputPath --scaleMin=MinScale --factor=F --nbScales=NBS [--window=min,max] [--core=nbCores] [--dilationSize=Size] [--mask=maskPath] [--verbose] [--normalize] [--uint8] [--series]

Parameters:
    <imagePath>         path to .nii image (string)
    <outputPath>        path to write the resulting image (string)
    <scaleMin>          Minimum path length (int)
    <factor>            factor for the geometric sequence of scales; scale_(n+1) = factor * scale_(n) (float)
    <nbScales>          Number of scales (int)

Options:
    --core              Number of CPUs used for RPO computation (int)
    --dilationSize      Size of the dilation for the noise robustness step.
    --window            Intensity range from the input image (2 int: window_min, window_max)
                        Convert input image to uint8. Intensities inferior to window_min become 0,
                        intensities superior to window_max become 255.
                        Linear transformation between window_min and window_max.
    --mask              Path to a mask image (0 for the background and 1 for the foreground).
                        RORPO will only be computed in this mask. The mask image type must be uint8.
    --verbose           Activation of a verbose mode.
    --dicom             Specify that <imagePath> is a DICOM image.
    --normalize         Return a double normalized output image.
    --uint8             Convert input image into uint8.
```

Usage Example :
```
./RORPO_multiscale_usage --input input.nii --output output.nii --scaleMin 40 --factor 1.32 --nbScales 4
```

Usage Example :
```
./RORPO_multiscale_usage --input input.nii --output output.nii --scaleMin 40 --factor 1.32 --nbScales 4 --window 0,255 --verbose --core 4
```

## Test
To build tests, (re-)generate a build system with any CMAKE_BUILD_TYPE flags except release.
```
cmake [options] -DCMAKE_BUILD_TYPE=Debug <path-to-source>
make tests
```
To run tests:
```
make test
```

