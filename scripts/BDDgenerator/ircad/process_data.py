#!/usr/bin/env python
import sys
import os
import itk
import numpy as np
import glob

outputDir = sys.argv[1]
IdentitySpacing = sys.argv[2]


bornes = {
    "3Dircadb1.1":((1,3),(4,6),(7,30)),
    "3Dircadb1.2":((1,2),(3,5),(7,30)),
    "3Dircadb1.3":((1,2),(3,7),(8,30)),
    "3Dircadb1.4":((1,3),(4,6),(7,30)),
    "3Dircadb1.5":((1,3),(4,6),(7,30)),
    "3Dircadb1.6":((1,5),(6,7),(8,30)),
    "3Dircadb1.7":((1,4),(5,7),(8,30)),
    "3Dircadb1.8":((1,5),(6,8),(9,30)),
    "3Dircadb1.9":((1,3),(4,7),(8,30)),
    "3Dircadb1.10":((1,3),(4,6),(7,30)),
    "3Dircadb1.11":((1,4),(5,7),(7,30)),
    "3Dircadb1.12":((1,5),(6,8),(10,30)),
    "3Dircadb1.13":((1,4),(5,8),(10,30)),
    "3Dircadb1.14":((1,3),(4,8),(10,30)),
    "3Dircadb1.15":((1,2),(3,6),(7,30)),
    "3Dircadb1.16":((1,4),(5,7),(8,30)),
    "3Dircadb1.17":((1,6),(7,9),(10,30)),
    "3Dircadb1.18":((1,2),(3,5),(6,30)),
    "3Dircadb1.19":((1,3),(4,7),(9,30)),
    "3Dircadb1.20":((1,4),(5,8),(9,30)),
}
# Actual thresholds for the 3 vessels marks

# [(0.1,3),(3.1,6),(6.1,100)]

for patientDirectory in glob.glob(outputDir +'/3D*'):
    print(patientDirectory.rsplit('/')[-1])

    patientPath =  patientDirectory+"/patient.nii"
    liverPath =  patientDirectory+ "/liver.nii"
    venousPath =  patientDirectory+ "/venacava.nii"
    portalPath =  patientDirectory+ "/portal.nii"
    vesselsIsoCutPath = patientDirectory+ "/vesselsIsoCut.nii"

    if not os.path.exists( patientDirectory):
        os.makedirs(patientDirectory)

    patientOutPath = patientDirectory+"/patientIso.nii"
    liverOutPath = patientDirectory+ "/liverMaskIso.nii"
    vesselsOutPath = patientDirectory+ "/vesselsIso.nii"
    vesselsCutOutPath = patientDirectory+ "/vesselsIsoCut_.nii"
    maskedLiverOutPath = patientDirectory+"/maskedLiverIso.nii"
    maskedLiverAndVesselsOutPath = patientDirectory+"/maskedLiverAndVesselsIso.nii"
    bifurcationsOutPath = patientDirectory+"/bifurcationsMaskIso.nii"
    dilatedVesselsMask = patientDirectory + "/dilatedVesselsMaskIso.nii"
    dilatedVesselsMasked = patientDirectory + "/dilatedVesselsMasked.nii"

    vesselsSizeEstimation = patientDirectory + "/vesselsSize.nii"    
    vesselsSizeEstimationMask = patientDirectory + "/vesselsSizeMask.nii"
    vesselsMaskSmall = patientDirectory + "/vesselsMaskSmall.nii"     
    vesselsMaskMedium = patientDirectory + "/vesselsMaskMedium.nii"     
    vesselsMaskLarge = patientDirectory + "/vesselsMaskLarge.nii"     

    print(patientOutPath)
    print(vesselsOutPath)
    print(liverOutPath)
    print(bifurcationsOutPath)
    print(dilatedVesselsMask)
    print(vesselsCutOutPath)
    print(dilatedVesselsMasked)


    #
    # Make masks with protal trunk using liver mask + vessels mask
    #
    """
    # vessels volume
    volume = itk.imread(vesselsIsoCutPath)
    npVolume = itk.array_from_image(volume).astype(np.uint8)
    npVolume[npVolume>0] = 255

    # liver volume
    liver = itk.imread(liverOutPath)
    npLiver = itk.array_from_image(liver).astype(np.uint8)

    print("liver",npLiver.shape)
    print("vessels",npVolume.shape)

    fusedMask = np.zeros( npLiver.shape, dtype=np.uint8 )
    fusedMask[ np.ma.mask_or(npLiver,npVolume) ] = 255

    img = itk.image_from_array(npVolume)
    img.SetSpacing( volume.GetSpacing() )
    img.SetOrigin( volume.GetOrigin() )
    itk.imwrite(img,vesselsCutOutPath)

    del img

    img = itk.image_from_array(fusedMask)
    img.SetSpacing( volume.GetSpacing() )
    img.SetOrigin( volume.GetOrigin() )
    itk.imwrite(img,maskedLiverAndVesselsOutPath)
    """
    #
    # Make vesselsNeighbourhood mask masked by liver mask (not the fused one but the liver one, important)
    #

    # dilated volumes
    """
    volume = itk.imread(dilatedVesselsMask)
    npVolume = itk.array_from_image(volume)

    # liver volume
    liver = itk.imread(maskedLiverAndVesselsOutPath)
    npLiver = itk.array_from_image(liver)

    # mask dilated vessels with liver mask
    fusedMask = np.zeros( npLiver.shape, dtype=np.uint8 )
    mask = (npLiver > 0) & (npVolume > 0)
    fusedMask[ mask ] = 255

    img = itk.image_from_array(fusedMask)
    img.SetSpacing( volume.GetSpacing() )
    img.SetOrigin( volume.GetOrigin() )
    itk.imwrite(img,dilatedVesselsMasked)
    
    commandLine = "./estimObjectWidthFMM -i " + vesselsCutOutPath +" -o "+ vesselsSizeEstimation + " 0"
    os.system(commandLine)
    """
    # thresholding values
    
    """
    volume = itk.imread(vesselsSizeEstimation)
    spacing = volume.GetSpacing()
    npVolume = itk.array_from_image(volume)

    npVol = npVolume.astype(np.float32) / spacing[0]

    bornSet = bornes[patientDirectory.rsplit('/')[-1]]
    i=1
    j=0
    print(bornSet)
    l = [vesselsMaskSmall,vesselsMaskMedium,vesselsMaskLarge]
    for bMin,bMax in [(0.1,3),(3.1,6),(6.1,100)]:#bornSet:
        #npVolume[ (npVol >= bMin) & (npVol <= bMax) ] = i

        vol = np.zeros(npVolume.shape,dtype=np.uint8)
        vol[(npVol >= bMin) & (npVol <= bMax)] = 255
        
        img = itk.image_from_array(vol)
        img.SetSpacing( volume.GetSpacing() )
        img.SetOrigin( volume.GetOrigin() )
        itk.imwrite(img,l[j])
        i+=1
        j+=1
    # calling external program
    # missing outputs
    #commandLine = "./MakeIso " + patientPath + " " + liverPath + " " + venousPath + " " + portalPath + " "
    #commandLine += patientOutPath + " " + liverOutPath + " " + vesselsOutPath + " " + dilatedVesselsMask + " " + maskedLiverOutPath  + " " + IdentitySpacing
    """
    #print(commandLine)
    #os.system(commandLine)

    commandLine = "./MakeIrcadBifurcationGT " + vesselsCutOutPath + " " + liverOutPath  + " " + bifurcationsOutPath
    
    print(commandLine)
    os.system(commandLine)
    exit()
    """
    bifurcationInPath = patientDirectory+"/bifurcationsMaskIso.nii"
    liverInPath = patientDirectory+ "/liverMaskIso.nii"
    bifurcationsOutPath = "/DATA/ircad_iso/"+patientDirectory.split("/")[3]+"/bifurcationsMaskIso.nii"
    commandLine = "./isoFromUnit " + bifurcationInPath + " " + liverInPath + " " + bifurcationsOutPath
    #print(commandLine)
    os.system(commandLine)
    """
    
