#!/usr/bin/env python
import sys
import os
import numpy as np
import glob
import fnmatch
import vascuSynth as vascuSynth
import shutil

from scipy import ndimage
import itk


# vascusynth database collection
inputDir = sys.argv[1]
# resulting database location
outputDir = sys.argv[2]

maskWholeImage = outputDir+"/maskWholeImage.nii"

PixelType = itk.UC
Dimension = 3
ImageType = itk.Image[PixelType, Dimension]
#running through top level directories

listDir = next(os.walk(inputDir))

generator = vascuSynth.Generator()

print(listDir)

for dirName in listDir[1][ :int( len(listDir[1]) ) ]:

    if dirName.startswith('Group'):
        # creating group dirs in destination folder
        if not os.path.exists( outputDir + "/" + dirName):
            os.makedirs(outputDir + "/" + dirName)

        # running through data directories
        listData = next(os.walk(listDir[0] + "/" +dirName))[1]
        for data in listData:
            #if( not(data in ["data7","data9","data11"]) ):
            #    continue
            if data.startswith('data'):

                # creating group dirs in destination folder
                if not os.path.exists( outputDir + "/" + dirName + "/" + data):
                    os.makedirs(outputDir + "/" + dirName + "/" + data)
                # retriving files
                listFiles = next(os.walk(listDir[0] + "/" +dirName + "/" + data))[2]

                print(dirName +"_"+ data)

                for file in listFiles:
                    filePath = listDir[0] + "/" +dirName + "/" + data
                    outputFilePath = outputDir + "/" + dirName + "/" + data

                    #  create GT files

                    #if file.endswith('.mhd'):
                    if file.endswith('data.nii'):        
                        
                        dataPath = outputFilePath + "/rician_2.0.nii" 
                        binaryVesselsPath = outputFilePath + "/binaryVessels.nii"
                        binaryBifurcations = outputFilePath + "/binaryBifurcationsMask.nii"
                        vesselsMaskSmall = outputFilePath + "/vesselsMaskSmall.nii"     
                        vesselsMaskMedium = outputFilePath + "/vesselsMaskMedium.nii"     
                        vesselsMaskLarge = outputFilePath + "/vesselsMaskLarge.nii"     
                        vesselsSkeleton = outputFilePath + "/vesselsSkeleton.nii"   
                        dilatedVessels = outputFilePath+"/"+"binaryVesselDilated.nii"
                        dilatedVesselsMaskSmall = outputFilePath + "/dilatedVesselsMaskSmall.nii"
                        dilatedVesselsMaskMedium = outputFilePath + "/dilatedVesselsMaskMedium.nii"
                        dilatedVesselsMaskLarge = outputFilePath + "/dilatedVesselsMaskLarge.nii" 

                        print( dataPath ) # only image at root directory
                        print( binaryVesselsPath )
                        print( maskWholeImage )
                        print( dilatedVessels )
                        print( binaryBifurcations )
                        print( dilatedVesselsMaskLarge )
                        print( dilatedVesselsMaskMedium )
                        print( dilatedVesselsMaskSmall )


                        #shutil.copyfile(filePath +"/"+ "binaryVessels.nii", outputFilePath +"/"+ "binaryVessels.nii")
                        
                        # generate bifurcation text file from .mat data
                        #now we call the script
                        #bifurcation files extraction
                        """
                        dataNumber = data.rpartition('a')[2]
                        generator.bifurcationCoordinatesFile(filePath,"treeStructure_"+dataNumber+".mat")

                        #generator.bifurcationsPositionsGT(filePath+"/"+"bifurcations_coordinates.txt",outputFilePath+"/"+"bifurcationPositions.nii",(128,128,128))
                        generator.paddImages(filePath+"/"+file,outputFilePath+"/"+"data.nii",(128,128,128))

                        # groundTruth generation
                        generator.groundTruth(outputFilePath+"/"+"data.nii",outputFilePath+"/"+"binaryVessels.nii",outputFilePath+"/"+"binaryVesselDilated.nii")

                        # bifurcation mask Generation
                        generator.groundTruthBifurcation(outputFilePath+"/"+"binaryVessels.nii",filePath+"/"+"bifurcations_coordinates.txt",outputFilePath+"/"+"binaryBifurcationsMask.nii")
                        
                        # vessels size mask
                        vesselsSizeEstimation = outputFilePath + "/vesselsSize.nii"  
                        
                        
                        commandLine = "./MakeVascuSynthSizeLabels " + binaryVesselsPath +" "+ outputFilePath + "/skelPurged.nii " + outputFilePath + "/skel.nii " + vesselsSizeEstimation
                        print(commandLine)
                        
                        os.system(commandLine)
                            
                        #commandLine = "./estimObjectWidthFMM -i " + binaryVesselsPath +" -o "+ vesselsSizeEstimation + " 0"
                        #os.system(commandLine)
                        
                        
                        
                        volume = itk.imread( str(vesselsSizeEstimation) )

                        spacing = volume.GetSpacing()
                        npVolume = itk.array_from_image(volume)

                        npVol = npVolume.astype(np.float32) / spacing[0]

                        i=1
                        j=0
                        l = [vesselsMaskSmall,vesselsMaskMedium,vesselsMaskLarge]
                        for bMin,bMax in [(0.1,1),(1.1,2),(2.1,100)]:
                            #npVolume[ (npVol >= bMin) & (npVol <= bMax) ] = i

                            vol = np.zeros(npVolume.shape,dtype=np.uint8)
                            vol[(npVol >= bMin) & (npVol <= bMax)] = 255
                            
                            print("vol",bMin)

                            img = itk.image_from_array(vol)
                            img.SetSpacing( volume.GetSpacing() )
                            img.SetOrigin( volume.GetOrigin() )
                            itk.imwrite(img,l[j])
                            i+=1
                            j+=1


                        sv = itk.imread(vesselsMaskSmall)
                        mv = itk.imread(vesselsMaskMedium)
                        lv = itk.imread(vesselsMaskLarge)


                        vessels = [lv,mv,sv]

                        radius = [7,5,3]
                        volumeNames = [dilatedVesselsMaskLarge,dilatedVesselsMaskMedium,dilatedVesselsMaskSmall]

                        dilatedVessels = []
                        dilatedVesselsNumpy = []
                        dilatedVesselsNumpyStatic = []
                        for v,r,vn in zip(vessels,radius,volumeNames):
                            print(r,vn) 
                            StructuringElementType = itk.FlatStructuringElement[Dimension]
                            structuringElement = StructuringElementType.Ball(r)

                            DilateFilterType = itk.BinaryDilateImageFilter[
                            ImageType, ImageType, StructuringElementType
                            ]
                            dilateFilter = DilateFilterType.New()
                            dilateFilter.SetInput(v)
                            dilateFilter.SetKernel(structuringElement)
                            dilateFilter.SetForegroundValue(255)
                            dilatedVessels.append(dilateFilter.GetOutput()) 
                            
                            dilatedVesselsNumpy.append( itk.array_view_from_image( dilateFilter.GetOutput() ).astype(np.uint8) )
                            dilatedVesselsNumpyStatic.append( itk.array_view_from_image( dilateFilter.GetOutput() ).astype(np.uint8) )

                        dilatedVesselsNumpy[1][ (dilatedVesselsNumpyStatic[1] > 0) & (dilatedVesselsNumpyStatic[0] > 0) ] = 0
                        dilatedVesselsNumpy[2][ (dilatedVesselsNumpyStatic[2] > 0) & (dilatedVesselsNumpyStatic[1] > 0) ] = 0

                        for vn,v,nv in zip(volumeNames,dilatedVessels,dilatedVesselsNumpy):

                            img = itk.image_from_array( nv.astype(np.uint8) )
                            img.SetSpacing( v.GetSpacing() )
                            img.SetOrigin( v.GetOrigin() )

                            WriterType = itk.ImageFileWriter[ImageType]
                            writer = WriterType.New()
                            writer.SetFileName(vn)
                            writer.SetInput(img)

                            writer.Update()
                        

                        # patch generator # TODO make different scripts instead of bumping everything with comments
                        #generator.makeHardClassificationPatches(outputFilePath,"binaryVesselDilated.nii","binaryBifurcationsMask.nii","vesselsNeighbourhoodForClassif.nii")
                        #generator.makeHardClassificationPatches(outputFilePath,"placeholder","binaryVesselDilated.nii","backgroundForClassif.nii")                        
                        
                        # rescale vessels intensity to [Imin,Imax] and background intensity to background value

                        # Note : IRM vessels mean value : 119 ; IRM vessels std : 16
                        # IRM background mean 108 ; IRM background std : 12 
                        # Manual samples on MRI slices.
                        """
                        backgroundValue = 108

                        vesselsPonderation = 0.3
                        backgroundPonderation = 0.4
                        Imin = 103 #140 (value CT ) #110 (value IRM) 
                        Imax = 135 #146 (value CT) #125 (value IRM)

                        # adding non homogeneous illumination to images
                        # sigma = size of the artefacts, Imin = intensity min of the gaussian, Imax = intensity max of the gaussian
                        IgMin =  0 # 0 # (115-108 = 7)
                        IgMax =  12 # 7 ( 108 + 16 = 119)
                        nbGaussian = 3
                        sigmaMin = 30
                        sigmaMax = 50

                        # Note : CT liver mean=101, std=14
                        # Vessels CT mean=139, std=16

                        #IgMin =  0
                        #IgMax =  15
                        #nbGaussian = 3
                        #sigmaMin = 30
                        #sigmaMax = 50
                        
                        print("input dir:",  outputFilePath +"/data.nii")
                        print("output dir:", outputFilePath + "/vesselsAndBackground.nii")
                        generator.vesselsAndBackground( filePath+"/data.nii", outputFilePath + "/vesselsAndBackground.nii",Imin,Imax,backgroundValue,nbGaussian,sigmaMin,sigmaMax,IgMin,IgMax,vesselsPonderation,backgroundPonderation)

                        nbGaussianArtefacts = 10
                        aSigmaMin = 3
                        aSigmaMax = 15
                        aImin = 0
                        aImax = 45
                        
                        generator.vesselsIllumination(outputFilePath+"/vesselsAndBackground.nii",
                                                    outputFilePath+"/vesselsAndBackgroundIlluminated.nii",
                                                    nbGaussianArtefacts,
                                                    aSigmaMin,
                                                    aSigmaMax,
                                                    aImin,
                                                    aImax)

                        #generator.noisyImage(outputFilePath+"/vesselsAndBackgroundIlluminated_ct.nii",outputFilePath+"/poisson" ,"poisson")
                        
                        generator.noisyImage(outputFilePath+"/"+"vesselsAndBackgroundIlluminated.nii",outputFilePath+"/"+"rician" ,"rician")            
