import decimal
import sys
import numpy as np
import random

#fixed scale space
sigmaMin = 1
sigmaMax = 1.4
nbSigmaSteps = 4

def drange(x, y, jump):
    while x<y:
        yield x
        x+= decimal.Decimal(jump)

def varyingSigma(start,end,step):
    print("{") 
    print("""    "OOF":
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
		{"sigma":\""""+str(i)+"""\"}
	    ]
	},"""
        print(st)


# -----------------------------
# -----------------------------

start = float(sys.argv[1])
end = float(sys.argv[2])
step = sys.argv[3]

varyingSigma(start,end,step)

st = """
	{
	    "Output":\""""+str(end)+""".nii",
	    "Arguments":[
		{"sigmaMin":\""""+str(sigmaMin)+"""\"},
		{"sigmaMax":\""""+str(sigmaMax)+"""\"},
		{"nbSigmaSteps":\""""+str(nbSigmaSteps)+"""\"},
		{"sigma":\""""+str(end)+"""\"}
	    ]
	}"""
print(st)
print(" ]")
print("}")
