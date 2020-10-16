#!/usr/bin/env python
import sys
import os
import itk
import numpy as np
import fnmatch

inputDir = sys.argv[1]
outputDir = sys.argv[2]
for patientDirectory in fnmatch.filter( os.listdir(inputDir),'3D*'):
    print(patientDirectory)

    
    patientPath = inputDir +"/"+ patientDirectory + "/PATIENT_DICOM/"
    liverPath = inputDir +"/"+ patientDirectory + "/MASKS_DICOM/liver"
    venousPath = inputDir +"/"+ patientDirectory + "/MASKS_DICOM/venoussystem/"
    if not os.path.isdir(venousPath) :
        venousPath = inputDir +"/"+ patientDirectory + "/MASKS_DICOM/venacava/"
    portalPath = inputDir +"/"+ patientDirectory + "/MASKS_DICOM/portalvein/"
    
    print(patientPath)
    print(venousPath)
    print(portalPath)
    print(liverPath)

    if not os.path.exists(outputDir +"/"+ patientDirectory):
        os.makedirs(outputDir +"/"+ patientDirectory)

    patientOutPath = outputDir +"/"+ patientDirectory+"/patient.nii"
    liverOutPath = outputDir +"/"+ patientDirectory+ "/liver.nii"
    venousOutPath = outputDir +"/"+ patientDirectory+ "/venacava.nii"
    portalOutPath = outputDir +"/"+ patientDirectory+ "/portal.nii"
    
    # calling external program
    # missing outputs
    commandLine = "./CropImages " + patientPath + " " + liverPath + " " + venousPath + " " + portalPath + " "
    commandLine += patientOutPath + " " + liverOutPath + " " + venousOutPath + " " + portalOutPath

    print(commandLine)
    
    os.system(commandLine)
    
