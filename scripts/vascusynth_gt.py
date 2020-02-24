#!/usr/bin/env python
import sys
import os
import numpy as np
import glob
import fnmatch
import vascuSynth


# vascusynth database collection
inputDir = sys.argv[1]
# resulting database location
outputDir = sys.argv[2]

maskWholeImage = "/DATA/March_2013_VascuSynth_Dataset/maskWholeImage.nii"

#running through top level directories
listDir = next(os.walk(inputDir))

generator = vascuSynth.Generator()
for dirName in listDir[1]:
    if dirName.startswith('Group'):
        # creating group dirs in destination folder
        if not os.path.exists( outputDir + "/" + dirName):
            os.makedirs(outputDir + "/" + dirName)

        # running through data directories
        listData = next(os.walk(listDir[0] + "/" +dirName))[1]
        for data in listData:
            if data.startswith('data'):

                # creating group dirs in destination folder
                if not os.path.exists( outputDir + "/" + dirName + "/" + data):
                    os.makedirs(outputDir + "/" + dirName + "/" + data)
                # retriving files
                listFiles = next(os.walk(listDir[0] + "/" +dirName + "/" + data))[2]

                print(dirName +"_"+ data)
                for file in listFiles:
                    filePath = listDir[0] + "/" +dirName + "/" + data

                    #  create GT files

                    if file.endswith('.mhd'):
                        print(filePath + "/vbi_rician_20.0.nii")
                        print("/DATA/March_2013_VascuSynth_Dataset/maskWholeImage.nii")
                        print(filePath + "/gt.nii")
                        """
                        #    #now we call the script
                        #bifurcation files extraction
                        #spliting file
                        dataNumber = data.rpartition('a')[2]
                        generator.bifurcationCoordinatesFile(filePath,"treeStructure_"+dataNumber+".mat")
                        # groundTruth generation
                        generator.groundTruth(filePath,file)
                        # bifurcation mask Generation
                        generator.groundTruthBifurcation(filePath,file)
                        # background generation
                        generator.vesselsAndBackground(filePath,file)

                         
                        generator.noisyImage(filePath,"vesselsAndBackground.nii","noisyPoisson" ,"poisson")
                        generator.noisyImage(filePath,"vesselsAndBackground.nii","noisyRician" ,"rician")

                        # adding non homogeneous illumination to 
                        # Reading input arguments
                        generator.vesselsIllumination(filePath,"vesselsAndBackground.nii")
                
                        generator.noisyImage(filePath,"vesselsAndBackgroundIlluminated.nii","vbi_poisson" ,"poisson")
                        generator.noisyImage(filePath,"vesselsAndBackgroundIlluminated.nii","vbi_rician" ,"rician")"""
                        
                        
