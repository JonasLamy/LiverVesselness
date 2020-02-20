# LiverVesselness
testRepository for vesselness trials

Warning : a wide range of scales can cause a big memory consuption.
All methods support Dicom inputs using the option --InPutIsDicom or -d

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
Name of the folder 1
Path to input image 1
Path to mask image 1
Path to ground truth image 1
Name of the folder 2
Path to input image 2
Path to mask image 2
Path to ground truth image 2

example:

Group4_data4
/DATA/March_2013_VascuSynth_Dataset/Group4/data4/testVascuSynth4_101_101_101_uchar.mhd
/DATA/March_2013_VascuSynth_Dataset/maskWholeImage.nii
/DATA/March_2013_VascuSynth_Dataset/Group4/data4/gt.nii
Group4_data10
/DATA/March_2013_VascuSynth_Dataset/Group4/data10/testVascuSynth10_101_101_101_uchar.mhd
/DATA/March_2013_VascuSynth_Dataset/maskWholeImage.nii
/DATA/March_2013_VascuSynth_Dataset/Group4/data10/gt.nii
```

The benchmark detects if input images (input,gt,mask) are DICOMs by checking if GT path leads to a directory or not.
It is possible to compute metrics without keeping output volumes (for memory management) using the option -r.