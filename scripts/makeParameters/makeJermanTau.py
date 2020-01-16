import decimal
import sys
import numpy as np
import random

#fixed scale space
sigmaMin = 0.5
sigmaMax = 0.7
nbSigmaSteps = 3

def drange(x, y, jump):
    while x<y:
        yield x
        x+= decimal.Decimal(jump)

def varyingTau(start,end,step):
    print("{") 
    print("""    "Jerman":
    [""")
    for i in list(drange(decimal.Decimal(start),decimal.Decimal(end),step)):
        if(i==0):
            continue
        st = """
    {
	    "Output":\""""+str(i)+""".nii",
	    "Arguments":[
		{"sigmaMin":\""""+str(sigmaMin)+"""\"},
		{"sigmaMax":\""""+str(sigmaMax)+"""\"},
		{"nbSigmaSteps":\""""+str(nbSigmaSteps)+"""\"},
		{"tau":\""""+str(i)+"""\"}
	    ]
	},"""
        print(st)


# -----------------------------
# -----------------------------

start = float(sys.argv[1])
end = float(sys.argv[2])
step = sys.argv[3]

varyingTau(start,end,step)

st = """
	{
	    "Output":\""""+str(end)+""".nii",
	    "Arguments":[
		{"sigmaMin":\""""+str(sigmaMin)+"""\"},
		{"sigmaMax":\""""+str(sigmaMax)+"""\"},
		{"nbSigmaSteps":\""""+str(nbSigmaSteps)+"""\"},
		{"tau":\""""+str(end)+"""\"}
	    ]
	}"""
print(st)
print(" ]")
print("}")
