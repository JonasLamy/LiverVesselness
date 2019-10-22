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
## Zhang's vesselness
Attention : La méthode de Zhang n'est pas bien définie par l'auteur....l'exposant -S^2/(2*alpha) ressemble à la combinaison de deux parties
distinctes de Frangi...l'implémentation choisie ici est -S^2/(2*c) décrite dans le papier de Frangi.

```
./ZhangVesselness --input liver.nii --ouput result.nii --tau 0.75 --sigmaMin 0.3 --sigmaMax 5 --nbSigmaSteps 8
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