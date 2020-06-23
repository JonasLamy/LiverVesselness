# LiverVesselness

Repository of the work submitted to ICPR 2020:

**"Vesselness Filters: A Survey with Benchmarks Applied to Liver Imaging"**, Jonas Lamy, Odyssée Merveille, Bertrand Kerautret, Nicolas Passat, Antoine Vacavant, ICPR, 2020


The implementations of the seven vessel filtering methods are available and the results presented on the paper can be reproduced.
An installation-free online demonstration is also available:
[http://ipol-geometry.loria.fr/~kerautre/ipol_demo/LiverVesselnessIPOLDemo](http://ipol-geometry.loria.fr/~kerautre/ipol_demo/LiverVesselnessIPOLDemo/)

## Dependancies
- Benchmark (C/C++)
ITK > 5.0 : [Github page](https://github.com/InsightSoftwareConsortium/ITK)
recommanded options : Module_Thickness3D, ITK_USE_FFTWF, ITK_USE_FFTWD

Jsoncpp : [Github page](https://github.com/open-source-parsers/jsoncpp)
Boost > 1.46.0 : needs program_option and filesystem

- scripts ( python 3.5+)
Pandas : (https://pandas.pydata.org/)
Numpy : (https://numpy.org/)
Matplotlib : (https://matplotlib.org/)

## Build
Livervesselness should be build with cmake and generate:
- An executable for each vesselness filter implemented (see [Vesselness filters](#vesselness-filters) for more details)
- A benchmark executable to easily compare different vesselness filters on the same dataset with different parameters (see [Benchmark](#benchmark) for more details)

## Vesselness filters

### Antiga
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
- Seven vesselness filters available (Sato, Frangi, Meijering, OOF(|lambda_1 + lambda_2|), Jerman, Zhang, RORPO)
- The benchmark is design to easily add extra vesselness filters.
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
- **maskType** : The type of mask used; it can be "Organ","DilatedVessels","Bifurcations or "" (for No masks). * 
- **nbThresholds** : Number of thresholds to compute the different metrics. Each metric compare a binary volume to the ground truth, the binary volume is obtained by thresholding the vesselness result. As the results are normalized between 0 and 1, nbThreshold = 5 would yield 5 thresholds: 0, 0.25, 0.5, 0.75, 1. This parameter also controls the amount of points of the ROC curves
- **removeResultsVolumes** : true/false. If true, the output volumes are discarded after the metrics computation. This option is useful to save disk space. 

Example of settings file:
```
{
    "Settings":{
	"name":"newBenchark",
	"path":"/home/path/WhereIWant/My/BenchmarkToBe",
	"inputVolumesList":"fileLists/ircad_10_and_11.txt",
	"algorithmSets":"paramSets/minimal.json",
	"maskType":"",
	"nbThresholds":200,
	"removeResultsVolumes":false
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
	|       |_ ouputVesselness1.nii
	|	|_ ouputVesselness2.nii
	|	|_outputVesselness3.nii
```
where:
- newBenchmark.csv contain the metrics computed on the full volume
- newBenchmark_dilatedVessels.csv contains the metrics computed only in the dilated vessel mask 
- newBenchmark_bifurcations.csv contains the metrics computed only in the mask of the bifurcations
- data1, data2 are the name of the two input volumes in the inputVolumesList file
- ouputVesselness1.nii, ouputVesselness2.nii, ouputVesselness3.nii are the vesseless filter results of each method specified in the inputVolumesList file.

### Input Volumes list
For a given data, for instance liver CTs, the input volume list requires an input volume, several masks and a groundtruth. The supported types are listed in the table below:

| Volume      | Other (.mhd,.nii,etc.) | Dicom |
|-------------|------------------------|-------|
| input volume| double                 | int_16|
| masks       | uint_8                 | uint_8|
| ground truth| uint_8                 | uint_8|

The input volume list is a .txt file listing all necessary volumes for the benchmark in the following order :

- A unique ID/Name
- The path to the raw data
- The path to the organ mask
- The path to the bifurcations mask
- The path to the dilated vessels mask
- The path to the vessels groundtruth 

Note that the unique ID is used to create a folder containing all its vesselness results. This corresponds to data1 and data2 in the previous section.

Example :
```
3Dircadb1.10 // ID of the first sequence 
/DATA/ircad_iso_111/3Dircadb1.10/patientIso.nii // input data
/DATA/ircad_iso_111/3Dircadb1.10/liverMaskIso.nii // 1rst mask 
/DATA/ircad_iso_111/3Dircadb1.10/bifurcationsMaskIso.nii // 2nd mask 
/DATA/ircad_iso_111/3Dircadb1.10/dilatedVesselsMaskIso.nii // 3rd mask
/DATA/ircad_iso_111/3Dircadb1.10/vesselsIso.nii // groundtruth
3Dircadb1.11 // ID of the second sequence
/DATA/ircad_iso_111/3Dircadb1.11/patientIso.nii // input data
/DATA/ircad_iso_111/3Dircadb1.11/liverMaskIso.nii // 1rst mask 
/DATA/ircad_iso_111/3Dircadb1.11/bifurcationsMaskIso.nii // 2nd mask 
/DATA/ircad_iso_111/3Dircadb1.11/dilatedVesselsMaskIso.nii // 3rd mask 
/DATA/ircad_iso_111/3Dircadb1.11/vesselsIso.nii // groundtruth 
...
```
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

Note that a parameter file can be used to compute one or several vesselness filters.

### Adding a custom vesselness

As stated before, the benchmark is designed to use stand alone vesselness functions.
Any extra vesselness filter can be included by following these two steps:
1) Put the new vesselness filter executable in the same directory as the Benchmark executable so that it can be called using bash command ./YourVesselnessName.
2) The new vesselness filter executable should be called using --option. Also --input and --ouput are compulsory parameters.
The minimal call of a new filter should be : ./YourVesselness --input inputVolume.nii --output outputVolume.nii

### Analyse scripts
The scripts provided are used to find the mean of the best MCC, Dice and ROC dist as well as computing the ROC curves once the metrics are computed. 

- **parsing csv**
This script ouputs the best set of parameters and threshold according to each metric (MCC, Dice or Roc Dist) for each result (volume, parameter set and filter) of the benchmark. The outputs are distributed into 3 CSV files, one per metric.

Usage
```
python3 parseCSV.py pathToCSV/myResultCSVFile.csv

```
- **analysing results** (means per parameter sets)

This script computes the mean and standard deviation of the best MCC, Dice and ROC dist for each parameter sets and summarize it in a csv file.
The script needs the csv file from the benchmark as unique input as it will determine the others from it's name.
Moreover, you can choose to save the ROC curves for visualization.

```
python3 parseCSV.py pathToCSV/myResultCSVFile.csv 1 

output example :
ParameterSet ,MCC   ,MCC_std ,Dice,  Dice_std ,ROC_dist ,ROC_dist_std
antiga1.nii  ,0.347 ,0.021   ,0.356 ,0.021    ,0.609    ,0.145
antiga2.nii  ,0.418 ,0.030   ,0.435 ,0.024    ,0.345    ,0.161
antiga3.nii  ,0.340 ,0.028   ,0.352 ,0.024    ,0.338    ,0.106
meijering.nii,0.000 ,0.000   ,0.064 ,0.025    ,1.000    ,0.000


```
