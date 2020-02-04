import decimal
import sys
import numpy as np
import random

#fixed scale space
sigmaMin = 2.4
sigmaMax = 3.1
nbSigmaSteps = 4

def drange(x, y, jump):
    count = 0
    if(x>y):
        return []
    if( (y-x/jump) > 10000 ):
        return []
    while x<=y:
        yield x
        x+= decimal.Decimal(jump)

def varyingPart(i,j):
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
        varyingPart(i,j)
        if( i==end and j==end):
            print("\t\t]\n\t}")
        else:
            print("\t\t]\n\t},")
print(" ]")
print("}")
