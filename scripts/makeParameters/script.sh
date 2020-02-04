# min scale pace bound
minBoundStart=0
minBoundEnd=3.3
minBoundStep=0.3
# max scale space bound
maxBoundStart=1
maxBoundEnd=3.3
maxBoundStep=0.3
# nbScales by interval [minBound;maxBound]
step=4

minBoundStartRORPO=80
minBoundEndRORPO=140
minBoundStepRORPO=10
factorStart=1.2
factorEnd=1.2
factorStep=1.2
stepRORPO=2

#python3 makeFrangiScales.py $minBoundStart $minBoundEnd $minBoundStep $maxBoundStart $maxBoundEnd $maxBoundStep $step > FrangiScalesSearch.json
#python3 makeMeijeringScales.py $minBoundStart $minBoundEnd $minBoundStep $maxBoundStart $maxBoundEnd $maxBoundStep $step > MeijeringScalesSearch.json
#python3 makeSatoScales.py $minBoundStart $minBoundEnd $minBoundStep $maxBoundStart $maxBoundEnd $maxBoundStep $step > SatoScalesSearch.json
#python3 makeJermanScales.py $minBoundStart $minBoundEnd $minBoundStep $maxBoundStart $maxBoundEnd $maxBoundStep $step > JermanScalesSearch.json
#python3 makeOOFScales.py $minBoundStart $minBoundEnd $minBoundStep $maxBoundStart $maxBoundEnd $maxBoundStep $step > OOFScalesSearch.json
#python3 makeRuiZhangScales.py $minBoundStart $minBoundEnd $minBoundStep $maxBoundStart $maxBoundEnd $maxBoundStep $step > RuiZhangScalesSearch.json
#python3 makeRORPOScales.py $minBoundStartRORPO $minBoundEndRORPO $minBoundStepRORPO $factorStart $factorEnd $factorStep $stepRORPO > RORPOScalesSearch.json

python3 makeFrangiParameters.py 0 1 0.2 > Psearch/FrangiParametersSearch.json
python3 makeMeijeringParameters.py -2 2 0.2 > Psearch/MeijeringParametersSearch.json
python3 makeSatoParameters.py 0 3 0.3 > Psearch/SatoParametersSearch.json
python3 makeJermanParameters.py 0 1 0.2 > Psearch/JermanParametersSearch.json
python3 makeOOFParameters.py 0 5 0.5 > Psearch/OOFParametersSearch.json
python3 makeRuiZhangParameters.py 0 1 0.2 > Psearch/RuiZhangParametersSearch.json
