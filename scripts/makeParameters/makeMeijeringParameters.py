import decimal
import sys
import numpy as np
import random

#fixed scale space
sigmaMin = 0.6
sigmaMax = 1.6
nbSigmaSteps = 10

def drange(x, y, jump):
    count = 0
    if(x>y):
        return []
    if( (y-x/jump) > 10000 ):
        return []
    while x<=y:
        yield x
        x+= decimal.Decimal(jump)

def varyingPart(i):
    st = """
           "Output":\""""+str(i)+""".nii",
           "Arguments":[
           {"sigmaMin":\""""+str(sigmaMin)+"""\"},
           {"sigmaMax":\""""+str(sigmaMax)+"""\"},
           {"nbSigmaSteps":\""""+str(nbSigmaSteps)+"""\"},
           {"alpha":\""""+str(i)+"""\"}"""
    print(st)

# -----------------------------
# -----------------------------

start = decimal.Decimal(sys.argv[1])
end = decimal.Decimal(sys.argv[2])
step = decimal.Decimal(sys.argv[3])

decimal.getcontext().prec = 3

name = "Meijering"
print("{")
print("""    \""""+str(name)+"""\":
     [""")

for i in drange(start,end,step) :
    if(i == 0):
        continue
    
    print("\t{",end="") 
    varyingPart(i)
    if( i == end):
        print("\t\t]\n\t}")
    else:
        print("\t\t]\n\t},")
print(" ]")
print("}")
