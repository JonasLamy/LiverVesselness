import sys
import os
import numpy as np
import itk
import matplotlib.pyplot as plt
from scipy.stats import rice
from scipy.stats import norm
from scipy import ndimage
import scipy.ndimage

class Generator:
    def __init__(self):
        self.noiseLevels = [2.0, 4.0, 6.0, 8.0]#[6.0, 8.0, 10.0, 20.0, 30.0, 40.0,70.0,150]##

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
        
        print("-- debug --")
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
                dat[dat <= 0] = 1
                datNoisy = rice.rvs(dat/i,scale=i)
            else:
                print("error noise type not supported")
                
            datNoisy[datNoisy < 0] = 0
            #datNoisy[datNoisy > 255] = 255
            datNoisy[0,0,0] = 0 # used for easier display in slicer
            #datNoisy[0,0,1] = 255 # used for easier display in slicer
                # writing image on disk
            noisyImg = itk.GetImageFromArray(datNoisy.astype(np.short) ) #.astype(np.uint8))

            print(i)                
            outputPath = outputName+"_"+str(i)+".nii.gz"
            noisyImg.SetOrigin(img.GetOrigin())
            noisyImg.SetSpacing(img.GetSpacing())
            itk.imwrite(noisyImg,outputPath)
            
            print(outputPath)

    def gaussianVariations(self,data, outputPath, sigmaMin, sigmaMax, nbGaussianBackground,ponderationWeight,dataOrigin,dataSpacing):

        # background intensity
        bgPonderation = np.zeros(data.shape)
        for i in range(nbGaussianBackground):
            bgPonderation += self.makeGaussian(bgPonderation,sigmaMin,sigmaMax)
        # background illumination magnetic artefacts
        bgPonderation = bgPonderation/bgPonderation.max() * ponderationWeight + (1-ponderationWeight)

        print("bgPonderation",np.max(bgPonderation),np.min(bgPonderation))

        dat = ( data + np.min(data) ) * bgPonderation

        #dat[dat < 0] = 0
        #dat[dat > 255] = 255

        dat[0,0,0] = 0 # used for easier display in slicer
        #dat[0,0,1] = 255 # used for easier display in slicer
        
        
        img = itk.GetImageFromArray(dat.astype(itk.ctype("short") )) # data is in double but it is not supported in itk
        img.SetOrigin(dataOrigin)
        img.SetSpacing(dataSpacing)
        itk.imwrite(img,outputPath)

        return

    def gaussianVariationsInMask(self,data, outputPath, sigmaMin, sigmaMax, nbGaussianBackground,ponderationWeight,dataOrigin,dataSpacing,mask):

        # background intensity
        bgPonderation = np.zeros(data.shape)
        for i in range(nbGaussianBackground):
            bgPonderation += self.makeGaussianInMask(bgPonderation,sigmaMin,sigmaMax,mask)
        # background illumination magnetic artefacts
        bgPonderation = bgPonderation/bgPonderation.max() * ponderationWeight + (1-ponderationWeight)

        print("bgPonderation",np.max(bgPonderation),np.min(bgPonderation))

        dat = ( data + np.min(data) ) * bgPonderation

        #dat[dat < 0] = 0
        #dat[dat > 255] = 255

        dat[0,0,0] = 0 # used for easier display in slicer
        #dat[0,0,1] = 255 # used for easier display in slicer
        
        
        img = itk.GetImageFromArray(dat.astype(itk.ctype("short") )) # data is in double but it is not supported in itk
        img.SetOrigin(dataOrigin)
        img.SetSpacing(dataSpacing)
        itk.imwrite(img,outputPath)

        return

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

    def makeHardClassificationPatches(self,DirPath,inputPath,maskPath,outputPath):
        imgInputPath = DirPath + "/" + inputPath #"/DATA/vascu_deepV2/maskWholeImage.nii"
        imgMaskPath = DirPath + "/" + maskPath
        imgROIPath = DirPath + "/" + outputPath

        imgInput = itk.imread(imgInputPath)
        imgMask = itk.imread(imgMaskPath)

        PixelType = itk.UC
        Dimension = 3
        ImageType = itk.Image[PixelType, Dimension]
        
        radiusValue = 4
        StructuringElementType = itk.FlatStructuringElement[Dimension]
        structuringElement = StructuringElementType.Ball(radiusValue)
        
        DilateFilterType = itk.BinaryDilateImageFilter[ImageType,
                                                    ImageType,
                                                    StructuringElementType]
        
        dilateFilter = DilateFilterType.New()
        
        dilateFilter.SetInput(imgMask)
        dilateFilter.SetKernel(structuringElement)
        dilateFilter.SetForegroundValue(255)
        imgMask = dilateFilter.GetOutput()

        imgInput = itk.GetArrayFromImage(imgInput)
        imgMask = itk.GetArrayFromImage(imgMask)


        
        imgROI = np.copy(imgInput)
        imgROI[ imgMask > 0 ] = 0
        
        
        itk.imwrite( itk.GetImageFromArray(imgROI.astype(np.uint8)),imgROIPath)
            
    def vesselsAndBackground(self,inputPath,outputPath,Imin,Imax,backgroundValue,nbGaussianBackground,sigmaMin,sigmaMax,IgMin,IgMax,VponderationWeight,BGponderationWeight):
        
        img = itk.imread(inputPath)
        vessels = itk.GetArrayFromImage(img)
        dat_save = itk.GetArrayFromImage(img)

        # process
        # get vessels
        # ponderate using gaussians
        # rescale so that lowest pixels is higher than background
        # add bachground to it
        # use ponderation on whole image
        # add noise
        # add artefacts
     
        # vessels intensity rescale 
        # generating gaussian ponderation
        vesselsPonderation = np.zeros(vessels.shape)
        for i in range(nbGaussianBackground):
            vesselsPonderation += self.makeGaussian(vesselsPonderation,sigmaMin,sigmaMax)
        print("vessels ponderation min-max:",vesselsPonderation.min(),vesselsPonderation.max())
        vesselsPonderation = vesselsPonderation/vesselsPonderation.max() * BGponderationWeight + (1-BGponderationWeight)
        # applying ponderation
        vessels[vessels>0] = vessels[vessels>0] * vesselsPonderation[vessels>0]
        
                # vessels are rescaled to desired intensity and weighted by gaussian intensity artefacts
        vessels[vessels>0] = vessels[vessels>0] / np.max(vessels) * (Imax - Imin) + Imin

        vessels[vessels==0] += backgroundValue
        VesselsAndBackground = vessels

        # background intensity
        bgPonderation = np.zeros(VesselsAndBackground.shape)
        for i in range(nbGaussianBackground):
            bgPonderation += self.makeGaussian(bgPonderation,sigmaMin,sigmaMax)
        # background illumination magnetic artefacts
        bgPonderation = bgPonderation/bgPonderation.max() * VponderationWeight + (1-VponderationWeight)
        #bgPonderation = bgPonderation/bgPonderation.max() * (IgMax-IgMin) + IgMin
        print(IgMin,IgMax,"background min-max")
        print("bgPonderation",np.max(bgPonderation),np.min(bgPonderation))

        dat = VesselsAndBackground * bgPonderation

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
        
        y,x, z = np.meshgrid(y,x,z) # get 3D variables instead of 1D
        print(len(x),len(y),len(z))
    
        m_x = np.random.randint(0,endX)
        m_y = np.random.randint(0,endY)
        m_z = np.random.randint(0,endZ)

        sigma1 = np.random.randint(sigmaMin,sigmaMax)
        sigma2 = np.random.randint(sigmaMin,sigmaMax)
        sigma3 = np.random.randint(sigmaMin,sigmaMax)

        return self.gauss3d(y,x, z,mx=m_y,my=m_x,mz=m_z,sx=sigma2,sy=sigma1,sz=sigma3)

    def makeGaussianInMask(self,dat,sigmaMin,sigmaMax,mask):
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
        
        y,x, z = np.meshgrid(y,x,z) # get 3D variables instead of 1D
        print(len(x),len(y),len(z))

        posVessels = np.where( mask > 0 )

        shuffledIndex = np.random.permutation(len(posVessels[0]))
        shuffledPosVessels = posVessels[0][shuffledIndex],posVessels[1][shuffledIndex],posVessels[2][shuffledIndex]
        
        m_x = shuffledPosVessels[0][0]#np.random.randint(0,endX)
        m_y = shuffledPosVessels[1][0]#np.random.randint(0,endY)
        m_z = shuffledPosVessels[2][0]#np.random.randint(0,endZ)

        sigma1 = np.random.randint(sigmaMin,sigmaMax)
        sigma2 = np.random.randint(sigmaMin,sigmaMax)
        sigma3 = np.random.randint(sigmaMin,sigmaMax)

        return self.gauss3d(y,x, z,mx=m_y,my=m_x,mz=m_z,sx=sigma2,sy=sigma1,sz=sigma3)

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