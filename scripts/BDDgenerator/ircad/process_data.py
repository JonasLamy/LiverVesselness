#!/usr/bin/env python
import sys
import os
import itk
import numpy as np
import glob

outputDir = sys.argv[1]
IdentitySpacing = sys.argv[2]
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

    print(patientOutPath)
    print(vesselsOutPath)
    print(liverOutPath)
    print(bifurcationsOutPath)
    print(dilatedVesselsMask)
    

    
    # calling external program
    # missing outputs
    commandLine = "./MakeIso " + patientPath + " " + liverPath + " " + venousPath + " " + portalPath + " "
    commandLine += patientOutPath + " " + liverOutPath + " " + vesselsOutPath + " " + dilatedVesselsMask + " " + maskedLiverOutPath  + " " + IdentitySpacing

    print(commandLine)
    os.system(commandLine)

    commandLine = "./MakeIrcadBifurcationGT " + vesselsOutPath + " " + liverOutPath  + " " + bifurcationsOutPath
    
    #print(commandLine)
    os.system(commandLine)

    bifurcationInPath = patientDirectory+"/bifurcationsMaskIso.nii"
    liverInPath = patientDirectory+ "/liverMaskIso.nii"
    bifurcationsOutPath = "/DATA/ircad_iso/"+patientDirectory.split("/")[3]+"/bifurcationsMaskIso.nii"
    commandLine = "./isoFromUnit " + bifurcationInPath + " " + liverInPath + " " + bifurcationsOutPath

    #print(commandLine)
    os.system(commandLine)
    
