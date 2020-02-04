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
            {"core":"3"},
            {"verbose":""}"""
    print(st)

minBoundaryStart = decimal.Decimal(sys.argv[1])
maxBoundaryStart = decimal.Decimal(sys.argv[2])
stepBoundaryStart = decimal.Decimal(sys.argv[3])

minFactor = decimal.Decimal(sys.argv[4])
maxFactor = decimal.Decimal(sys.argv[5])
stepFactor = decimal.Decimal(sys.argv[6])

step = decimal.Decimal(sys.argv[7])

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
            print("\t{",end="") 
            scaleSpaceSingleScale(i,j,step)
            
            if( i==maxBoundaryStart  and j == maxFactor ):
                print("\t\t]\n\t}")
            else:
                print("\t\t]\n\t},")
                    
print("    ]")
print("}")
