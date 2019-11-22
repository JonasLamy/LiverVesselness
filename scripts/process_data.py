#!/usr/bin/env python
import sys
import os
import itk
import numpy as np
import glob

outputDir = sys.argv[1]

for patientDirectory in glob.glob('3D*'):
    print(patientDirectory)
    patientPath = outputDir +"/"+ patientDirectory+"/patient.nii"
    liverPath = outputDir +"/"+ patientDirectory+ "/liver.nii"
    venousPath = outputDir +"/"+ patientDirectory+ "/venacava.nii"
    portalPath = outputDir +"/"+ patientDirectory+ "/portal.nii"
        
    print(patientPath)
    print(venousPath)
    print(portalPath)
    print(liverPath)

    if not os.path.exists(outputDir +"/"+ patientDirectory):
        os.makedirs(outputDir +"/"+ patientDirectory)

    patientOutPath = outputDir +"/"+ patientDirectory+"/patientIso.nii"
    liverOutPath = outputDir +"/"+ patientDirectory+ "/liverIso.nii"
    vesselsOutPath = outputDir +"/"+ patientDirectory+ "/vesselsIso.nii"
    
    # calling external program
    # missing outputs
    commandLine = "./MakeIso " + patientPath + " " + liverPath + " " + venousPath + " " + portalPath + " "
    commandLine += patientOutPath + " " + liverOutPath + " " + vesselsOutPath 

    print(commandLine)
    
    os.system(commandLine)
    
