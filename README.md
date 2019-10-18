# LiverVesselness
testRepository for vesselness trials

Attention, tester dans un premier temps avec des valeurs sigmaMin/sigmaMax proches, pour tester les besoins en RAM de l'algo.

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



TODO : mettre a jour les licences, auteurs, etc.