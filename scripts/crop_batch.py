#!/usr/bin/env python
import sys
import os
import itk
import numpy as np
import glob

outputDir = sys.argv[1]

for patientDirectory in glob.glob('3D*'):
    print(patientDirectory)
    patientPath = patientDirectory + "/PATIENT_DICOM/"
    liverPath = patientDirectory + "/MASKS_DICOM/liver"
    venousPath = patientDirectory + "/MASKS_DICOM/venoussystem/"
    if not os.path.isdir(venousPath) :
        venousPath = patientDirectory + "/MASKS_DICOM/venacava/"
    portalPath = patientDirectory + "/MASKS_DICOM/portalvein/"
    
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
