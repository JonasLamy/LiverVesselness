#!/usr/bin/env python
import sys
import os
import numpy as np
import glob
import fnmatch

# vascusynth database collection
inputDir = sys.argv[1]
# resulting database location
outputDir = sys.argv[2]

maskWholeImage = "/DATA/March_2013_VascuSynth_Dataset/maskWholeImage.nii"

#running through top level directories
listDir = next(os.walk(inputDir))

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
                    # create bifurcation coordiates file
                    #if file.endswith('.mat'):
                    #    filePath = listDir[0] + "/" +dirName + "/" + data + "/" + file
                    #    fileBif = listDir[0] + "/" +dirName + "/" + data + "/bifurcations_coordinates.txt"

                    #    matlabCall = "matlab -nojvm -nodisplay -nosplash -r"
                    #    scriptCall = " \"load('"+filePath+"');matTotxt(node,'"+fileBif+"');exit; \" "
                    #    os.system(matlabCall + scriptCall)
                    #  create GT files
                    if file.endswith('.mhd'):
                    #    #now we call the script
                        filePath = listDir[0] + "/" +dirName + "/" + data + "/" + file
                        fileGT = listDir[0] + "/" +dirName + "/" + data + "/gt.nii"    
                    #    os.system("./MakeVascuSynthGT "+filePath +" "+fileGT)
                        print(filePath)
                        print(maskWholeImage)
                        print(fileGT)
                        
    
