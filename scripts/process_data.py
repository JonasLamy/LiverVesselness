#!/usr/bin/env python
import sys
import os
import itk
import numpy as np
import glob

outputDir = sys.argv[1]
IdentitySpacing = sys.argv[2]
bifurcationBoxSize = "5"
for patientDirectory in glob.glob(outputDir +'/3D*'):
    print(patientDirectory.rsplit('/')[-1])

    patientPath =  patientDirectory+"/patient.nii"
    liverPath =  patientDirectory+ "/liver.nii"
    venousPath =  patientDirectory+ "/venacava.nii"
    portalPath =  patientDirectory+ "/portal.nii"
        

    if not os.path.exists( patientDirectory):
        os.makedirs(patientDirectory)

    patientOutPath = patientDirectory+"/patientIso.nii"
    liverOutPath = patientDirectory+ "/liverMaskIso.nii"
    vesselsOutPath = patientDirectory+ "/vesselsIso.nii"
    maskedLiverOutPath = patientDirectory+"/maskedLiverIso.nii"
    bifurcationsOutPath = patientDirectory+"/bifurcationsMaskIso.nii"
    dilatedVesselsMask = patientDirectory + "/dilatedVesselsMaskIso.nii"

    print(maskedLiverOutPath)
    print(liverOutPath)
    print(bifurcationsOutPath)
    print(dilatedVesselsMask)
    print(vesselsOutPath)

    
    # calling external program
    # missing outputs
    commandLine = "./MakeIso " + patientPath + " " + liverPath + " " + venousPath + " " + portalPath + " "
    commandLine += patientOutPath + " " + liverOutPath + " " + vesselsOutPath + " " + dilatedVesselsMask + " " + maskedLiverOutPath  + " " + IdentitySpacing

    #print(commandLine)
    #os.system(commandLine)

    commandLine = "./MakeIrcadBifurcationGT " + vesselsOutPath + " " + liverOutPath  + " " + bifurcationsOutPath  + " "+ bifurcationBoxSize
    
    #print(commandLine)
    #os.system(commandLine)
