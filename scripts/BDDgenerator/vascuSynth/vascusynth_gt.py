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

                    if file.endswith('.mhd'):
                        
                        #print(filePath +"/"+ "vbi_rician_20.0.nii")
                        #print("vascu_2013/maskWholeImage.nii") # only image at root directory
                        #print(filePath +"/"+ "bifurcationGT.nii")
                        #print(filePath +"/"+ "gtDilated.nii")
                        #print(filePath +"/"+ "gt.nii")

                        # generate bifurcation text file from .mat data
                        #now we call the script
                        #bifurcation files extraction
                        #dataNumber = data.rpartition('a')[2]
                        #generator.bifurcationCoordinatesFile(filePath,"treeStructure_"+dataNumber+".mat")

                        # generate bifurcations groundtruth in fork shapes (work in progress)
                        #generator.bifurcationsYGT(filePath,"gt.nii","bifurcationGT.nii")

                        #generator.bifurcationsPositionsGT(filePath+"/"+"bifurcations_coordinates.txt",outputFilePath+"/"+"bifurcationsGT.nii",(128,128,128))
                        #generator.paddImages(filePath+"/"+file,outputFilePath+"/"+"data.nii",(128,128,128))

                        # groundTruth generation
                        #generator.groundTruth(outputFilePath+"/"+"data.nii",outputFilePath+"/"+"gt.nii",outputFilePath+"/"+"gtDilated.nii")
                        
                        # bifurcation mask Generation
                        #generator.groundTruthBifurcation(filePath,"gt.nii")
                        # background generation

                        # rescale vessels intensity to [Imin,Imax] and set background intensity to Imin
                        Imin = 50
                        Imax = 100
                        generator.vesselsAndBackground(outputFilePath+"/"+"data.nii",outputFilePath + "/vesselsAndBackground.nii",Imin,Imax)

                        # adding non homogeneous illumination to images
                        # sigma = size of the artefacts, Imin = intensity min of the gaussian, Imax = intensity max of the gaussian
                        sigma = 40
                        Imin = 50
                        Imax = 100
                        
                        generator.vesselsIllumination(outputFilePath+"/"+"vesselsAndBackground.nii",outputFilePath+"/"+"vesselsAndBackgroundIlluminated.nii",sigma,Imin,Imax)

                        #generator.noisyImage(filePath,"vesselsAndBackgroundIlluminated.nii","vbi_poisson" ,"poisson")
                        generator.noisyImage(outputFilePath+"/"+"vesselsAndBackgroundIlluminated.nii",outputFilePath+"/"+"rician" ,"rician")
                        
