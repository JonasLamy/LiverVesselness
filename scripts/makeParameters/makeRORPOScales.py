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

def isOK(i,j,step,minImageSize):

    #print("path length:",i,"for step=0")

    for k in range(1,int(step)):
        #print("path length:",i*j**k,"for step="+str(k))
        if(int(i * j**k) >= minImageSize):
            #print("skipping",i,j,step,"because:"+str(int(i * j**k))+" for step= "+str(k))
            return False
    if(i*j**step < 40):
        return False
        
    return True
        
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

minBoundaryStart = decimal.Decimal(sys.argv[1])
maxBoundaryStart = decimal.Decimal(sys.argv[2])
stepBoundaryStart = decimal.Decimal(sys.argv[3])

minFactor = decimal.Decimal(sys.argv[4])
maxFactor = decimal.Decimal(sys.argv[5])
stepFactor = decimal.Decimal(sys.argv[6])

stepMin = int(sys.argv[7])
stepMax = int(sys.argv[8])

minImageSize = 101

decimal.getcontext().prec = 3

name = "RORPO_multiscale_usage"
print("{")
print("""    \""""+str(name)+"""\":
     [""")

for i in drange(minBoundaryStart,maxBoundaryStart,stepBoundaryStart) :
    if(i == 0):
        continue
    if(minFactor == maxFactor):
        print("\t{",end="") 
        scaleSpaceSingleScale(i,minFactor,step)
        if( i==maxBoundaryStart):
            print("\t\t]\n\t}")
        else:
            print("\t\t]\n\t},")
    else:
        
        for j in drange(minFactor,maxFactor,stepFactor) :
            for step in range(stepMin,stepMax+1):
                # checking that the paths are not over
                # the desired threshold
                
                if not isOK(i,j,step,minImageSize):
                    break
                #print("scales:",i,j,step+1,int(i * j**k))
                
                print("\t{",end="") 
                scaleSpaceSingleScale(i,j,step)
                
                if( i==maxBoundaryStart  and j == maxFactor ):
                    print("\t\t]\n\t}")
                else:
                    print("\t\t]\n\t},")    
print("    ]")
print("}")
