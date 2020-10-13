from scaleSearch import *
from parameterSearch import *
import os
##################
# Main 
##################

dirPath = "test/"

print("Saving files in directory :",dirPath)
if not os.path.exists(dirPath):
    print("path to directory "+dirPath+" does not exists...aborting")
    exit()

bounds = BoundsSS()
bounds.minBoundStart = 1.0
bounds.minBoundEnd   = 2.0
bounds.minBoundStep  = 0.4

bounds.maxBoundStart = 2.0
bounds.maxBoundEnd   = 3.0
bounds.maxBoundStep  = 0.4

bounds.nbScales      = 3


print("Scale search - Number of parameters sets")
print("--------------------")
# Frangi SS
frangiParams = FrangiParameters(alpha=0.5,beta=0.5,gamma=5)
frangiSS = HessianScaleSearch(bounds,"Antiga",frangiParams)
print(frangiSS, file=open(dirPath+"frangiScaleSearchTest.json","w"))
print("Frangi scale search:",frangiSS.nbParameters)

# Sato SS
satoParams = SatoParameters(alpha1=0.5,alpha2=2)
satoSS = HessianScaleSearch(bounds,"Sato",satoParams)
print(satoSS, file=open(dirPath+"SatoScaleSearchTest.json","w"))
print("Sato scale search:",satoSS.nbParameters)

# Meijering SS
meijeringParams = MeijeringParameters(alpha=-0.33)
meijeringSS = HessianScaleSearch(bounds,"Meijering",meijeringParams)
print(meijeringSS, file=open(dirPath+"MeijeringScaleSearchTest.json","w"))
print("Meijering scale search:",meijeringSS.nbParameters)

# OOF SS
OOFParams = OOFParameters(sigma=0.5)
OOFSS = HessianScaleSearch(bounds,"OOF",OOFParams)
print(OOFSS, file=open(dirPath+"OOFScaleSearchTest.json","w"))
print("OOF scale search:",OOFSS.nbParameters)

# Jerman SS
jermanParams = JermanParameters(tau=0.6)
jermanSS = HessianScaleSearch(bounds,"Jerman",jermanParams)
print(jermanSS, file=open(dirPath+"JermanScaleSearchTest.json","w"))
print("Jerman scale search:",jermanSS.nbParameters)

# Zhang SS
zhangParams = ZhangParameters(tau=0.75)
zhangSS = HessianScaleSearch(bounds,"Zhang",zhangParams)
print(zhangSS, file=open(dirPath+"ZhangScaleSearchTest.json","w"))
print("Zhang scale search:",zhangSS.nbParameters)

rorpoBoundsSS = RORPOBoundsSS()
rorpoBoundsSS.minScaleStart = 10
rorpoBoundsSS.minScaleEnd = 50
rorpoBoundsSS.minScaleStep = 10

rorpoBoundsSS.factorStart = 1.2
rorpoBoundsSS.factorEnd = 1.4
rorpoBoundsSS.factorStep = 0.1

rorpoBoundsSS.nbScalesStart = 1
rorpoBoundsSS.nbScalesEnd = 3
rorpoBoundsSS.nbScalesStep = 1

rorpoParams = RORPOParameters(core=3,dilationSize=0,verbose=True)
rorpoSS = RORPOScaleSearch("RORPO_multiscale_usage",rorpoBoundsSS,rorpoParams)
print(rorpoSS,file=open(dirPath+"RORPOScaleSearchTest.json","w"))
print("RORPO scale search:",rorpoSS.nbParameters)

# Frangi PS
frangiBoundsPS = FrangiBounds()
frangiBoundsPS.alphaMin  = 0.2
frangiBoundsPS.alphaMax  = 1
frangiBoundsPS.alphaStep = 0.2
frangiBoundsPS.betaMin   = 0.2
frangiBoundsPS.betaMax   = 1
frangiBoundsPS.betaStep  = 0.2

frangiPS = FrangiParametersSearch("Antiga",frangiBoundsPS,sigmaMin=1,sigmaMax=3,sigmaSteps=3)
print(frangiPS,file=open(dirPath+"FrangiParameterSearch.json","w"))
print("frangi parameters search:",frangiPS.nbParameters)

# Sato PS
satoBoundsPS = SatoBounds()
satoBoundsPS.alpha1Min  = 0.1
satoBoundsPS.alpha1Max  = 1
satoBoundsPS.alpha1Step = 0.1
satoBoundsPS.alpha2Min   = 1
satoBoundsPS.alpha2Max   = 2
satoBoundsPS.alpha2Step  = 0.1

satoPS = SatoParametersSearch("Sato",satoBoundsPS,sigmaMin=1,sigmaMax=3,sigmaSteps=3)
print(satoPS,file=open(dirPath+"SatoParameterSearch.json","w"))
print("sato parameters search:",satoPS.nbParameters)


# Meijering PS
meijeringBoundsPS = MeijeringBounds()
meijeringBoundsPS.alphaMin  = 0.1
meijeringBoundsPS.alphaMax  = 1
meijeringBoundsPS.alphaStep = 0.3

meijeringPS = MeijeringParametersSearch("Meijering",meijeringBoundsPS,sigmaMin=1,sigmaMax=3,sigmaSteps=3)
print(meijeringPS,file=open(dirPath+"MeijeringParameterSearch.json","w"))
print("meijering parameters search:",meijeringPS.nbParameters)

# OOF PS
# careful here OOF sigma is for bluring
# whereas benchmark sigma is scale space (in truth the sigma from scale scaleSpace correspond to sphere radius)
# check implementation for reference
OOFBoundsPS = OOFBounds()
OOFBoundsPS.sigmaMin  = 0.1
OOFBoundsPS.sigmaMax  = 1
OOFBoundsPS.sigmaStep = 0.1

OOFPS = OOFParametersSearch("OOF",OOFBoundsPS,sigmaMin=1,sigmaMax=3,sigmaSteps=3)
print(OOFPS,file=open(dirPath+"OOFParameterSearch.json","w"))
print("OOF parameters search:",OOFPS.nbParameters)

# Jerman PS
jermanBoundsPS = JermanBounds()
jermanBoundsPS.tauMin  = 0.1
jermanBoundsPS.tauMax  = 1
jermanBoundsPS.tauStep = 0.1

jermanPS = JermanParametersSearch("Jerman",jermanBoundsPS,sigmaMin=1,sigmaMax=3,sigmaSteps=3)
print(jermanPS,file=open(dirPath+"JermanParameterSearch.json","w"))
print("Jerman parameters search:",jermanPS.nbParameters)

# Zhang PS
zhangBoundsPS = ZhangBounds()
zhangBoundsPS.tauMin  = 0.1
zhangBoundsPS.tauMax  = 1
zhangBoundsPS.tauStep = 0.1

zhangPS = ZhangParametersSearch("Zhang",zhangBoundsPS,sigmaMin=1,sigmaMax=3,sigmaSteps=3)
print(zhangPS,file=open(dirPath+"ZhangParameterSearch.json","w"))
print("Zhang parameters search:",zhangPS.nbParameters)
