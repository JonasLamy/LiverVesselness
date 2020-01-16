import decimal
import sys
import numpy as np
import random

#fixed scale space
sigmaMin = 2.4
sigmaMax = 2.8
nbSigmaSteps = 4

def drange(x, y, jump):
    while x<y:
        yield x
        x+= decimal.Decimal(jump)

def varyingPart(start,end,step):
    print("{") 
    print("""    "Sato":
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
            {"alpha1":\""""+str(i)+"""\"},
            {"alpha2":\""""+str(j)+"""\"}
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
		{"alpha1":\""""+str(end)+"""\"},
		{"alpha2":\""""+str(end)+"""\"}
	    ]
	}"""
print(st)
print(" ]")
print("}")
