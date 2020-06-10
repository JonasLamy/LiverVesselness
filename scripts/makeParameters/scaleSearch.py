from methods import *

class BoundsSS:
    def __init__(self):
        self.minBoundStart = 0
        self.minBoundEnd = 0
        self.minBoundStep = 0

        self.maxBoundStart = 0
        self.maxBoundEnd = 0
        self.maxBoundStep = 0
        # number of scales per interval [minBound,maxBound]
        self.nbScales = 0

class RORPOBoundsSS:
    def __init__(self):
        self.minScaleStart = 0
        self.minScaleEnd = 0
        self.minScaleStep = 0

        self.factorStart = 0
        self.factorEnd = 0
        self.factorStep = 0

        self.nbScalesStart = 0
        self.nbScalesEnd = 0
        self.nbScalesStep = 0
        
class HessianScaleSearch:
    def __init__(self,boundsSS,methodName,methodParameters):
        self.boundsSS = boundsSS
        self.methodName = methodName
        self.methodParameters = methodParameters

        #debug purpose
        self.nbParameters = 0

    def instance(self,sigmaMin,sigmaMax,nbScales,methodName,methodParameters):
        outputName = "{:.2f}".format(sigmaMin) + "-" + "{:.2f}".format(sigmaMax) + "-" + "{:.2f}".format(nbScales) + ".nii"

        if(self.methodName == "Antiga"):
            return Antiga(outputName,sigmaMin,sigmaMax,nbScales,methodParameters)
        if(self.methodName == "Sato"):
            return Sato(outputName,sigmaMin,sigmaMax,nbScales,methodParameters)
        if(self.methodName == "Meijering"):
            return Meijering(outputName,sigmaMin,sigmaMax,nbScales,methodParameters)
        if(self.methodName == "OOF"):
            return OOF(outputName,sigmaMin,sigmaMax,nbScales,methodParameters)
        if(self.methodName == "Jerman"):
            return Jerman(outputName,sigmaMin,sigmaMax,nbScales,methodParameters)
        if(self.methodName == "Zhang"):
            return Zhang(outputName,sigmaMin,sigmaMax,nbScales,methodParameters)
        # ADD other instances here.............
        
    def parametersSet(self):
        # Warning check floating point consistency....
        # if filtering parameter sets is needed, this is where it should happen
        parametersSets = []
        
        minB = self.boundsSS.minBoundStart
        while(minB <= self.boundsSS.minBoundEnd):
            maxB = self.boundsSS.maxBoundStart
            while(maxB <= self.boundsSS.maxBoundEnd):
                parameterSet = self.instance(minB,maxB,self.boundsSS.nbScales,self.methodName,self.methodParameters)
                
                parametersSets.append(parameterSet)
                maxB += self.boundsSS.maxBoundStep
            minB += self.boundsSS.minBoundStep
            
        return parametersSets
           
    def __str__(self):
        paramSets = self.parametersSet()
        self.nbParameters = len(paramSets)
        
        paramString = ""
        for i in range(0,len(paramSets)-1):
            paramString += str(paramSets[i]) + ",\n"
        paramString += str(paramSets[-1])
        
        st=("{\n"
            " \"" + self.methodName + "\":\n"
            " [\n"
            + paramString +
            " \n ]\n"
            "}"
            )
        return st


class RORPOScaleSearch:
    def __init__(self,methodName,boundsSS,methodParameters):
        self.boundsSS = boundsSS
        self.methodParameters = methodParameters
        self.methodName = methodName
        #debug purpose
        self.nbParameters = 0

    def instance(self,scaleMin,factor,nbScales,methodParameters):
        outputName = str(scaleMin) + "-" + "{:.2f}".format(factor) + "-" + str(nbScales) + ".nii"

        return RORPO(outputName,scaleMin,factor,nbScales,methodParameters)
    
    def parametersSet(self):
        # Warning check floating point consistency....
        # if filtering parameter sets is needed, this is where it should happen
        parametersSets = []
        minS = self.boundsSS.minScaleStart
        while(minS <= self.boundsSS.minScaleEnd):
            factor = self.boundsSS.factorStart
            while(factor <= self.boundsSS.factorEnd):
                nbScales = self.boundsSS.nbScalesStart
                while(nbScales <= self.boundsSS.nbScalesEnd):
                    parameterSet = self.instance(minS,factor,nbScales,self.methodParameters)
                    parametersSets.append(parameterSet)

                    nbScales += self.boundsSS.nbScalesStep
                factor += self.boundsSS.factorStep
            minS += self.boundsSS.minScaleStep
            
        return parametersSets
            
    def __str__(self):
        paramSets = self.parametersSet()
        self.nbParameters = len(paramSets)
        print(self.nbParameters)
        paramString = ""
        for i in range(0,len(paramSets)-1):
            paramString += str(paramSets[i]) + ",\n"
        paramString += str(paramSets[-1])
        
        st=("{\n"
            " \"" + str(self.methodName) + "\":\n"
            " [\n"
            + paramString +
            " \n ]\n"
            "}"
            )
        return st
 
