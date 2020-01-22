

# min scale pace bound
$minBoundStart=1
$minBoundEnd=4
$minBoundStep=1
# max scale space bound
$maxBoundStart=1
$maxBoundEnd=5
$maxBoundStep=1
# nbScales by interval [minBound;maxBound]
$step=3

python3 makeParameters/makeFrangiScales.py $minBoundStart $minBoundEnd $minBoundStep $maxBoundStart $maxBoundEnd $maxBoundStep $step > FrangiScales.json
#python3 makeParameters/makeMeijeringScales.py 1 5 1 1 5 1 3 > MeijeringScales.json
#python3 makeParameters/makeRORPOScales.py 10 50 10 1.1 1.4 3 > RORPOScales.json
#python3 makeParameters/makeSatoScales.py 1 5 1 1 5 1 3 > SatoScales.jso n
#python3 makeParameters/makeJermanScales.py 1 5 1 1 5 1 3 > Jerman.json
#python3 makeParameters/makeOOFScales.py 1 5 1 1 5 1 3 > OOFScales.json
#python3 makeParameters/makeRuiZhangScales.py 1 5 1 1 5 1 3 > RuiZhangScales.json
