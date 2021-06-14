import numpy as np
import pandas as pd 
import matplotlib.pyplot as plt
import sys 

def gauss(x,a,mu,sigma):
    return a*np.exp(-(x-mu)**2 /(2*sigma**2))

csvFilePath = sys.argv[1]
is_mri = int(sys.argv[2])

frame = pd.read_csv(csvFilePath)

if(is_mri):
    x = np.linspace(0,3000,1000)
else:
    x = np.linspace(-200,300,550)

mean = frame.mean()
print(mean)
for id,patient,liver_a,liver_mu,liver_sigma,vessels_a,vessels_mu,vessels_sigma in frame.itertuples(): 

    # liver
    y = gauss(x,liver_a,liver_mu,liver_sigma)
    
    plt.figure(1)
    plt.plot(x,y,alpha=0.3,dashes=[30, 5, 10, 5])
    # vessels
    y = gauss(x,vessels_a,vessels_mu,vessels_sigma)
    
    plt.figure(2)
    plt.plot(x,y,alpha=0.3,dashes=[30, 5, 10, 5])

plt.figure(1)
plt.title("liver intensity (no vessels), mean gaussian in full line")
plt.xlabel("intensity bins value")
plt.ylabel("number of voxels in bin")
y = gauss(x,mean["liver_a"],mean["liver_mu"],mean["liver_sigma"])
plt.plot(x,y)

plt.savefig("liver.svg")

plt.figure(2)
plt.title("vessels intensity, mean gaussian in full line")
plt.xlabel("intensity bins value")
plt.ylabel("number of voxels in bin")
y = gauss(x,mean["vessels_a"],mean["vessels_mu"],mean["vessels_sigma"])
plt.plot(x,y)

plt.savefig("vessels.svg")

plt.show()