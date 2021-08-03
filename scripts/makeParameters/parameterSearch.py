from methods import *

class FrangiBounds:
    def __init__(self,alphaMin=0.1,alphaMax=1,alphaStep=0.1,betaMin=0,betaMax=1,betaStep=0.1,gammaMin=5,gammaMax=10,gammaStep=5):
        self.alphaMin = alphaMin
        self.alphaMax = alphaMax
        self.alphaStep = alphaStep

        self.betaMin = betaMin
        self.betaMax = betaMax
        self.betaStep = betaStep

        self.gammaMin = gammaMin
        self.gammaMax = gammaMax
        self.gammaStep = gammaStep

class SatoBounds:
    def __init__(self,alpha1Min=0.1,alpha1Max=1,alpha1Step=0.1,alpha2Min=2,alpha2Max=3,alpha2Step=0.1):
        self.alpha1Min = alpha1Min
        self.alpha1Max = alpha1Max
        self.alpha1Step = alpha1Step

        self.alpha2Min = alpha2Min
        self.alpha2Max = alpha2Max
        self.alpha2Step = alpha2Step

class MeijeringBounds:
    def __init__(self,alphaMin=0.1,alphaMax=1,alphaStep=0.1):
        self.alphaMin = alphaMin
        self.alphaMax = alphaMax
        self.alphaStep = alphaStep

class OOFBounds:
    def __init__(self,sigmaMin=0.5,sigmaMax=3,sigmaStep=0.5):
        self.sigmaMin = sigmaMin
        self.sigmaMax = sigmaMax
        self.sigmaStep = sigmaStep

class JermanBounds:
    def __init__(self,tauMin=0.1,tauMax=1,tauStep=0.1):
        self.tauMin = tauMin
        self.tauMax = tauMax
        self.tauStep = tauStep

class ZhangBounds:
    def __init__(self,tauMin=0.1,tauMax=1,tauStep=0.1):
        self.tauMin = tauMin
        self.tauMax = tauMax
        self.tauStep = tauStep
    
class FrangiParametersSearch:
    def __init__(self,methodName,boundsPS,sigmaMin,sigmaMax,sigmaSteps):
        self.sigmaMin = sigmaMin
        self.sigmaMax = sigmaMax
        self.sigmaSteps = sigmaSteps
        self.methodName = methodName
        self.boundsPS = boundsPS
        
    def parametersSet(self):
        # Warning check floating point consistency....
        # if filtering parameter sets is needed, this is where it should happen
        parametersSets = []
        alpha = self.boundsPS.alphaMin
        while(alpha <= self.boundsPS.alphaMax):
            beta = self.boundsPS.betaMin
            while(beta <= self.boundsPS.betaMax):
                gamma = self.boundsPS.gammaMin 
                while(gamma <= self.boundsPS.gammaMax):
                    name = "{:.2f}".format(alpha)+"-"+"{:.2f}".format(beta)+".nii"
                
                    paramSet = self.instance(name,self.sigmaMin,self.sigmaMax,self.sigmaSteps,FrangiParameters(alpha,beta,gamma) )

                    parametersSets.append( paramSet )
                
                    gamma += self.boundsPS.gammaStep
                beta += self.boundsPS.betaStep
            alpha += self.boundsPS.alphaStep

        return parametersSets

    def instance(self,name,sigmaMin,sigmaMax,sigmaSteps,frangiParameters):
        return Antiga(name,sigmaMin,sigmaMax,sigmaSteps,frangiParameters)
    
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

class SatoParametersSearch:
    def __init__(self,methodName,boundsPS,sigmaMin,sigmaMax,sigmaSteps):
        self.sigmaMin = sigmaMin
        self.sigmaMax = sigmaMax
        self.sigmaSteps = sigmaSteps
        self.methodName = methodName
        self.boundsPS = boundsPS
        
    def parametersSet(self):
        # Warning check floating point consistency....
        # if filtering parameter sets is needed, this is where it should happen
        parametersSets = []
        alpha1 = self.boundsPS.alpha1Min
        while(alpha1 <= self.boundsPS.alpha1Max):
            alpha2 = self.boundsPS.alpha2Min
            while(alpha2 <= self.boundsPS.alpha2Max):
                
                if( alpha1 < alpha2 ): # Sato condition
                    name = "{:.2f}".format(alpha1)+"-"+"{:.2f}".format(alpha2)+".nii"
                    paramSet = self.instance(name,self.sigmaMin,self.sigmaMax,self.sigmaSteps,SatoParameters(alpha1,alpha2) )
                    parametersSets.append( paramSet )
                
                alpha2 += self.boundsPS.alpha2Step
            alpha1 += self.boundsPS.alpha1Step

        return parametersSets

    def instance(self,name,sigmaMin,sigmaMax,sigmaSteps,satoParameters):
        return Sato(name,sigmaMin,sigmaMax,sigmaSteps,satoParameters)
    
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

class MeijeringParametersSearch:
    def __init__(self,methodName,boundsPS,sigmaMin,sigmaMax,sigmaSteps):
        self.sigmaMin = sigmaMin
        self.sigmaMax = sigmaMax
        self.sigmaSteps = sigmaSteps
        self.methodName = methodName
        self.boundsPS = boundsPS
        
    def parametersSet(self):
        # Warning check floating point consistency....
        # if filtering parameter sets is needed, this is where it should happen
        parametersSets = []
        alpha = self.boundsPS.alphaMin
        while(alpha <= self.boundsPS.alphaMax):
                    
            name = "{:.2f}".format(alpha)+".nii"
            paramSet = self.instance(name,self.sigmaMin,self.sigmaMax,self.sigmaSteps,MeijeringParameters(alpha) )
            parametersSets.append( paramSet )
            
            alpha += self.boundsPS.alphaStep

        return parametersSets

    def instance(self,name,sigmaMin,sigmaMax,sigmaSteps,satoParameters):
        return Meijering(name,sigmaMin,sigmaMax,sigmaSteps,satoParameters)
    
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

class OOFParametersSearch:
    def __init__(self,methodName,boundsPS,sigmaMin,sigmaMax,sigmaSteps):
        self.sigmaMin = sigmaMin
        self.sigmaMax = sigmaMax
        self.sigmaSteps = sigmaSteps
        self.methodName = methodName
        self.boundsPS = boundsPS
        
    def parametersSet(self):
        # Warning check floating point consistency....
        # if filtering parameter sets is needed, this is where it should happen
        parametersSets = []
        sigma = self.boundsPS.sigmaMin
        while(sigma <= self.boundsPS.sigmaMax):
                    
            name = "{:.2f}".format(sigma)+".nii"
            paramSet = self.instance(name,self.sigmaMin,self.sigmaMax,self.sigmaSteps,OOFParameters(sigma) )
            parametersSets.append( paramSet )
            
            sigma += self.boundsPS.sigmaStep

        return parametersSets

    def instance(self,name,sigmaMin,sigmaMax,sigmaSteps,satoParameters):
        return OOF(name,sigmaMin,sigmaMax,sigmaSteps,satoParameters)
    
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

class JermanParametersSearch:
    def __init__(self,methodName,boundsPS,sigmaMin,sigmaMax,sigmaSteps):
        self.sigmaMin = sigmaMin
        self.sigmaMax = sigmaMax
        self.sigmaSteps = sigmaSteps
        self.methodName = methodName
        self.boundsPS = boundsPS
        
    def parametersSet(self):
        # Warning check floating point consistency....
        # if filtering parameter sets is needed, this is where it should happen
        parametersSets = []
        tau = self.boundsPS.tauMin
        while(tau <= self.boundsPS.tauMax):
                    
            name = "{:.2f}".format(tau)+".nii"
            paramSet = self.instance(name,self.sigmaMin,self.sigmaMax,self.sigmaSteps,JermanParameters(tau) )
            parametersSets.append( paramSet )
            
            tau += self.boundsPS.tauStep

        return parametersSets

    def instance(self,name,sigmaMin,sigmaMax,sigmaSteps,jermanParameters):
        return Jerman(name,sigmaMin,sigmaMax,sigmaSteps,jermanParameters)
    
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

class ZhangParametersSearch:
    def __init__(self,methodName,boundsPS,sigmaMin,sigmaMax,sigmaSteps):
        self.sigmaMin = sigmaMin
        self.sigmaMax = sigmaMax
        self.sigmaSteps = sigmaSteps
        self.methodName = methodName
        self.boundsPS = boundsPS
        
    def parametersSet(self):
        # Warning check floating point consistency....
        # if filtering parameter sets is needed, this is where it should happen
        parametersSets = []
        tau = self.boundsPS.tauMin
        while(tau <= self.boundsPS.tauMax):
                    
            name = "{:.2f}".format(tau)+".nii"
            paramSet = self.instance(name,self.sigmaMin,self.sigmaMax,self.sigmaSteps,ZhangParameters(tau) )
            parametersSets.append( paramSet )
            
            tau += self.boundsPS.tauStep

        return parametersSets

    def instance(self,name,sigmaMin,sigmaMax,sigmaSteps,zhangParameters):
        return Zhang(name,sigmaMin,sigmaMax,sigmaSteps,zhangParameters)
    
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

