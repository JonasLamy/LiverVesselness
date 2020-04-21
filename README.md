# LiverVesselness
Repository for Vesselness benchmark

## Dependancies

ITK > 5.0 : [Github page](https://github.com/InsightSoftwareConsortium/ITK)
recommanded options : Module_Thickness3D, ITK_USE_FFTWF, ITK_USE_FFTWD

Jsoncpp : [Github page](https://github.com/open-source-parsers/jsoncpp)
Boost > 1.46.0 : needs program_option and filesystem




Warning : a wide range of scales can cause a big memory consuption.
All methods support Dicom inputs using the option --InPutIsDicom or -d
All methods support a masking option for vesselness intensity rescale.

## Antiga (generalized Frangi's vesselness)
```
./Antiga --input liver.nii --output result.nii --sigmaMin 1 --sigmaMax 5 --nbSigmaSteps 5 --alpha 0.5 --beta 0.5 --gamma 0.5
```
## Sato
```
./Sato --input liver.nii --output result.nii --sigmaMin 1 --sigmaMax 5 --nbSigmaSteps 5 --alpha1 0.5 --alpha2 0.5
```

## Jerman's Vesselness
```
./JermanVesselness --input liver.nii --output result.nii --tau 0.75 --sigmaMin 0.3 --sigmaMax 5 --nbSigmaSteps 8
```
## Meijering's Neuriteness
```
./MeijeringNeuriteness --input liver.nii --output result.nii --alpha -0.5 --sigmaMin 0.3 --sigmaMax 2 --nbSigmaSteps 4
```

## Rui Zhang's Vesselness
```
./RuiZhangVesselness --input liver.nii --output result.nii --tau 0.75 --sigmaMin 0.3 --sigmaMax 5 --nbSigmaSteps 8
```

## RORPO
The syntax of RORPO's arguments have been changed to match the benchmark.
Futhermore, the Image class has been improved to take into account input image spacing, origin and transformation matrix.
This allows overlapping of output volumes and GT in 3D Slicer visualization.

```
./RORPO --input liver.nii --output result.nii --scaleMin 1 --factor 2 --nbScales 4 --core 3
```
## OOF
```
./OOF --input liver.nii --output result.nii --sigmaMin 1 --sigmaMax 5 --nbSigmaSteps 5
```
## Benchmark 
```
(Nifti support - type:float/double)
./Benchmark --input filePath.txt --parametersFile parameters.json
(Dicom series support - type: int16)
./Benchmark --input filePath.txt --parametersFile parameters.json
```

The filePath file must have the following architeture
```
Name of the output folder 1
Path to input image 1
Path to organ mask image 1
Path to bifurcations mask image 1
Path to dilated vessels mask image 1 
Path to vessels ground truth image 1
Name of the output folder 2
Path to input image 2
Path to organ mask image 2
Path to bifurcations mask image 2
Path to dilated vessels mask image 2 
Path to vessels ground truth image 2

example:

3Dircadb1.10
/DATA/ircad_iso_111/3Dircadb1.10/patientIso.nii
/DATA/ircad_iso_111/3Dircadb1.10/liverMaskIso.nii
/DATA/ircad_iso_111/3Dircadb1.10/bifurcationsMaskIso.nii
/DATA/ircad_iso_111/3Dircadb1.10/dilatedVesselsMaskIso.nii
/DATA/ircad_iso_111/3Dircadb1.10/vesselsIso.nii
```

The parameter file must have the following form :
```
{  
    "Executable Name" :
    [
	{
	    "Output":"OutputFileName.nii",
	    "Arguments":[
		{"arg 1":"value"},
		{"arg 2":"value"},
		{"arg 3":"value"},
		{"arg 4":"value"},
		{"arg 5":"value"}
	    ]
	}
    ]
}
```
Example:
```
{  
    "RORPO_multiscale_usage" :
    [
	{
	    "Output":"rorpo.nii",
	    "Arguments":[
		{"scaleMin":"30"},
		{"factor":"1.6"},
		{"nbScales":"4"},
		{"core":"4"},
		{"verbose":""}
	    ]
	}
    ]
}
```

The benchmark detects if input images (input,gt,mask) are DICOMs by checking if GT path leads to a directory or not.
It is possible to compute metrics without keeping output volumes (for memory management) using the option -r.
It is possible to rescale responses according to a mask using the option --useMask # ( Organ mask:0  Dilated vessels mask:1 Bifurcation mask:2).

