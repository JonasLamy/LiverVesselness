################ RORPO Multiscale Usage ##############

Compute the RORPO multiscale for blood vessels from a .nii image.

Accepted type : int8, uint8, int16, uint16, int32, uint32, float
An isotropic image resolution is required (cubic voxels). 

Parameters : 

<imagePath>: path to .nii image (string).
<outputPath>: path to write the resulting image (string).
<scaleMin>: Minimum path length (int)
<factor>: factor for the geometric sequence of scales; scale_(n+1) = factor * scale_(n) (float)
<nbScales>: Number of scales (int)

Options:
--window:     Intensity range from the input image (2 int: window_min, window_max)
		      Convert input image to uint8. Intensities inferior to window_min become 0, intensities superior to window_max become 255; Linear transformation between window_min and window_max
--core:       Number of CPUs used for RPO computation (int)
--mask or :   Path to a mask image (0 for the background and 1 for the foreground). RORPO will only be computed in this mask. The mask image type must be uint8.
--verbose:    Activation of a verbose mode.

Usage Example : ./RORPO_multiscale_usage input.nii output.nii 40 1.32 4 
Usage Example : ./RORPO_multiscale_usage input.nii output.nii 40 1.32 4 --window 0,255 --verbose --core 4

