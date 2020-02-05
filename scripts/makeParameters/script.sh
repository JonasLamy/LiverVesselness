# min scale pace bound
minBoundStart=0
minBoundEnd=3
minBoundStep=0.2
# max scale space bound
maxBoundStart=1
maxBoundEnd=3
maxBoundStep=0.2
# nbScales by interval [minBound;maxBound]
step=4

minBoundStartRORPO=20
minBoundEndRORPO=120
minBoundStepRORPO=20
factorStart=1.2
factorEnd=1.2
factorStep=1.2
stepRORPO=2

#python3 makeFrangiScales.py $minBoundStart $minBoundEnd $minBoundStep $maxBoundStart $maxBoundEnd $maxBoundStep $step > Psearch/FrangiScalesSearch.json
#python3 makeMeijeringScales.py $minBoundStart $minBoundEnd $minBoundStep $maxBoundStart $maxBoundEnd $maxBoundStep $step > Psearch/MeijeringScalesSearch.json
#python3 makeSatoScales.py $minBoundStart $minBoundEnd $minBoundStep $maxBoundStart $maxBoundEnd $maxBoundStep $step > Psearch/SatoScalesSearch.json
#python3 makeJermanScales.py $minBoundStart $minBoundEnd $minBoundStep $maxBoundStart $maxBoundEnd $maxBoundStep $step > Psearch/JermanScalesSearch.json
#python3 makeOOFScales.py $minBoundStart $minBoundEnd $minBoundStep $maxBoundStart $maxBoundEnd $maxBoundStep $step > Psearch/OOFScalesSearch.json
#python3 makeRuiZhangScales.py $minBoundStart $minBoundEnd $minBoundStep $maxBoundStart $maxBoundEnd $maxBoundStep $step > Psearch/RuiZhangScalesSearch.json
#python3 makeRORPOScales.py $minBoundStartRORPO $minBoundEndRORPO $minBoundStepRORPO $factorStart $factorEnd $factorStep $stepRORPO > Psearch/RORPOScalesSearch.json


metric=ROC
# sato
Sato_Smin=1.4
Sato_Smax=3.0
# frangi
Frangi_Smin=2.2
Frangi_Smax=3.0
# OOF
OOF_Smin=1.4
OOF_Smax=3.0
# Meijering
#Meijering_Smin=0
#Meijering_Smax=1
# Jerman
Jerman_Smin=1.2
Jerman_Smax=3.0
# Zhang
Zhang_Smin=1.8
Zhang_Smax=3.0
# steps by scale intervals
nbSteps=4

python3 makeFrangiParameters.py 0 1 0.2 $Frangi_Smin $Frangi_Smax $nbSteps > Psearch/FrangiParametersSearch_$metric.json
#python3 makeMeijeringParameters.py -2 2 0.2 $Meijering_Smin $Meijering_Smax $nbSteps > Psearch/MeijeringParametersSearch_$metric.json
python3 makeSatoParameters.py 0 3 0.3 $Sato_Smin $Sato_Smax $nbSteps > Psearch/SatoParametersSearch_$metric.json
python3 makeJermanParameters.py 0 1 0.2 $Jerman_Smin $Jerman_Smax $nbSteps > Psearch/JermanParametersSearch_$metric.json
python3 makeOOFParameters.py 0 5 0.5 $OOF_Smin $OOF_Smax $nbSteps > Psearch/OOFParametersSearch_$metric.json
python3 makeRuiZhangParameters.py 0 1 0.2 $Zhang_Smin $Zhang_Smax $nbSteps> Psearch/RuiZhangParametersSearch_$metric.json
