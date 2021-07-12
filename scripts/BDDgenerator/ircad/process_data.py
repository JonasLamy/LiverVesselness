#!/usr/bin/env python
import sys
import os
import itk
import numpy as np
import glob

outputDir = sys.argv[1]
IdentitySpacing = sys.argv[2]



# Actual thresholds for the 3 vessels marks
# [(0.1,3),(3.1,6),(6.1,100)]
#/pbs/home/j/jlamy/LiverVesselnessDev/build/bin/

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
    #dilatedVesselsMasked = patientDirectory + "/dilatedVesselsCutMasked.nii"

    vesselsSizeEstimation = patientDirectory + "/vesselsSize.nii"    
    vesselsSizeEstimationMask = patientDirectory + "/vesselsSizeMask.nii"
    vesselsMaskSmall = patientDirectory + "/vesselsMaskSmall.nii"     
    vesselsMaskMedium = patientDirectory + "/vesselsMaskMedium.nii"     
    vesselsMaskLarge = patientDirectory + "/vesselsMaskLarge.nii"     
    vesselsSkeleton = patientDirectory + "/vesselsSkeleton.nii"     

    dilatedVesselsMaskSmall = patientDirectory + "/dilatedVesselsMaskSmall.nii"
    dilatedVesselsMaskMedium = patientDirectory + "/dilatedVesselsMaskMedium.nii"
    dilatedVesselsMaskLarge = patientDirectory + "/dilatedVesselsMaskLarge.nii"

    print(patientOutPath)
    print(vesselsCutOutPath)
    print(maskedLiverAndVesselsOutPath)
    print(dilatedVesselsMasked)
    print(bifurcationsOutPath)
    print(dilatedVesselsMaskLarge)
    print(dilatedVesselsMaskMedium)
    print(dilatedVesselsMaskSmall)

    
    # Make dilated masks for vessels of different sizes
    
    sv = itk.imread(vesselsMaskSmall)
    mv = itk.imread(vesselsMaskMedium)
    lv = itk.imread(vesselsMaskLarge)

    PixelType = itk.UC
    Dimension = 3
    ImageType = itk.Image[PixelType, Dimension]

    vessels = [lv,mv,sv]
    radius = [9,7,5]
    volumeNames = [dilatedVesselsMaskLarge,dilatedVesselsMaskMedium,dilatedVesselsMaskSmall]

    dilatedVessels = []
    dilatedVesselsNumpy = []
    dilatedVesselsNumpyStatic = []
    for v,r,vn in zip(vessels,radius,volumeNames):
        print(r,vn) 
        StructuringElementType = itk.FlatStructuringElement[Dimension]
        structuringElement = StructuringElementType.Ball(r)

        DilateFilterType = itk.BinaryDilateImageFilter[
        ImageType, ImageType, StructuringElementType
        ]
        dilateFilter = DilateFilterType.New()
        dilateFilter.SetInput(v)
        dilateFilter.SetKernel(structuringElement)
        dilateFilter.SetForegroundValue(255)
        dilatedVessels.append(dilateFilter.GetOutput()) 
        
        dilatedVesselsNumpy.append( itk.array_view_from_image( dilateFilter.GetOutput() ).astype(np.uint8) )
        dilatedVesselsNumpyStatic.append( itk.array_view_from_image( dilateFilter.GetOutput() ).astype(np.uint8) )

    #dilatedVesselsNumpy[0][ (dilatedVesselsNumpyStatic[0] > 0) & (dilatedVesselsNumpyStatic[1] > 0) ] = 0
    #dilatedVesselsNumpy[0][ (dilatedVesselsNumpyStatic[0] > 0) & (dilatedVesselsNumpyStatic[2] > 0) ] = 0

    dilatedVesselsNumpy[1][ (dilatedVesselsNumpyStatic[1] > 0) & (dilatedVesselsNumpyStatic[0] > 0) ] = 0
    #dilatedVesselsNumpy[1][ (dilatedVesselsNumpyStatic[1] > 0) & (dilatedVesselsNumpyStatic[2] > 0) ] = 0
    
    #dilatedVesselsNumpy[2][ (dilatedVesselsNumpyStatic[2] > 0) & (dilatedVesselsNumpyStatic[0] > 0) ] = 0
    dilatedVesselsNumpy[2][ (dilatedVesselsNumpyStatic[2] > 0) & (dilatedVesselsNumpyStatic[1] > 0) ] = 0

    for vn,v,nv in zip(volumeNames,dilatedVessels,dilatedVesselsNumpy):

        img = itk.image_from_array( nv.astype(np.uint8) )
        img.SetSpacing( v.GetSpacing() )
        img.SetOrigin( v.GetOrigin() )

        WriterType = itk.ImageFileWriter[ImageType]
        writer = WriterType.New()
        writer.SetFileName(vn)
        writer.SetInput(img)

        writer.Update()
    
    #
    # Make masks with portal trunk using liver mask + vessels mask
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
    
    #
    # Make vesselsNeighbourhood mask masked by liver mask (not the fused one but the liver one, important)
    #

    # dilated volumes
    """
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
    
    # thresholding values
    
    
    volume = itk.imread(vesselsSizeEstimation)
    spacing = volume.GetSpacing()
    npVolume = itk.array_from_image(volume)

    npVol = npVolume.astype(np.float32) / spacing[0]

    i=1
    j=0
    print(bornSet)
    l = [vesselsMaskSmall,vesselsMaskMedium,vesselsMaskLarge]
    for bMin,bMax in [(0.1,3),(3.1,6),(6.1,100)]:
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
    """
    commandLine = "./MakeIrcadBifurcationGT " + vesselsCutOutPath + " " + liverOutPath  + " " + bifurcationsOutPath + " " + vesselsSkeleton
    
    print(commandLine)
    os.system(commandLine)
    
    exit()
    """
    
    #bifurcationInPath = patientDirectory+"/bifurcationsMaskIso.nii"
    #liverInPath = patientDirectory+ "/liverMaskIso.nii"
    #bifurcationsOutPath = "/DATA/ircad_iso/"+patientDirectory.split("/")[3]+"/bifurcationsMaskIso.nii"
    #commandLine = "./isoFromUnit " + bifurcationInPath + " " + liverInPath + " " + bifurcationsOutPath
    #print(commandLine)
    #os.system(commandLine)
    
    
