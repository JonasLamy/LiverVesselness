# LiverVesselness

This is the Repository for the journal article in review : 

**"A Benchmark Framework for Multi-Region Analysis of Vesselness Filters"** Jonas Lamy, Odyssee Merveille, Bertrand Kerautret, Nicolas Passat

For results on the work submitted to ICPR 2020, please see the ICPR_2020 branch.

The implementations of the seven vessel filtering methods are available and the results presented on the paper can be reproduced.
An installation-free online demonstration is also available:
[http://ipol-geometry.loria.fr/~kerautre/ipol_demo/LiverVesselnessIPOLDemo](http://ipol-geometry.loria.fr/~kerautre/ipol_demo/LiverVesselnessIPOLDemo/)

## DATA

Data for the benchmark comes from the modification of public datasets. The tweaked versions are available here : http://eidolon.univ-lyon2.fr/~jlamy/

Original data are available on the authors websites :

Ircad Dataset : https://www.ircad.fr/research/3d-ircadb-01/
Vascusynth Dataset : http://vascusynth.cs.sfu.ca/Data.html

Scripts to transform data are available in the scripts/BDDgenerator folder

## Dependancies
- Benchmark (C/C++)

CMake > 3.10.2 (https://cmake.org/)

ITK >= 5.2 : [Github page](https://github.com/InsightSoftwareConsortium/ITK)
recommanded options : Module_Thickness3D, ITK_USE_FFTWF, ITK_USE_FFTWD

Jsoncpp : [Github page](https://github.com/open-source-parsers/jsoncpp)

Boost Math : [Github page](https://github.com/boostorg/math). This is only required for bessel functions in OOF.

- scripts ( python 3.5+)
Pandas : (https://pandas.pydata.org/)
Numpy : (https://numpy.org/)
Sklearn : (https://sklearn.org/)
Matplotlib : (https://matplotlib.org/)

## Build
Livervesselness should be built with cmake and generate:
- An executable for each vesselness filter implemented (see [Vesselness filters](#vesselness-filters) for more details)
- A benchmark executable to easily compare different vesselness filters on the same dataset with different parameters (see [Benchmark](#benchmark) for more details)

## Vesselness filters

All vesselness filters are available as standalone programs. Look for each filter's help for usage.

### Antiga (Frangi)
```
./Antiga --input OneInputFromVolumeList --ouput antiga.nii --sigmaMin 2.4 --sigmaMax 2.6 --nbSigmaSteps 4 --alpha 0.7 --beta 0.1 --gamma 5
```


## Benchmark
### Purpose
This benchmarking tool was created to compare Vesselness filters in a common application.
It is decomposed in 3 steps :

1) **Parameters files creation**
This step uses python scripts to generate the parameters of the vesselness filters used by the benchmark.
2) **Filters computation**
For each parameter sets the vesselness filter is computed
3) **Computing metrics**
Several metrics are computed for each filter result and summed up in a csv file.
This step is also implemented in C/C++ as 3D data are expensive to process. 
4) **Analysing results**
The metrics are analysed using python scripts (ROC curves, mean metrics, etc.).

### Features
- Seven vesselness filters available (Sato, Frangi, Meijering, OOF(|lambda_1 + lambda_2|), OOF_GM (geometric mean), Jerman, Zhang, RORPO)
- The benchmark is designed to easily add extra vesselness filters.
- Supports all medical formats read by ITK (.mhd,.nii, etc.) as well as DICOM series
- Scripts to automate the vesselness filters parameters generation are provided.
- Computed metrics : confusions matrix (TP,FP,TN,FN), sensitivity, specificity, precision, accuracy, Dice, MCC
- Results saved in a CSV file.
- Metrics computed on 3 different region of interest (mask of an organ, vessels neighbourhood and vessels bifurcations)

### Usage
A Json settings file should be created and give to the benchmark executable as follows:

```
./Benchmark --settingsFile settingFile.json
```
or 
```
./Benchmark -s settingFile.json
```

#### Benchmark Settings file
The settings file options are the following :

- **name** : name of the benchmark. This name is used as the benchmark folder name as well as the results csv file name.
- **path** : path where the benchmark folder is created ( see [Benchmark Hierarchy](#benchmark-hierarchy) for more details )
- **inputVolumesList** : path to the .txt file listing the data used (see [Input Volumes list](#input-volumes-list) for more details)
- **algorithmSets** : path to the json file listing which vesselness filter to compute and their associated parameters
- **maskList** : The list of masks used in the benchmark. The order of the list must match the order of the input volume list. At least one mask is required. If you are interested in metrics on the full image, you can create a dummy mask for the whole image.
- **enhancementMask** : The name of the mask used as a region of interest for the enhancement filters. This parameter can be set with an empty string if no enhancement mask is used.*
- **nbThresholds** : Number of thresholds to compute the different metrics. Each metric compare a binary volume to the ground truth, the binary volume is obtained by thresholding the vesselness result. As the results are normalized between 0 and 1, nbThreshold = 5 would yield 5 thresholds: 0, 0.25, 0.5, 0.75, 1. This parameter also controls the amount of points of the ROC curves
- **removeResultsVolumes** : true/false. If true, the output volumes are discarded after the metrics computation. This option is useful to save disk space. 
- **rescaleFilters** : true/false. If true, the output of the filter is rescaled to 0, 1. This option might be usefull to compare metrics such as SNR/PSNR on the same level.

Example of settings file:
```
{
    "Settings":{
	"name":"newBenchark",
	"path":"/home/path/WhereIWant/My/BenchmarkToBe",
	"inputVolumesList":"fileLists/ircad_10_and_11.txt",
	"algorithmSets":"paramSets/minimal.json",
	"maskList":["Organ","Vessels","Bifurcations],
	"enhancementMask":"Organ",
	"nbThresholds":200,
	"removeResultsVolumes":false,
	"rescaleFilters":true /* Note: this option is only available with the journal version of the benchmark */
    }
}
```

For existing methods parameters, please refers to vesselness executables documentation and reference papers.

*Masks can greatly beneficiate some methods that relies on gobal information such that the biggest eigen value in the whole image, or a Kmean in a specific area.

### Benchmark Hierarchy
Running a benchmark will create the following hierarchy :
```
path/newBenchmark
	|_ csv
	|   |_ newBenchmark.csv
	|   |_ newBenchmark_dilatedVessels.csv
	|   |_ newBenchmark_bifurcations.csv
	|_ data1
	|	|_ ouputVesselness1.nii
	|	|_ ouputVesselness2.nii
	|	|_outputVesselness3.nii
	|_ data2
	|   |_ ouputVesselness1.nii
	|	|_ ouputVesselness2.nii
	|	|_outputVesselness3.nii
```
where:
- newBenchmark_Organ.csv contain the metrics computed on the full volume
- newBenchmark_Vessels.csv contains the metrics computed only in the dilated vessel mask 
- newBenchmark_Bifurcations.csv contains the metrics computed only in the mask of the bifurcations
- data1, data2 are the name of the two input volumes in the inputVolumesList file
- ouputVesselness1.nii, ouputVesselness2.nii, ouputVesselness3.nii are the vesseless filter results of each method specified in the methods parameters file.

### Input Volumes list
For a given data, for instance liver CTs, the input volume list requires an input volume, several masks and a groundtruth. The supported Image types are listed in the table below:

| Volume      | Other (.mhd,.nii,etc.) | Dicom |
|-------------|------------------------|-------|
| input volume| double                 | int_16|
| masks       | uint_8                 | uint_8|
| ground truth| uint_8                 | uint_8|

Other types will require to tweak the C++ templates in the code...

The input volume list is a .txt file listing all necessary volumes for the benchmark in the following order :

- A unique ID/Name
- The path to the raw data
- The path to the vessels groundtruth
- The path to the organ mask
- The path to the bifurcations mask
- The path to the dilated vessels mask
 

Note that the unique ID is used to create a folder containing all its vesselness results. This corresponds to data1 and data2 in the previous section.

Example :
```
3Dircadb1.10 // ID of the first sequence 
/DATA/ircad_iso_111/3Dircadb1.10/patientIso.nii // input data
/DATA/ircad_iso_111/3Dircadb1.10/vesselsIso.nii // groundtruth
/DATA/ircad_iso_111/3Dircadb1.10/liverMaskIso.nii // 1rst mask 
/DATA/ircad_iso_111/3Dircadb1.10/bifurcationsMaskIso.nii // 2nd mask 
/DATA/ircad_iso_111/3Dircadb1.10/dilatedVesselsMaskIso.nii // 3rd mask

3Dircadb1.11 // ID of the second sequence
/DATA/ircad_iso_111/3Dircadb1.11/patientIso.nii // input data
/DATA/ircad_iso_111/3Dircadb1.11/vesselsIso.nii // groundtruth
/DATA/ircad_iso_111/3Dircadb1.11/liverMaskIso.nii // 1rst mask 
/DATA/ircad_iso_111/3Dircadb1.11/bifurcationsMaskIso.nii // 2nd mask 
/DATA/ircad_iso_111/3Dircadb1.11/dilatedVesselsMaskIso.nii // 3rd mask 
...
```

Notes: 
The number and order of masks in this file must match the number of masks declared in  the settings file.
Be careful not to add an extra end of line caracter at the end of the input volume list file.

### methods Parameters

The parameter file lists the parameters used for each vesselness filter.

Each parameter set is defined by :
- The name of the vesselness function (i.e the name of the vesselness executable)
- The name of the output.
- The list of parameters of the vesselness executable.

Here is an example :
```
{
 "Antiga" :
    [
	 {
	    "Output":"antiga.nii",
	    "Arguments":[
		{"sigmaMin":"2.4"},
		{"sigmaMax":"2.6"},
		{"nbSigmaSteps":"4"},
		{"alpha":"0.7"},
		{"beta":"0.1"},
		{"gamma":"5"}
	    ]
	 }
    ],
	"Jerman" :
    [
	 {
	    "Output":"jerman.nii",
	    "Arguments":[
		{"sigmaMin":"2.0"},
		{"sigmaMax":"2.2"},
		{"nbSigmaSteps":"5"},
		{"tau":"0.75"}
	    ]
	 },
	 {
	    "Output":"jerman.nii",
	    "Arguments":[
		{"sigmaMin":"1.0"},
		{"sigmaMax":"3.0"},
		{"nbSigmaSteps":"3"},
		{"tau":"0.2"}
	    ]
	 }
    ]
}
```

Note that a parameter file can be composed of one or several vesselness with one or several instances of parameters.

for instance, when testing one instance of each vesselness "frangi.nii", "Jerman.nii", "Sato.nii", naming scheme is suitable. When benchmarking parameters, a naming scheme with the value of the parameters is more practical. For example, "0.1-0.5.nii","0.5-0.5.nii" for testing the influence of the pair of parameters alpha and beta for frangi's vesselness. 

### Adding a custom vesselness

As stated before, the benchmark is designed to use stand alone vesselness functions.
Any extra vesselness filter can be included by following these two steps:
1) Put the new vesselness filter executable in the same directory as the Benchmark executable so that it can be called using bash command ./YourVesselnessName.
2) The new vesselness filter executable should be called using --option. Also --input and --ouput are compulsory parameters.
The minimal call of a new filter should be : ./YourVesselness --input inputVolume.nii --output outputVolume.nii
3) An optionnal --mask option is also prefered, when used it should return a masked version of the vesselness. Vesselness pixels masked by the foreground pixels values should be preserved.

### Analyse scripts
These scripts compute, per parameter set, the mean of the binary filter output that maximize the MCC over the "Organ" ROI.

The csv metric files can be processed using two files AnalyseBenchmark.py and aggregateResults.py

#### AnalyseBenchmark.py

This script expects output benchmark folders with names in the form {database name}_{optimization step}_{filter name} for example 'ircad_PS_Frangi'.

usage:
```
python3 analyseBenchmark.py ircad_PS_Frangi
```

This scripts create one folder per ROI mask, a folder named Summary and a Pickle Folder.

The summary folder contains the MCC, Dice, SNR, PSNR for all ROI of the best parameter set.
Each ROI folder contains a number of results files: 

```
ROI
|_ Best_mean_metric
|	|_ XXX_Best_CF_M.csv : mean metrics values for the maximized metric M per parameter set
|	|_ XXX_Best_mean_M.csv : mean metrics values for the maximized metric M per parameter set
|_ Best_metric_per_volume
|	|_ XXX_Best_M_per_volume.csv : Metrics values for the threshold maximizing the metric M for all pairs {volume, parameter set}
|	|_ XXX_Best_M_per_volume_summary.csv : Metrics values for the threshold maximizing the metric M the best pair {volume, parameter set}
|_ mean_metric
	|_ XXX_mean_M.csv : mean metrics values in ROI for the parameter set chosen by the optimization of metric M in the Organ ROI
	|_ XXX_mean_CF_M.csv : mean metrics values in ROI for the parameter set chosen by the optimization of metric M in the Organ ROI
```

#### agregrateResults.py

This scripts agregate the results of each filters into pdf and csv files.

With the following hierarchy :
```
Benchmark
|_ ircad_PS_Frangi
|_ ircad_PS_Sato
|_ ircad_PS_Jerman
```
usage: 
```
python3 agregateResults.py Benchmark
```

The command will create several files with a summary of the best parameters and associated metrics of each filters present in the folder.
