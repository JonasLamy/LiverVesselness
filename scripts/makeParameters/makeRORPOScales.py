import decimal
import sys
import numpy as np
import random

def drange(x, y, jump):
    while x<=y:
        yield x
        if(x < 0 or y < 0):
            return []
        x+= jump
        
def scaleSpaceSingleScale(minBoundary,factor,step):
    st = """
	    "Output":\""""+str(minBoundary)+"-"+str(factor)+"-"+str(step)+""".nii\",
	    "Arguments":[
            {"scaleMin":\""""+str(minBoundary)+"""\"},
            {"factor":\""""+str(factor)+"""\"},
            {"nbScales":\""""+str(step)+"""\"},
            {"core":"4"},
            {"dilationSize":"0"},
            {"verbose":""}"""
    print(st)

minBoundaryStart = int(sys.argv[1])
maxBoundaryStart = int(sys.argv[2])
stepBoundaryStart = int(sys.argv[3])

minFactor = int( float(sys.argv[4]) * 10)
maxFactor = int( float(sys.argv[5]) * 10)
stepFactor = int( float(sys.argv[6]) * 10)

stepMin = int(sys.argv[7])
stepMax = int(sys.argv[8])

pathMin = int(sys.argv[9])
pathMax = int(sys.argv[10])

minImageSize = 101

decimal.getcontext().prec = 3

name = "RORPO_multiscale_usage"
print("{")
print("""    \""""+str(name)+"""\":
     [""")
nbParam=0

#print(minBoundaryStart,maxBoundaryStart,stepBoundaryStart)
#print(minFactor,maxFactor,stepFactor)
#print(stepMin,stepMax)
for s in range(minBoundaryStart,maxBoundaryStart,stepBoundaryStart):
    for f in range(minFactor,maxFactor,stepFactor):
        for n in range(stepMin,stepMax):

            vectScales=[]
            for i in range(n):
                vectScales +=[round(s*((f/10)**i))]
            vectScales = np.array(vectScales)
            if np.amax(vectScales) < pathMax and np.amax(vectScales) > pathMin:
                nbParam += 1
            
                print("\t{",end="") 
                scaleSpaceSingleScale(s,f/10,n)
            
                if( i==maxBoundaryStart  and j == maxFactor ):
                    print("\t\t]\n\t}")
                else:
                    print("\t\t]\n\t},")    
print("    ]")
print("}")
