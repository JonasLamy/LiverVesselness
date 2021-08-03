"""
Methods.py

Format algorithm parameters to Json item.

To add a new vesselness you should create a class outputting parameters in the following format:
"MethodName":
[
 {
  "Output":"OutputPath",
  "Arguments":[
      {"parameter1":"value"},
      {"parameter2":"value"},
      {"parameter3":"value"}
      ]
 }
]

See NewVesselness class example

"""
from methodsParameters import *

class NewVesselness:
    def __init__(self,output,param1,param2,param3):
        self.output = output
        self.param1 = param1
        self.param2 = param2
        self.param3 = param3
    def __str__(self):
        st = ("""  {\n"""
              """   \"Output\":\""""+self.output+"""\",\n"""
              """   \"Arguments\":[\n"""
              """\t{"sigmaMin":\""""+"{:.2f}".format(self.param1)+"""\"},\n"""
              """\t{"sigmaMax":\""""+"{:.2f}".format(self.param2)+"""\"},\n"""
              """\t{"nbSigmaSteps":\""""+str(self.param3)+"""\"},\n""")


        return st


class Vesselness:
    def __init__(self,output):
        self.output = output
    def __str__(self):
        return """   \"Output\":\""""+self.output+"""\",\n"""

class Hessian(Vesselness):
    def __init__(self,output,sigmaMin,sigmaMax,nbSigmaSteps):
        super().__init__(output)
        self.sigmaMin = sigmaMin
        self.sigmaMax = sigmaMax
        self.nbSigmaSteps = nbSigmaSteps
    def __str__(self):
        superSt = super().__str__()
        st = (superSt +
              """   \"Arguments\":[\n"""
              """\t{"sigmaMin":\""""+"{:.2f}".format(self.sigmaMin)+"""\"},\n"""
              """\t{"sigmaMax":\""""+"{:.2f}".format(self.sigmaMax)+"""\"},\n"""
              """\t{"nbSigmaSteps":\""""+str(self.nbSigmaSteps)+"""\"},\n""")
        return st

class Antiga(Hessian):
    def __init__(self,output,scaleMin,scaleMax,nbScales,frangiParameters):
        super().__init__(output,scaleMin,scaleMax,nbScales)
        self.frangiParameters = frangiParameters
    def __str__(self):
        superSt = super().__str__()
        st = ("""  {\n"""
              
              + superSt +
              """\t{\"alpha\":\""""+"{:.2f}".format(self.frangiParameters.alpha)+"""\"},\n"""
              """\t{\"beta\":\""""+"{:.2f}".format(self.frangiParameters.beta)+"""\"},\n"""
              """\t{\"gamma\":\""""+"{:.2f}".format(self.frangiParameters.gamma)+"""\"}\n"""
              """  \t]\n"""
              """  }"""
        )
        return st

class Sato(Hessian):
    def __init__(self,output,scaleMin,scaleMax,nbScales,satoParameters):
        super().__init__(output,scaleMin,scaleMax,nbScales)
        self.satoParameters = satoParameters
    def __str__(self):
        superSt = super().__str__()
        st = ("""  {\n"""
              + superSt +
              """\t{\"alpha1\":\""""+"{:.2f}".format(self.satoParameters.alpha1)+"""\"},\n"""
              """\t{\"alpha2\":\""""+"{:.2f}".format(self.satoParameters.alpha2)+"""\"}\n"""
              """  \t]\n"""
              """  }"""
        )
        return st

class Meijering(Hessian):
    def __init__(self,output,scaleMin,scaleMax,nbScales,meijeringParameters):
        super().__init__(output,scaleMin,scaleMax,nbScales)
        self.meijeringParameters = meijeringParameters
    def __str__(self):
        superSt = super().__str__()
        st = ("""  {\n"""
              + superSt +
              """\t{\"alpha\":\""""+"{:.2f}".format(self.meijeringParameters.alpha)+"""\"}\n"""
              """  \t]\n"""
              """  }"""
        )
        return st

class OOF(Hessian):
    def __init__(self,output,scaleMin,scaleMax,nbScales,OOFParameters):
        super().__init__(output,scaleMin,scaleMax,nbScales)
        self.OOFParameters = OOFParameters
    def __str__(self):
        superSt = super().__str__()
        st = ("""  {\n"""
              + superSt +
              """\t{\"sigma\":\""""+"{:.2f}".format(self.OOFParameters.sigma)+"""\"}\n"""
              """  \t]\n"""
              """  }"""
        )
        return st

class Jerman(Hessian):
    def __init__(self,output,scaleMin,scaleMax,nbScales,jermanParameters):
        super().__init__(output,scaleMin,scaleMax,nbScales)
        self.jermanParameters = jermanParameters
    def __str__(self):
        superSt = super().__str__()
        st = ("""  {\n"""
              + superSt +
              """\t{\"tau\":\""""+"{:.2f}".format(self.jermanParameters.tau)+"""\"}\n"""
              """  \t]\n"""
              """  }"""
        )
        return st

class Zhang(Hessian):
    def __init__(self,output,scaleMin,scaleMax,nbScales,zhangParameters):
        super().__init__(output,scaleMin,scaleMax,nbScales)
        self.zhangParameters = zhangParameters
    def __str__(self):
        superSt = super().__str__()
        st = ("""  {\n"""
              + superSt +
              """\t{\"tau\":\""""+"{:.2f}".format(self.zhangParameters.tau)+"""\"}\n"""
              """  \t]\n"""
              """  }"""
        )
        return st

class RORPO(Vesselness):
    def __init__(self,output,scaleMin,factor,nbScales,RORPOParameters):
        super().__init__(output)
        self.scaleMin= scaleMin
        self.factor = factor
        self.nbScales = nbScales
        self.RORPOParameters = RORPOParameters
    def __str__(self):
        superSt = super().__str__()
        
        verbose = ""
        if(self.RORPOParameters.verbose):
            verbose = """\t{\"verbose\":\""""+str("")+"""\"},\n""" #verbose is just --verbose option in the benchmark, no need for value

        st = ("""  {\n"""
              + superSt +
              """   \"Arguments\":[\n"""
              """\t{\"scaleMin\":\""""+str(self.scaleMin)+"""\"},\n"""
              """\t{\"factor\":\""""+"{:.2f}".format(self.factor)+"""\"},\n"""
              """\t{\"nbScales\":\""""+str(self.nbScales)+"""\"},\n"""
              """\t{\"core\":\""""+str(self.RORPOParameters.core)+"""\"},\n"""
              + verbose + 
              """\t{\"dilationSize\":\""""+str(self.RORPOParameters.dilationSize)+"""\"},\n"""
              """\t{\"normalize\":\""""+str("")+"""\"},\n"""
              """  \t]\n"""
              """  }"""
            )
        return st

# ADD custom method here


"""
# testing json output
print("{")
# super classes
#print(str( Vesselness("vesselness.nii") )+",")
#print(str( Hessian("Hessian.nii",1,3,3) )+",")

print(str( Antiga("antiga.nii",1,3,3,FrangiParameters(0.5,0.5,5) ) )+",")
print(str( Sato("sato.nii",1,3,3,SatoParameters(0.5,2)) )+"," )
print(str( Meijering("Meijering.nii",1,3,3,MeijeringParameters(-0.33)) )+"," )
print(str( OOF("OOF.nii",1,3,3,OOFParameters(0.5)) )+"," )
print(str( Jerman("Jerman.nii",1,3,3,JermanParameters(0.75)) )+"," )
print(str( Zhang("Zhang.nii",1,3,3,ZhangParameters(0.75)) )+"," )
print(str( RORPO("RORPO.nii",40,1.5,4,RORPOParameters(3,0,True)) ) )
print("}")

"""
