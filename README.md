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

# Results

-MCC = Matthew's correlation
-Dice = Sørensen–Dice index
-ROC = min distance of points on the curve from the point(TPR=1,FPR=0)

Scale search is performed with the followings parameters :

- ScaleMin-ScaleMax-nbImagesPerScales
- Scale range search = [0.2,3.0] with a step of 0.2
- nbImagesPerScales is fixed to 4

RORPO is handled differently, a grid search is performed in this space :

--Length of path --

minBoundStartRORPO=20
minBoundEndRORPO=120
minBoundStepRORPO=10

--Path multiplicative factor--

factorStart=1.2
factorEnd=1.6
factorStep=0.2

--number of Images per scales--

stepRORPOMin=2
stepRORPOMax=4
stepLength=1

--

 RORPO and Meijering doesn't have parameters to optimize except scale space.

Scales are fixed based on best scales for each metric.
Scale Search -> ParameterSearch
MCC          -> MCC
Dice         -> Dice 
ROC          -> ROC

We might investigate ROC->MCC, ROC->Dice. As ROC has a larger Scale distribution.

-- Parameters list

- Sato : alpha1-alpha2
- Frangi : alpha1-beta
- Meijering : None
- OOF : sigma (pre-processing smoothing)
- Jerman : tau
- Zhang : tau
- RORPO : None

--


## Vascusynth + photometric artefacts + rician noise (sigma=20)
### Scale Space Optimization

*** Metrics
```
| Method                        |     MCC |    DICE |     ROC |
|-------------------------------+---------+---------+---------|
| Sato                          |   0.830 |   0.830 |   0.055 |
| Frangi                        |   0.796 |   0.795 |   0.060 |
| Meijering                     |   0.656 |   0.637 |   0.111 |
| OOF ($\lambda_1 + \lambda_2$) |   0.388 |   0.350 |   0.286 |
| Jerman                        | *0.836* | *0.833* |   0.055 |
| RORPO                         |   0.420 |   0.350 |   0.411 |
| Zhang                         |   0.760 |   0.759 | *0.052* |
```
*** Best scales
```
| Method                        |       MCC |      DICE |       ROC |
|-------------------------------+-----------+-----------+-----------|
| Sato                          | 1.4-3.0-4 | 1.4-2.6-4 | 1.6-3.0-4 |
| Frangi                        | 1.8-3.0-4 | 1.8-3.0-4 | 2.0-3.0-4 |
| Meijering                     | 1.4-2.4-4 | 1.6-2.4-4 | 1.4-3.0-4 |
| OOF ($\lambda_1 + \lambda_2$) | 1.6-3.0-4 | 1.4-3.0-4 | 1.4-3.0-4 |
| Jerman                        | 1.4-2.4-4 | 1.4-2.4-4 | 1.6-3.0-4 |
| RORPO                         |  70-1.2-2 |  70-1.2-2 |  70-1.4-2 |
| Zhang                         | 1.8-2.8-4 | 1.8-2.8-4 | 1.4-2.8-4 |
```
### Parameters Optimization

*** Metrics
```
| Method                        |     MCC |    DICE |     ROC |
|-------------------------------+---------+---------+---------|
| Sato                          | *0.846* | *0.842* | *0.052* |
| Frangi                        |   0.830 |   0.828 |   0.055 |
| Meijering                     |   0.656 |   0.637 |   0.111 |
| OOF ($\lambda_1 + \lambda_2$) |   0.510 |   0.500 |   0.143 |
| Jerman                        |   0.837 |   0.833 |   0.054 |
| RORPO                         |   0.420 |   0.350 |   0.411 |
| Zhang                         |   0.767 |   0.760 |   0.100 |
```
*** Best Parameters
```
| Method                        |     MCC |    DICE |     ROC |
|-------------------------------+---------+---------+---------|
| Sato                          | 1.5-1.8 | 0.9-1.2 | 1.8-3.0 |
| Frangi                        | 0.4-0.6 | 0.4-0.6 | 0.4-1.0 |
| Meijering                     |       X |       X |       X |
| OOF ($\lambda_1 + \lambda_2$) |     2.0 |     2.0 |     2.0 |
| Jerman                        |     0.8 |     0.8 |     0.6 |
| RORPO                         |       X |       X |       X |
| Zhang                         |     0.1 |     0.1 |    0.02 |
```
## IRCAD
## Scale Parameters Optimization
```
*** Metrics

| Method                        |     MCC |    DICE |     ROC |
|-------------------------------+---------+---------+---------|
| Sato                          |   0.344 |   0.358 |   0.350 |
| Frangi                        |   0.330 |   0.346 | *0.340* |
| Meijering                     |   0.178 |   0.188 |   0.440 |
| OOF ($\lambda_1 + \lambda_2$) |   0.140 |   0.158 |   0.540 |
| Jerman                        |   0.314 |   0.320 |   0.370 |
| RORPO                         |   0.292 |   0.284 |   0.748 |
| Zhang                         | *0.392* | *0.395* |   0.348 |
```
*** Best scales
```
| Method                        |       MCC |      DICE |         ROC |
|-------------------------------+-----------+-----------+-------------|
| Sato                          | 2.0-2.2-4 | 2.0-2.2-4 |   2.0-3.0-4 |
| Frangi                        | 2.4-2.6-4 | 2.4-2.6-4 | *2.2-3.0-4* |
| Meijering                     | 1.6-2.0-4 | 1.6-1.8-4 |   1.6-3.0-4 |
| OOF ($\lambda_1 + \lambda_2$) | 1.8-3.0-4 | 1.8-3.0-4 |   1.4-3.0-4 |
| Jerman                        | 2.0-2.2-4 |   2.0-2.2 |   1.8-3.0-4 |
| RORPO                         |  50-1.4-3 |  70-1.4-2 |    50-1.4-3 |
| Zhang                         | *2.4-2.6* | *2.4-2.6* |   2.2-3.0-4 |
```
## Parameters optimization
*** Metrics
```
| Method                        |     MCC |    DICE |   ROC |
|-------------------------------+---------+---------+-------|
| Sato                          |   0.345 |   0.360 | 0.349 |
| Frangi                        |   0.340 |   0.355 | 0.365 |
| Meijering +                   |   0.178 |   0.188 |  0.44 |
| OOF ($\lambda_1 + \lambda_2$) |   0.240 |   0.240 | 0.456 |
| Jerman                        |   0.322 |   0.330 | 0.368 |
| RORPO +                       |   0.292 |   0.284 | 0.748 |
| Zhang                         | *0.392* | *0.398* | 0.347 |
```
*** Best Parameters
```
| Method                        |     MCC |    DICE |     ROC |
|-------------------------------+---------+---------+---------|
| Sato                          | 0.6-3.0 | 0.6-3.0 | 0.6-3.0 |
| Frangi                        | 0.4-0.6 | 0.4-0.6 | 0.4-1.0 |
| Meijering                     |       X |       X |       X |
| OOF ($\lambda_1 + \lambda_2$) |     2.0 |     2.0 |     2.5 |
| Jerman                        |     0.2 |     0.2 |     0.2 |
| RORPO                         |       X |       X |       X |
| Zhang                         |     1.0 |     1.0 |     1.0 |
```