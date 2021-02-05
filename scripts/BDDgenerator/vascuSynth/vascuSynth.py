import sys
import os
import numpy as np
import itk
import matplotlib.pyplot as plt
from scipy.stats import rice
from scipy.stats import norm
import scipy.ndimage

class Generator:
    def __init__(self):
        self.noiseLevels = [2.0, 4.0, 6.0, 8.0]#[2.0,3.0, 4.0,5.0, 6.0,7.0, 8.0, 9.0, 10.0]

    def gauss3d(self,x=0,y=0,z=0,mx=0,my=0,mz=0,sx=1,sy=1,sz=1):  
        return  1 / (sx*sy*sz * np.sqrt(2. * np.pi ) * np.sqrt(2. * np.pi ) ) * np.exp(-( (x - mx)**2. / (2. * sx**2.) + (y - my)**2. / (2. * sy**2.) + (z - mz)**2. / (2. * sz**2.) ) )  

    
    def bifurcationCoordinatesFile(self,DirPath,file):
        filePath = DirPath + "/" + file
        fileBif = DirPath + "/bifurcations_coordinates.txt"

        matlabCall = "matlab -nojvm -nodisplay -nosplash -r"
        scriptCall = " \"load('"+filePath+"');idx = strcmp('bif',{node.type});a = transpose( reshape([node(idx).coord],3,[]) );dlmwrite('"+fileBif+"',a,'delimiter',',');exit; \" "
        print(scriptCall)
        os.system(matlabCall + scriptCall)

    def groundTruth(self,filePath,gtPath,gtDilatedPath):
        #    #now we call the script
        # groundTruth generation
        os.system("./MakeVascuSynthGT "+filePath +" "+gtPath+" "+gtDilatedPath)
        
        print(gtPath)
        print(gtDilatedPath)

    def groundTruthBifurcation(self,binaryVesselsPath,bifurcationTxtFile,bifurcationGtFilePath):
        
        os.system("./MakeVascuSynthBifurcationGT "+binaryVesselsPath+" "+bifurcationTxtFile+" "+bifurcationGtFilePath)
        
        print(binaryVesselsPath)
        print(bifurcationTxtFile)
        print(bifurcationGtFilePath)
             
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
            datNoisy[0,0,0] = 0 # used for easier display in slicer
            datNoisy[0,0,1] = 255 # used for easier display in slicer
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
        
        imgY = np.zeros(imgGT.size)
        
        imgY = imgGT & imgGTB

        print(imgGT.shape)
        print(imgGTB.shape)
        
        itk.imwrite( itk.GetImageFromArray(imgY.astype(np.uint8)),DirPath + "/binaryBifurcations.nii")

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
            
    def vesselsAndBackground(self,inputPath,outputPath,Imin,Imax,backgroundValue,nbGaussianBackground,sigmaMin,sigmaMax,IgMin,IgMax):
        
        img = itk.imread(inputPath)
        dat = itk.GetArrayFromImage(img)
        
        # vessels intensity rescale
        dat = dat / np.max(dat) * (Imax - Imin) + Imin 
            # background intensity
        d = np.zeros(dat.shape)
        for i in range(nbGaussianBackground):
            d += self.makeGaussian(dat,sigmaMin,sigmaMax)
        # background illumination magnetic artefacts
        d = d/d.max() * (0.3) + 0.7
        
        dat[dat>0] = dat[dat>0] * d[dat>0]
        



        # background intensity
        d = np.zeros(dat.shape)
        for i in range(nbGaussianBackground):
            d += self.makeGaussian(dat,sigmaMin,sigmaMax)
        # background illumination magnetic artefacts
        d = d/d.max() * (IgMax-IgMin) + IgMin
        print(IgMin,IgMax,"toto")
        print("d",np.max(d),np.min(d))

        # adding background + vessels (additive gaussian model)
        dat = d+dat

        dat[dat < 0] = 0
        dat[dat > 255] = 255

        dat[0,0,0] = 0 # used for easier display in slicer
        dat[0,0,1] = 255 # used for easier display in slicer
        
        dat = dat.astype(np.uint8) # for now
        if(dat.dtype == np.uint8):
            img = itk.GetImageFromArray(dat.astype(np.uint8))
        else:
            img = itk.GetImageFromArray(dat.astype(np.float32)) # data is in double but it is not supported in itk

        itk.imwrite(img,outputPath)



    def makeGaussian(self,dat,sigmaMin,sigmaMax):
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
            
            x, y,z = np.meshgrid(x,y,z) # get 2D variables instead of 1D
            print(len(x),len(y),len(z))
        
            m_x = np.random.randint(0,endX)
            m_y = np.random.randint(0,endY)
            m_z = np.random.randint(0,endZ)

            sigma1 = np.random.randint(sigmaMin,sigmaMax)
            sigma2 = np.random.randint(sigmaMin,sigmaMax)
            sigma3 = np.random.randint(sigmaMin,sigmaMax)

            return self.gauss3d(x, y, z,mx=m_x,my=m_y,mz=m_z,sx=sigma1,sy=sigma2,sz=sigma3)


    def vesselsIllumination(self,
                            inputPath,
                            outputPath,
                            nbGaussianArtefacts,
                            aSigmaMin,
                            aSigmaMax,
                            aImin,
                            aImax):
        # Retrieving nifti data into arrays
        img = itk.imread(inputPath)
        dat = itk.GetArrayFromImage(img)
        dat = dat.astype(np.float32)
        
        # Artefacts
        a = np.zeros(dat.shape)
        if(nbGaussianArtefacts >0):
            for i in range(nbGaussianArtefacts):
                a += self.makeGaussian(dat,aSigmaMin,aSigmaMax)
            
            a = a/a.max() * (aImax-aImin) + aImin
            a[a < aImin + 2] = 0
            print("a",np.max(a),np.min(a))
            dat = a + dat

                        
        dat[dat < 0] = 0
        dat[dat > 255] = 255
        
        dat[0,0,0] = 0 # used for easier display in slicer
        dat[0,0,1] = 255 # used for easier display in slicer

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

    def imageFromGaussianPDF(self,inputPath,outputPath,dataType):
        if(dataType == "MRI" ):
            print("using MRI")
            gaussianPDFLiver = norm(loc=108,scale=12)
            gaussianPDFVessels = norm(loc=119,scale=16)
        if(dataType == "CT" ):
            print("using CT")
            gaussianPDFLiver = norm(loc=101,scale=14)
            gaussianPDFVessels = norm(loc=139,scale=16)

        img = itk.imread(inputPath)
        img_np = itk.GetArrayFromImage(img)
        
        mask = (img_np == 0)
        img_np[mask] = gaussianPDFLiver.rvs(img_np[mask].shape[0])
        
        mask_vessels = (img_np > 0)
        img_np[mask_vessels] = gaussianPDFVessels.rvs(img_np[mask_vessels].shape[0])

        dat = img_np
        dat = dat.astype(np.uint8) # for now
        if(dat.dtype == np.uint8):
            img = itk.GetImageFromArray(dat.astype(np.uint8))
        else:
            img = itk.GetImageFromArray(dat.astype(np.float32)) # data is in double but it is not supported in itk

        itk.imwrite(img,outputPath)