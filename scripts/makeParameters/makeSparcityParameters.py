from scaleSearch import *
from parameterSearch import *
import os
##################
# Main 
##################

dirPath = "sparcity/"

print("Saving files in directory :",dirPath)
if not os.path.exists(dirPath):
    print("path to directory "+dirPath+" does not exists...aborting")
    exit()

bounds = BoundsSS()
bounds.minBoundStart = 0.1
bounds.minBoundEnd   = 1
bounds.minBoundStep  = 0.1

bounds.maxBoundStart = 1.1
bounds.maxBoundEnd   = 2
bounds.maxBoundStep  = 0.1

bounds.nbScales      = 3


print("Scale search - Number of parameters sets")
print("--------------------")
"""
# Jerman SS
jermanParams = JermanParameters(tau=0.75)
jermanSS = HessianScaleSearch(bounds,"Jerman",jermanParams)
print(jermanSS, file=open(dirPath+"JermanScaleSearchTest.json","w"))
print("Jerman scale search:",jermanSS.nbParameters)
"""

print("ParameterSearch - Number of parameters")
# Jerman PS
jermanBoundsPS = JermanBounds()
jermanBoundsPS.tauMin  = 0.2
jermanBoundsPS.tauMax  = 1
jermanBoundsPS.tauStep = 0.2

jermanPS = JermanParametersSearch("Jerman",jermanBoundsPS,sigmaMin=1,sigmaMax=3,sigmaSteps=3)
print(jermanPS,file=open(dirPath+"JermanParameterSearch.json","w"))
print("Jerman parameters search:",jermanPS.nbParameters)
