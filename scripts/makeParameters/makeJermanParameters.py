import decimal
import sys
import numpy as np
import random

def drange(x, y, jump):
    count = 0
    if(x>y):
        return []
    if( (y-x/jump) > 10000 ):
        return []
    while x<=y:
        yield x
        x+= decimal.Decimal(jump)

def varyingPart(i,sigmaMin,sigmaMax,nbSigmaSteps):
    st = """
           "Output":\""""+str(i)+""".nii",
           "Arguments":[
           {"sigmaMin":\""""+str(sigmaMin)+"""\"},
           {"sigmaMax":\""""+str(sigmaMax)+"""\"},
           {"nbSigmaSteps":\""""+str(nbSigmaSteps)+"""\"},
           {"tau":\""""+str(i)+"""\"}"""
    print(st)

# -----------------------------
# -----------------------------

start = decimal.Decimal(sys.argv[1])
end = decimal.Decimal(sys.argv[2])
step = decimal.Decimal(sys.argv[3])
#fixed scale space
sigmaMin = decimal.Decimal(sys.argv[4])
sigmaMax = decimal.Decimal(sys.argv[5])
nbSigmaSteps = decimal.Decimal(sys.argv[6])

decimal.getcontext().prec = 3

name = "Jerman"
print("{")
print("""    \""""+str(name)+"""\":
     [""")

for i in drange(start,end,step) :
    if(i == 0):
        continue
    
    print("\t{",end="") 
    varyingPart(i,sigmaMin,sigmaMax,nbSigmaSteps)
    if( i == end):
        print("\t\t]\n\t}")
    else:
        print("\t\t]\n\t},")
print(" ]")
print("}")
