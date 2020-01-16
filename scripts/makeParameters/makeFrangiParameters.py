import decimal
import sys
import numpy as np
import random

#fixed scale space
sigmaMin = 0.6
sigmaMax = 1.6
nbSigmaSteps = 10
gamma = 10

def drange(x, y, jump):
    count = 0
    if(x>y):
        return []
    if( (y-x/float(jump)) > 10000 ):
        return []
    while x<y:
        yield x
        x+= decimal.Decimal(jump)

def varyingPart(start,end,step):
    print("{") 
    print("""    "Antiga":
    [""")
    for i in list(drange(decimal.Decimal(start),decimal.Decimal(end),step)):
        for j in list(drange(decimal.Decimal(start),decimal.Decimal(end),step)):
            if( i==0 or j==0):
                continue
            st = """
        {
            "Output":\""""+str(i)+"-"+str(j)+""".nii",
            "Arguments":[
            {"sigmaMin":\""""+str(sigmaMin)+"""\"},
            {"sigmaMax":\""""+str(sigmaMax)+"""\"},
            {"nbSigmaSteps":\""""+str(nbSigmaSteps)+"""\"},
            {"alpha":\""""+str(i)+"""\"},
            {"beta":\""""+str(j)+"""\"},
            {"gamma":\""""+str(gamma)+"""\"}
            ]
        },"""
            print(st)

# -----------------------------
# -----------------------------

start = float(sys.argv[1])
end = float(sys.argv[2])
step = sys.argv[3]

varyingPart(start,end,step)

st = """
	{
	    "Output":\""""+str(end)+"-"+str(end)+""".nii",
	    "Arguments":[
		{"sigmaMin":\""""+str(sigmaMin)+"""\"},
		{"sigmaMax":\""""+str(sigmaMax)+"""\"},
		{"nbSigmaSteps":\""""+str(nbSigmaSteps)+"""\"},
		{"alpha":\""""+str(end)+"""\"},
		{"beta":\""""+str(end)+"""\"},
		{"gamma":\""""+str(gamma)+"""\"}
	    ]
	}"""
print(st)
print(" ]")
print("}")
