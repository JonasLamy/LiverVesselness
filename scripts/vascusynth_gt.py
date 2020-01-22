#!/usr/bin/env python
import sys
import os
import numpy as np
import glob
import fnmatch

# import for noise generation
import itk
from scipy.stats import rice

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

                    #if file.endswith('.mhd'):
                        #    #now we call the script
                        # groundTruth generation
                        #filePath = listDir[0] + "/" +dirName + "/" + data + "/" + file
                        #fileGT = listDir[0] + "/" +dirName + "/" + data + "/gt.nii"
                        #os.system("./MakeVascuSynthGT "+filePath +" "+fileGT)

                        #print(filePath)
                        #print(fileGT)
                        
                        # bifurcation mask Generation
                        #filePath = listDir[0] + "/" +dirName + "/" + data + "/" + file
                        #bifurcationFilePath = listDir[0] + "/" +dirName + "/" + data + "/bifurcations_coordinates.txt"
                        #bifurcationGTpath = listDir[0] + "/" +dirName + "/" + data + "/bifurcationGT.nii"
                        #os.system("./MakeVascuSynthBifurcationGT "+filePath+" "+bifurcationFilePath+" "+bifurcationGTpath+" 5")

                        #print(filePath)
                        #print(bifurcationFilePath)
                        #print(fileGT)
                        
    
                    if( file == "vesselsAndBackground.nii" ):
                        imgPath = listDir[0] + "/" +dirName + "/" + data + "/" + file

                        print(dirName)
                        print(imgPath)
                        
                        for id,i in enumerate([0.0, 2.0, 5.0, 10.0, 15.0, 20.0]):
                            img = itk.imread(imgPath)
                            dat = itk.GetArrayFromImage(img)

                            minValue = np.min(dat[dat>0])
                            print(minValue)
                            dat[dat == 0 ] = minValue
                            # simulated CT noise poisson + gaussian noise
                            if(id == 0):
                                datNoisy = dat
                            else:
                                datNoisy = rice.rvs(dat/i,scale=i)#np.random.poisson(dat,None) + np.random.normal(dat,i,None)
                                datNoisy[datNoisy < 0] = 0
                                datNoisy[datNoisy > 255] = 255
                            # writing image on disk
                            if(dat.dtype == np.uint8):
                                noisyImg = itk.GetImageFromArray(datNoisy.astype(np.uint8))
                            else:
                                noisyImg = itk.GetImageFromArray(datNoisy.astype(np.float32)) # data is in double but it is not supported in itk
                            print(i)
                            outputPath = listDir[0] + "/" +dirName + "/" + data + "/noisyRician_"+str(i)+".nii"
                            itk.imwrite(noisyImg,outputPath)
                            
                            print(outputPath)
