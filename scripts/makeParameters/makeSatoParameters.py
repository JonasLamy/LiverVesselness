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

def varyingPart(i,j,sigmaMin,sigmaMax,nbSigmaSteps):
    st = """
           "Output":\""""+str(i)+"-"+str(j)+""".nii",
           "Arguments":[
           {"sigmaMin":\""""+str(sigmaMin)+"""\"},
           {"sigmaMax":\""""+str(sigmaMax)+"""\"},
           {"nbSigmaSteps":\""""+str(nbSigmaSteps)+"""\"},
           {"alpha1":\""""+str(i)+"""\"},
           {"alpha2":\""""+str(j)+"""\"}"""
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

name = "Sato"
print("{")
print("""    \""""+str(name)+"""\":
     [""")

for i in drange(start,end,step) :
    if(i == 0):
        continue
    for j in drange(start,end,step) : 
        if(j == 0):
            continue
        if(j<=i): # Sato condition
            continue
        print("\t{",end="") 
        varyingPart(i,j,sigmaMin,sigmaMax,nbSigmaSteps)
        if( i==(end-step) and j==end):
            print("\t\t]\n\t}")
        else:
            print("\t\t]\n\t},")
print(" ]")
print("}")
