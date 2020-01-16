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

def scaleSpaceSingleScale(start,end,step):
    st = """
	    "Output":\""""+str(start)+"-"+str(end)+"-"+str(step)+""".nii\",
	    "Arguments":[
		{"sigmaMin":\""""+str(start)+"""\"},
		{"sigmaMax":\""""+str(end)+"""\"},
		{"nbSigmaSteps":\""""+str(step)+"""\"},
	        {"sigma":"0.2"}"""
    print(st)


minBoundaryStart = decimal.Decimal(sys.argv[1])
maxBoundaryStart = decimal.Decimal(sys.argv[2])
stepBoundaryStart = decimal.Decimal(sys.argv[3])

minBoundaryEnd = decimal.Decimal(sys.argv[4])
maxBoundaryEnd = decimal.Decimal(sys.argv[5])
stepBoundaryEnd = decimal.Decimal(sys.argv[6])

step = decimal.Decimal(sys.argv[7])


decimal.getcontext().prec = 3


name = "OOF"
print("{")
print("""    \""""+str(name)+"""\":
     [""")

for i in drange(minBoundaryStart,maxBoundaryStart,stepBoundaryStart) :
    if(i == 0):
        continue
    for j in drange(minBoundaryEnd,maxBoundaryEnd,stepBoundaryEnd) : 
        if(j <= i):
            continue
        print("\t{",end="") 
        scaleSpaceSingleScale(i,j,step)
        if( maxBoundaryEnd == maxBoundaryStart ):
            if( i == (maxBoundaryStart-stepBoundaryStart)  and j == maxBoundaryEnd ):
                print("\t\t]\n\t}")
            else:
                print("\t\t]\n\t},")
        else:
            if( i==maxBoundaryStart  and j == maxBoundaryEnd ):
                print("\t\t]\n\t}")
            else:
                print("\t\t]\n\t},")
            
print("    ]")
print("}")
