import sys
import os
import numpy as np
import itk
from scipy.stats import rice

class Generator:
    def __init__(self):
        self.noiseLevels = [0.0, 2.0, 5.0, 10.0, 15.0, 20.0]

    def gauss3d(self,x=0,y=0,z=0,mx=0,my=0,mz=0,sx=1,sy=1,sz=1):  
        return  1 / (sx*sy*sz * np.sqrt(2. * np.pi ) * np.sqrt(2. * np.pi ) ) * np.exp(-( (x - mx)**2. / (2. * sx**2.) + (y - my)**2. / (2. * sy**2.) + (z - mz)**2. / (2. * sz**2.) ) )  

    
    def bifurcationCoordinatesFile(self,DirPath,file):
        filePath = DirPath + "/" + file
        fileBif = DirPath + "/bifurcations_coordinates.txt"
        
        matlabCall = "matlab -nojvm -nodisplay -nosplash -r"
        scriptCall = " \"load('"+filePath+"');matTotxt(node,'"+fileBif+"');exit; \" "
        os.system(matlabCall + scriptCall)

    def groundTruth(self,DirPath,file):
        #    #now we call the script
        # groundTruth generation
        filePath = DirPath + "/" + file
        fileGT = DirPath + "/gt.nii"
        os.system("./MakeVascuSynthGT "+filePath +" "+fileGT)
        
        print(filePath)
        print(fileGT)

    def groundTruthBifurcation(self,DirPath,file):
        
        # bifurcation mask Generation
        filePath = DirPath + "/" + file
        bifurcationFilePath = DirPath + "/bifurcations_coordinates.txt"
        bifurcationGTPath = DirPath + "/bifurcationGT.nii"
        os.system("./MakeVascuSynthBifurcationGT "+filePath+" "+bifurcationFilePath+" "+bifurcationGTPath+" 7")
        
        print(filePath)
        print(bifurcationFilePath)
        print(bifurcationGTPath)
             
    def noisyImage(self,DirPath,file,outputFile,noiseType):
        imgPath = DirPath + "/" + file

        print(DirPath)
        print(imgPath)
        
        for id,i in enumerate(self.noiseLevels):
            img = itk.imread(imgPath)
            dat = itk.GetArrayFromImage(img)
            
            # simulated CT noise poisson + gaussian noise
            if(id == 0):
                datNoisy = dat
            else:
                if( noiseType == "poisson"):
                    datNoisy = 0.5 * np.random.poisson(dat,None) + 0.5 * np.random.normal(dat,i,None)
                elif( noiseType == "rician"):
                    datNoisy = rice.rvs(dat/i,scale=i)
                else:
                    print("error noise type not supported")
                    
                datNoisy[datNoisy < 0] = 0
                datNoisy[datNoisy > 255] = 255
                # writing image on disk
            if(dat.dtype == np.uint8):
                noisyImg = itk.GetImageFromArray(datNoisy.astype(np.uint8))
            else:
                noisyImg = itk.GetImageFromArray(datNoisy.astype(np.float32)) # data is in double but it is not supported in itk

            print(i)                
            outputPath = DirPath + "/"+outputFile+"_"+str(i)+".nii"
            itk.imwrite(noisyImg,outputPath)
            
            print(outputPath)
    def vesselsAndBackground(self,DirPath,file):
        imgPath = DirPath + "/" + file

        Imin = 50
        Imax = 150
        
        img = itk.imread(imgPath)
        dat = itk.GetArrayFromImage(img)
        
        dat = dat / np.max(dat) * (Imax - Imin) + Imin 
        
        minValue = np.min(dat[dat>0])
        print(minValue)
        dat[dat == 0 ] = minValue

        dat = dat.astype(np.uint8) # for now
        if(dat.dtype == np.uint8):
            img = itk.GetImageFromArray(dat.astype(np.uint8))
        else:
            img = itk.GetImageFromArray(dat.astype(np.float32)) # data is in double but it is not supported in itk

        outputPath = DirPath + "/vesselsAndBackground.nii"
        itk.imwrite(img,outputPath)
            
    def vesselsIllumination(self,DirPath,file ):
        imgPath = DirPath + "/" + file
        outputPath = DirPath + "/vesselsAndBackgroundIlluminated"+".nii"
        sigma = 30
        IMin = 50 
        IMax = 200
                    
        # Retrieving nifti data into arrays
        img = itk.imread(imgPath)
        dat = itk.GetArrayFromImage(img)
        dat = dat.astype(np.float32)
        
        startX = 0
        startY = 0
        startZ = 0
        
        endX = dat.shape[0]
        endY = dat.shape[1]
        endZ = dat.shape[2]
        
        stepX = endX - startX
        stepY = endY - startY
        stepZ = endZ - startZ
                        
        x = np.linspace(startX,endX,stepX)
        y = np.linspace(startY,endY,stepY)
        z = np.linspace(startZ,endZ,stepZ)

        halfSpaceX = (endX - startX) /2
        halfSpaceY = (endY - startY) /2
        halfSpaceZ = (endZ - startZ) /2
                        
        x, y,z = np.meshgrid(x,y,z) # get 2D variables instead of 1D
        print(len(x),len(y),len(z))
        # axial view in slicer is y axis in python
        
        sd1 = sigma
        sd2 = sigma
        sd3 = sigma
        d1 = self.gauss3d(x, y, z,mx=0,my=0,mz=0,sx=sd1,sy=sd1,sz=sd1)
        d1 *= 1.0/d1.max() * (IMax-IMin) + IMin 
                        
        d2 = self.gauss3d(x, y, z,mx=halfSpaceX,my=halfSpaceY,mz=endZ,sx=sd2,sy=sd2,sz=sd2)
        d2 *= 1.0/d2.max() * (IMax-IMin) + IMin
        
        d3 = self.gauss3d(x, y, z,mx=endX,my=endY,mz=endZ,sx=sd3,sy=sd3,sz=sd3)
        d3 *= 1.0/d3.max() * (IMax-IMin) + IMin
        d = d1 + d2 + d3
                        
        dat = np.maximum(d,dat).astype(np.uint8)
                        
        dat[dat < 0] = 0
        dat[dat > 255] = 255
        
        # writing image on disk
        # writing image on disk
        if(dat.dtype == np.uint8):
            illuminatedImg = itk.GetImageFromArray(dat.astype(np.uint8))
        else:
            illuminatedImg = itk.GetImageFromArray(dat.astype(np.float32)) # data is in double but it is not supported in itk
        print(dat.dtype)
            
        itk.imwrite(illuminatedImg,outputPath)

