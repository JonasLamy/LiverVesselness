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

start1 = decimal.Decimal(sys.argv[1])
end1 = decimal.Decimal(sys.argv[2])
step1 = decimal.Decimal(sys.argv[3])

start2 = decimal.Decimal(sys.argv[4])
end2 = decimal.Decimal(sys.argv[5])
step2 = decimal.Decimal(sys.argv[6])

#fixed scale space
sigmaMin = decimal.Decimal(sys.argv[7])
sigmaMax = decimal.Decimal(sys.argv[8])
nbSigmaSteps = decimal.Decimal(sys.argv[9])


decimal.getcontext().prec = 3

name = "Sato"
print("{")
print("""    \""""+str(name)+"""\":
     [""")

for i in drange(start1,end1,step1) :
    if(i == 0):
        continue
    for j in drange(start2,end2,step2) : 
        if(j == 0):
            continue
        if(j<=i): # Sato condition
            continue
        print("\t{",end="") 
        varyingPart(i,j,sigmaMin,sigmaMax,nbSigmaSteps)
        if( i==(end1) and j==end2):
            print("\t\t]\n\t}")
        else:
            print("\t\t]\n\t},")
print(" ]")
print("}")
