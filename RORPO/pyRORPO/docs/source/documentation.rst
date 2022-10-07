*************
Documentation
*************

RPO
===

.. py:function:: pyRORPO.RPO(image, scale, spacing=None, origin=None, nbCores=1, dilationSize=3, verbose=False, mask=None)

	Compute the 7 orientations of the Robust Path Opening.

	:param numpy.ndarray image: Input image
	:param int scale: Path length smaller than the real curvilinear structure length
	:param List[float] spacing: The size of a pixel in physical space along each dimension
	:param List[float] origin: The origin of the image - the geometric coordinates of the index (0,0,0)
	:param int nbCores: Number of CPUs used for RPO computation
	:param int dilationSize: Size of the dilation for the noise robustness step.
	:param bool verbose: Activation of a verbose mode
	:param numpy.ndarray mask: Path to a mask image (0 for the background and 1 for the foreground)

	:return: 7 Robust Path Opening responses of 7 orientations .
	:rtype: numpy.ndarray


Example:

Open image with SimpleITK

.. code:: python

	import SimpleITK as sitk

	reader = sitk.ImageFileReader()
	reader.SetFileName(PATH_TO_IMAGE)
	itk_image = reader.Execute()


Prepare variables

.. code:: python

	import numpy as np

	im_arr = sitk.GetArrayFromImage(itk_image)
	scale = 80
	spacing = itk_image.GetSpacing()
	origin = itk_image.GetOrigin()
	mask = np.zeros(im_arr.shape, dtype="uint8")
	mask[50:-50, 50:-50, 50:-50] = 1

run RPO

.. code:: python

	import pyRORPO
	responses = pyRORPO.RPO(im_arr, scale, spacing, origin, nbCores=8, dilationSize=2)



RORPO
=====

.. py:function:: pyRORPO.RORPO(image, scale, spacing=None, origin=None, nbCores=1, dilationSize=2, verbose=False, mask=None)

	Compute the Ranking Orientations Response of Path Operators

	:param numpy.ndarray image: Input image
	:param int scale: Path length smaller than the real curvilinear structure length
	:param List[float] spacing: The size of a pixel in physical space along each dimension
	:param List[float] origin: The origin of the image - the geometric coordinates of the index (0,0,0)
	:param int nbCores: Number of CPUs used for RPO computation
	:param int dilationSize: Size of the dilation for the noise robustness step.
	:param bool verbose: Activation of a verbose mode
	:param numpy.ndarray mask: Path to a mask image (0 for the background and 1 for the foreground)

	:return: Ranking Orientations Response of Path Operators.
	:rtype: numpy.ndarray

.. code:: python

	import pyRORPO
	response = pyRORPO.RORPO(im_arr, scale, spacing, origin, nbCores=8, dilationSize=2)



RORPO_multiscale
================

.. py:function:: pyRORPO.RORPO_multiscale(image, scaleMin, factor, nbScale, spacing=None, origin=None, nbCores=1, dilationSize=2, verbose=False, mask=None)

	Compute the multiscale RORPO

	:param numpy.ndarray image: Input image
	:param float scaleMin: Path length smaller than the real curvilinear structure length
	:param float factor: Factor for the geometric sequence of scales; scale_(n+1) = factor * scale_(n)
	:param int nbScale: Number of scales
	:param List[float] spacing: The size of a pixel in physical space along each dimension
	:param List[float] origin: The origin of the image - the geometric coordinates of the index (0,0,0)
	:param int nbCores: Number of CPUs used for RPO computation
	:param int dilationSize: Size of the dilation for the noise robustness step.
	:param bool verbose: Activation of a verbose mode
	:param numpy.ndarray mask: Path to a mask image (0 for the background and 1 for the foreground)

	:return: the multiscale RORPO
	:rtype: numpy.ndarray

.. code:: python

	import pyRORPO
	response = pyRORPO.RORPO_multiscale(im_arr, scaleMin=80, factor=1.5, nbScale=4, spacing=spacing, origin=origin, nbCores=8, dilationSize=2)
