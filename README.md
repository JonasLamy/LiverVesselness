# LiverVesselness
testRepository for vesselness trials

Attention, tester dans un premier temps avec des valeurs sigmaMin/sigmaMax proches, pour tester les besoins en RAM de l'algo.

## Antiga (Frangi Généralisé)
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
Todo : Checker la valeur optimal de lambda pour la hessienne modifiée. Obara donne une valeur sans preuve formelle.

## Rui Zhang's Vesselness
```
./RuiZhangVesselness --input liver.nii --output result.nii --tau 0.75 --sigmaMin 0.3 --sigmaMax 5 --nbSigmaSteps 8
```

## RORPO
syntaxe des arguments de RORPO différente du dépôt original pour automatiser le benchmark (docopt est toujours utilisé)

```
./RORPO --input liver.nii --output result.nii --scaleMin 1 --factor 2 --nbScales 4 --core 3
```
## OOF
```
./OOF --input liver.nii --output result.nii --sigmaMin 1 --sigmaMax 5 --nbSigmaSteps 5
```

TODO : mettre a jour les licences, auteurs, etc.

## Benchmark 
```
(Nifti support - type:float/double)
./Benchmark --input liver.nii --groundTruth gt_liver.nii --mask mask_liver.nii --parametersFile parameters.json
(Dicom series support - type: int16)
./Benchmark --input liver_directory/ --groundTruth gt_directory/ --mask mask_directory/ --parametersFile parameters.json
```
Le benchmark détecte si les images en input (input,gt,mask) sont du DICOM en regardant si le chemin du gt est un répertoire.