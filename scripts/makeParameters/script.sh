dirName=$1
# min scale pace bound
minBoundStart=0.4
minBoundEnd=6
minBoundStep=0.6
# max scale space bound
maxBoundStart=2
maxBoundEnd=6
maxBoundStep=0.6
# nbScales by interval [minBound;maxBound]
step=4

minBoundStartRORPO=10
minBoundEndRORPO=61
minBoundStepRORPO=10
factorStart=1.2
factorEnd=1.7
factorStep=0.2
stepRORPOMin=3
stepRORPOMax=5

python3 makeFrangiScales.py $minBoundStart $minBoundEnd $minBoundStep $maxBoundStart $maxBoundEnd $maxBoundStep $step > $dirName/FrangiScalesSearch.json
python3 makeMeijeringScales.py $minBoundStart $minBoundEnd $minBoundStep $maxBoundStart $maxBoundEnd $maxBoundStep $step > $dirName/MeijeringScalesSearch.json
python3 makeSatoScales.py $minBoundStart $minBoundEnd $minBoundStep $maxBoundStart $maxBoundEnd $maxBoundStep $step > $dirName/SatoScalesSearch.json
python3 makeJermanScales.py $minBoundStart $minBoundEnd $minBoundStep $maxBoundStart $maxBoundEnd $maxBoundStep $step > $dirName/JermanScalesSearch.json
python3 makeOOFScales.py $minBoundStart $minBoundEnd $minBoundStep $maxBoundStart $maxBoundEnd $maxBoundStep $step > $dirName/OOFScalesSearch.json
python3 makeRuiZhangScales.py $minBoundStart $minBoundEnd $minBoundStep $maxBoundStart $maxBoundEnd $maxBoundStep $step > $dirName/RuiZhangScalesSearch.json
python3 makeRORPOScales.py $minBoundStartRORPO $minBoundEndRORPO $minBoundStepRORPO $factorStart $factorEnd $factorStep $stepRORPOMin $stepRORPOMax > $dirName/RORPOScalesSearch.json


metric=MCC
# sato
Sato_Smin=1.8
Sato_Smax=2.0
# frangi
Frangi_Smin=2.6
Frangi_Smax=2.8
# OOF
OOF_Smin=0.2
OOF_Smax=1.8
# Jerman
Jerman_Smin=1.4
Jerman_Smax=1.6
# Zhang
Zhang_Smin=2.0
Zhang_Smax=2.4
# steps by scale intervals
nbSteps=4

python3 makeFrangiParameters.py 0 1 0.2 $Frangi_Smin $Frangi_Smax $nbSteps > $dirName/FrangiParametersSearch_$metric.json
python3 makeSatoParameters.py 0 3 0.3 $Sato_Smin $Sato_Smax $nbSteps > $dirName/SatoParametersSearch_$metric.json
python3 makeJermanParameters.py 0 1 0.2 $Jerman_Smin $Jerman_Smax $nbSteps > $dirName/JermanParametersSearch_$metric.json
python3 makeOOFParameters.py 0 3 0.5 $OOF_Smin $OOF_Smax $nbSteps > $dirName/OOFParametersSearch_$metric.json
python3 makeRuiZhangParameters.py 0 1 0.2 $Zhang_Smin $Zhang_Smax $nbSteps> $dirName/RuiZhangParametersSearch_$metric.json


metric=Dice
# sato
Sato_Smin=1.8
Sato_Smax=2.0
# frangi
Frangi_Smin=2.6
Frangi_Smax=2.8
# OOF
OOF_Smin=0.2
OOF_Smax=1.4
# Jerman
Jerman_Smin=1.4
Jerman_Smax=1.6
# Zhang
Zhang_Smin=2.0
Zhang_Smax=2.2
# steps by scale intervals
nbSteps=4

python3 makeFrangiParameters.py 0 1 0.2 $Frangi_Smin $Frangi_Smax $nbSteps > $dirName/FrangiParametersSearch_$metric.json
python3 makeSatoParameters.py 0 3 0.3 $Sato_Smin $Sato_Smax $nbSteps > $dirName/SatoParametersSearch_$metric.json
python3 makeJermanParameters.py 0 1 0.2 $Jerman_Smin $Jerman_Smax $nbSteps > $dirName/JermanParametersSearch_$metric.json
python3 makeOOFParameters.py 0 3 0.5 $OOF_Smin $OOF_Smax $nbSteps > $dirName/OOFParametersSearch_$metric.json
python3 makeRuiZhangParameters.py 0 1 0.2 $Zhang_Smin $Zhang_Smax $nbSteps> $dirName/RuiZhangParametersSearch_$metric.json

metric=ROC
# sato
Sato_Smin=1.8
Sato_Smax=3.0
# frangi
Frangi_Smin=2.4
Frangi_Smax=3.0
# OOF
OOF_Smin=1.2
OOF_Smax=3.0
# Jerman
Jerman_Smin=1.6
Jerman_Smax=3.0
# Zhang
Zhang_Smin=1.8
Zhang_Smax=3.0
# steps by scale intervals
nbSteps=4

python3 makeFrangiParameters.py 0 1 0.2 $Frangi_Smin $Frangi_Smax $nbSteps > $dirName/FrangiParametersSearch_$metric.json
python3 makeSatoParameters.py 0 3 0.3 $Sato_Smin $Sato_Smax $nbSteps > $dirName/SatoParametersSearch_$metric.json
python3 makeJermanParameters.py 0 1 0.2 $Jerman_Smin $Jerman_Smax $nbSteps > $dirName/JermanParametersSearch_$metric.json
python3 makeOOFParameters.py 0 3 0.5 $OOF_Smin $OOF_Smax $nbSteps > $dirName/OOFParametersSearch_$metric.json
python3 makeRuiZhangParameters.py 0 1 0.2 $Zhang_Smin $Zhang_Smax $nbSteps> $dirName/RuiZhangParametersSearch_$metric.json
