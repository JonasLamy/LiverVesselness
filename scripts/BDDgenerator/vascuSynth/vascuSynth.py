import sys
import os
import numpy as np
import itk
import matplotlib.pyplot as plt
from scipy.stats import rice

class Generator:
    def __init__(self):
        self.noiseLevels = [5.0, 10.0, 20.0]

    def gauss3d(self,x=0,y=0,z=0,mx=0,my=0,mz=0,sx=1,sy=1,sz=1):  
        return  1 / (sx*sy*sz * np.sqrt(2. * np.pi ) * np.sqrt(2. * np.pi ) ) * np.exp(-( (x - mx)**2. / (2. * sx**2.) + (y - my)**2. / (2. * sy**2.) + (z - mz)**2. / (2. * sz**2.) ) )  

    
    def bifurcationCoordinatesFile(self,DirPath,file):
        filePath = DirPath + "/" + file
        fileBif = DirPath + "/bifurcations_coordinates.txt"
        
        matlabCall = "matlab -nojvm -nodisplay -nosplash -r"
        scriptCall = " \"load('"+filePath+"');matTotxt(node,'"+fileBif+"');exit; \" "
        os.system(matlabCall + scriptCall)

    def groundTruth(self,filePath,gtPath,gtDilatedPath):
        #    #now we call the script
        # groundTruth generation
        os.system("./MakeVascuSynthGT "+filePath +" "+gtPath+" "+gtDilatedPath)
        
        print(gtPath)
        print(gtDilatedPath)

    def groundTruthBifurcation(self,DirPath,file):
        
        # bifurcation mask Generation
        filePath = DirPath + "/" + file
        bifurcationFilePath = DirPath + "/bifurcations_coordinates.txt"
        bifurcationGTPath = DirPath + "/bifurcationGT.nii"
        os.system("./MakeVascuSynthBifurcationGT "+filePath+" "+bifurcationFilePath+" "+bifurcationGTPath)
        
        print(filePath)
        print(bifurcationFilePath)
        print(bifurcationGTPath)
             
    def noisyImage(self,inputPath,outputName,noiseType):
        
        for id,i in enumerate(self.noiseLevels):
            img = itk.imread(inputPath)
            dat = itk.GetArrayFromImage(img)
            dat = dat.astype(np.float32)
            # simulated CT noise poisson + gaussian noise
            if( noiseType == "poisson"):
                datNoisy = 0.5 * np.random.poisson(dat,None) + 0.5 * np.random.normal(dat,i,None)
            elif( noiseType == "rician"):
                datNoisy = rice.rvs(dat/i,scale=i)
            else:
                print("error noise type not supported")
                
            datNoisy[datNoisy < 0] = 0
            datNoisy[datNoisy > 255] = 255
                # writing image on disk
            noisyImg = itk.GetImageFromArray(datNoisy.astype(np.uint8))

            print(i)                
            outputPath = outputName+"_"+str(i)+".nii"
            itk.imwrite(noisyImg,outputPath)
            
            print(outputPath)

    def bifurcationsYGT(self,DirPath,gtVessels,gtBifurcations):
        imgGTPath = DirPath + "/" + gtVessels
        imgGTBPath = DirPath + "/" + gtBifurcations

        
        imgGT = itk.imread(imgGTPath)
        imgGTB = itk.imread(imgGTBPath)

        imgGT = itk.GetArrayFromImage(imgGT)
        imgGTB = itk.GetArrayFromImage(imgGTB)

        print(imgGT.shape)
        print(imgGTB.shape)

        # TODO : work in progress

        imgGT[imgGTB == 0] = 0
        
        itk.imwrite( itk.GetImageFromArray(imgGT),DirPath + "/bifurcationGTY.nii")

    def bifurcationsPositionsGT(self,filePath,outputPath,imgSize):
        with open(filePath) as file:
            imgGTPos = np.zeros(imgSize)
            kernelSize = 7
            sigma = 1
            gaussianKernel = np.zeros((kernelSize,kernelSize,kernelSize),dtype=np.float32)
        
            halfKS = int(kernelSize/2)
            for i in range( -halfKS, halfKS ):
                for j in range( -halfKS, halfKS ):
                    for k in range( -halfKS, halfKS ):
                        gaussianKernel[i+halfKS, j+halfKS, k+halfKS] = np.exp( -(i**2 + j**2 + k**2)/(3*sigma*sigma) )
            gaussianKernel = gaussianKernel / np.max(gaussianKernel)
                        
            for line in file:
                coordinates = line.split("\n" )[0].split(",")
                x = int(float(coordinates[0]))
                y = int(float(coordinates[1]))
                z = int(float(coordinates[2]))
                print(x,y,z)

                try :
                    # Warning here itk and numpy indexing are reversed  !!! ITK is [x,y,z] while numpy is [z,y,x]
                    patch = np.maximum(imgGTPos[ z-halfKS:z+halfKS+1, y-halfKS:y+halfKS+1 ,x-halfKS:x+halfKS+1], gaussianKernel)
                    imgGTPos[ z-halfKS:z+halfKS+1, y-halfKS:y+halfKS+1 ,x-halfKS:x+halfKS+1] = patch
                except:
                    pass
            itk.imwrite( itk.GetImageFromArray(imgGTPos.astype(np.float32)), outputPath )
            
    def vesselsAndBackground(self,inputPath,outputPath,Imin,Imax):
        
        img = itk.imread(inputPath)
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

        itk.imwrite(img,outputPath)
            
    def vesselsIllumination(self,inputPath,outputPath,sigma,IMin,IMax):
        # Retrieving nifti data into arrays
        img = itk.imread(inputPath)
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
        d2 = self.gauss3d(x, y, z,mx=halfSpaceX,my=halfSpaceY,mz=halfSpaceZ,sx=sd2,sy=sd2,sz=sd2)
        d3 = self.gauss3d(x, y, z,mx=endX,my=endY,mz=endZ,sx=sd3,sy=sd3,sz=sd3)
        
        d = d1 + d2 + d3
        d = d/d.max() * (IMax-IMin) + IMin
        print("d",np.max(d),np.min(d))
                        
        dat = np.maximum(d,dat)
                        
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

    def paddImages(self,inputPath,outputPath,imageShape):

        img = itk.imread(inputPath)
        img_np = itk.GetArrayFromImage(img)
        
        dat = np.zeros(imageShape)
        # padding
        dat[:img_np.shape[0],:img_np.shape[1],:img_np.shape[2]] = img_np

        dat = dat.astype(np.uint8) # for now
        if(dat.dtype == np.uint8):
            img = itk.GetImageFromArray(dat.astype(np.uint8))
        else:
            img = itk.GetImageFromArray(dat.astype(np.float32)) # data is in double but it is not supported in itk

        itk.imwrite(img,outputPath)

