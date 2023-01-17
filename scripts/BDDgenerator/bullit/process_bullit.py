"""
process_bullit.py

This script is used to create all ground truth of the bullit dataset. 
Usage :
python3 process_bullit.py BASE_DIR OUTPUT_DIR IDENTITY_SPACING
    BASE_DIR : original bullitt dataset location
    OUTPUT_DIR : Destination directory
    IDENTITY_SPACING : either 0 or 1. Define if the isotropic spacing is [1mm,1mm,1mm] or based on volume maximum resolution [maxRes,maxRes,maxRes].

The steps involve :

"""
import sys
import os
import glob
from os.path import exists
import shutil
import copy

import itk
from itk import TubeTK as ttk
import numpy as np
from scipy import ndimage
import skimage as sk


import matplotlib.pyplot as plt

from vascuSynth import Generator

inputDir = sys.argv[1]
outputDir = sys.argv[2]
IdentitySpacing = sys.argv[3]

# ----------------------------
# Reading the histogram 
# ----------------------------
sizeHistogram = dict()
for patientDirectory in glob.glob(inputDir +'/Normal*'):
    volumeName = patientDirectory.rsplit('/')[-1]
    print(volumeName)
    outputFilePath = patientDirectory.split("/Normal")[0]
    
    mraImagePath = patientDirectory + "/data.nii"
    vesselTreePath = patientDirectory + "/VascularNetwork.tre"
    
    # reading tubes from tubeTK
    # Read tre file
    Dimension = 3
    TubeFileReaderType = itk.SpatialObjectReader[Dimension]
        
    tubeFileReader = TubeFileReaderType.New()
    tubeFileReader.SetFileName(vesselTreePath)
    tubeFileReader.Update()

    tubes = tubeFileReader.GetGroup()

    print("Number of objects = ",tubes.GetNumberOfChildren() )
    sobj = tubes.GetChildren(0,"VesselTube")
    for tubeId in range(len(sobj) ):
        tube = itk.down_cast( sobj[tubeId] )
        tubePoints = tube.GetPoints()
        numPoints = len(tubePoints)

        for pId in range(numPoints):
            radius = tubePoints[pId].GetRadiusInObjectSpace()
            
            radiusStr = "{:10.3f}".format(radius) 
            roundedRadius = round(radius,3)

            if( roundedRadius in sizeHistogram ):
                sizeHistogram[roundedRadius] += 1
            else:
                sizeHistogram[roundedRadius] = 0

histogram = np.array( sorted( sizeHistogram.items() ) )
print(histogram)
plt.plot( histogram[:,0],histogram[:,1] )
plt.title("- number of points over radius in the file .tre -")
plt.ylabel("number of pixels")
plt.xlabel("radius (Radius In Object Space)")
plt.show()

# shape of the directory -> Bullit/Normal###-MRA/ ( data.nii | VascularNetwork.tre | brainMask.nii )
# pipeline data.nii -> dataCropped.nii.gz -> dataIso.nii.gz 
# since Isometric transformation increase the size of the volume, we crop the volume using the mask bounding box
nbVolumes=0
meanRatio = 0


for patientDirectory in glob.glob(inputDir +'/Normal*'):
    nbVolumes += 1

    volumeName = patientDirectory.rsplit('/')[-1]
    outputFilePath = patientDirectory.split("/Normal")[0]
    
    mraImagePath = patientDirectory + "/data.nii"
    vesselTreePath = patientDirectory + "/VascularNetwork.tre"

    #if(exists(outputDir + f"/{volumeName}" + "/labeledVessels.nii.gz") ):
    #    continue
    
    # Producing vessels ground truth
    # from VascularNetwork.tre
    
    Dimension = 3

    PixelType = itk.ctype("unsigned short")
    ImageType = itk.Image[PixelType, Dimension]

    MaskPixelType = itk.UC
    MaskImageType = itk.Image[MaskPixelType, Dimension]
        
    # Read tre file
    TubeFileReaderType = itk.SpatialObjectReader[Dimension]
        
    tubeFileReader = TubeFileReaderType.New()
    tubeFileReader.SetFileName(vesselTreePath)
    tubeFileReader.Update()

    tubes = tubeFileReader.GetGroup()


    # Read template image
    TemplateImageType = itk.Image[PixelType, Dimension]
    TemplateImageReaderType = itk.ImageFileReader[TemplateImageType]
        
    templateImageReader = TemplateImageReaderType.New()
    templateImageReader.SetFileName(mraImagePath)
    templateImageReader.Update()

    templateImage = templateImageReader.GetOutput()

    print("converting image to tubes")
    VesselsFilterType = ttk.ConvertTubesToImage[TemplateImageType]
    VesselsFilter = VesselsFilterType.New()
    VesselsFilter.SetUseRadius(True)
    VesselsFilter.SetTemplateImage(templateImageReader.GetOutput())
    VesselsFilter.SetInput(tubes)
    VesselsFilter.Update()

    vesselsImage = VesselsFilter.GetOutput()

    skeletonFilterType = ttk.ConvertTubesToImage[TemplateImageType]
    skeletonFilter = skeletonFilterType.New()
    skeletonFilter.SetUseRadius(False)
    skeletonFilter.SetTemplateImage(templateImageReader.GetOutput())
    skeletonFilter.SetInput(tubes)
    skeletonFilter.Update()

    skeletonImage = skeletonFilter.GetOutput()
    print("done")
     
    outputDirPath = outputDir + f"/{volumeName}"
    croppedDir = outputDir + f"/{volumeName}/cropped" 

    if not os.path.exists( croppedDir ):
        os.makedirs(croppedDir)
    
    # DATA
    dataPath             = outputDir + f"/{volumeName}" + "/data.nii"
    dataCroppedPath      = croppedDir + "/dataCropped.nii.gz"
    dataIsoPath          = outputDir + f"/{volumeName}" + "/dataIso.nii.gz"
    dataGAPath           = outputDir + f"/{volumeName}" + "/dataGA.nii.gz"
    # VESSELS
    binaryVesselsPath           = outputDir + f"/{volumeName}" + "/binaryVessels.nii.gz"
    binaryVesselsErodedPath     = outputDir + f"/{volumeName}" + "/bvEroded.nii.gz"
    binaryVesselsCroppedPath    = croppedDir + "/binaryVesselsCropped.nii.gz"
    binaryVesselsIsoPath        = outputDir + f"/{volumeName}" + "/binaryVesselsIso.nii.gz"
    # VESSELS SANCHEZ
    bvSanchezPath           = outputDir + f"/{volumeName}" + "/gtSanchez.nii"
    bvSanchezErodedPath     = outputDir + f"/{volumeName}" + "/bvEroded_S.nii.gz"
    bvSanchezCroppedPath    = croppedDir + "/binaryVesselsCropped_S.nii.gz"
    bvSanchezIsoPath        = outputDir + f"/{volumeName}" + "/binaryVesselsIso_S.nii.gz"
    # LABELED VESSELS
    # TubeTk
    labeledVesselsPath      = outputDir + f"/{volumeName}" + "/labeledVessels.nii.gz"
    # Sanchez
    lvSanchezPath      = outputDir + f"/{volumeName}" + "/labeledVessels_S.nii.gz"
    # MASK
    maskImagePath           = outputDir + f"/{volumeName}" + "/brainMask.nii"
    maskErodedImagePath     = outputDir + f"/{volumeName}" + "/brainMaskEroded.nii.gz"
    maskImageCroppedPath    = croppedDir + "/brainMaskCropped.nii.gz"
    maskImageIsoPath        = outputDir + f"/{volumeName}" + "/brainMaskIso.nii.gz"

    originalCroppedMaskPath = croppedDir + "/originalBrainMaskCropped.nii.gz"
    originalMaskIsoPath     = outputDir + f"/{volumeName}" + "/originalBrainMaskIso.nii.gz"
    # SKELETON
    binarySkelPath          = outputDir + f"/{volumeName}" + "/skeleton.nii.gz"
    binarySkelErodedPath    = outputDir + f"/{volumeName}" + "/skelEroded.nii.gz"
    binarySkelCroppedPath   = croppedDir + "/skelCropped.nii.gz"
    binaryCleanedSkelPath   = outputDir + f"/{volumeName}" + "/skelClean.nii.gz"
    binarySkelIsoPath       = outputDir + f"/{volumeName}" + "/skelIso.nii.gz"
    
    # VESSELS MASK
    # TubeTK
    vesselsMaskSmallPath  = outputDir + f"/{volumeName}" + "/vesselsMaskSmall.nii.gz"
    vesselsMaskMediumPath = outputDir + f"/{volumeName}" + "/vesselsMaskMedium.nii.gz"
    vesselsMaskLargePath  = outputDir + f"/{volumeName}" + "/vesselsMaskLarge.nii.gz"
    # Sanchez
    vmsSanchezPath  = outputDir + f"/{volumeName}" + "/vesselsMaskSmall_S.nii.gz"
    vmmSanchezPath = outputDir + f"/{volumeName}" + "/vesselsMaskMedium_S.nii.gz"
    vmlSanchezPath  = outputDir + f"/{volumeName}" + "/vesselsMaskLarge_S.nii.gz"

    # DILATED VESSELS MASK
    # TubeTK
    dilatedVesselsMask = outputDir + f"/{volumeName}" + "/dilatedVesselsMask.nii.gz"
    dilatedVesselsMaskLarge  = outputDir + f"/{volumeName}" + "/dilatedVesselsMaskLarge.nii.gz"
    dilatedVesselsMaskMedium = outputDir + f"/{volumeName}" + "/dilatedVesselsMaskMedium.nii.gz"
    dilatedVesselsMaskSmall  = outputDir + f"/{volumeName}" + "/dilatedVesselsMaskSmall.nii.gz"
    # Sanchez
    dvmSanchez   = outputDir + f"/{volumeName}" + "/dilatedVesselsMask_S.nii.gz"
    dvmlSanchez  = outputDir + f"/{volumeName}" + "/dilatedVesselsMaskLarge_S.nii.gz"
    dvmmSanchez  = outputDir + f"/{volumeName}" + "/dilatedVesselsMaskMedium_S.nii.gz"
    dvmsSanchez  = outputDir + f"/{volumeName}" + "/dilatedVesselsMaskSmall_S.nii.gz"
    # BIFURCATIONS
    # TubeTK
    bifurcationMaskIso  = outputDir + f"/{volumeName}" + "/bifurcationMaskIso.nii.gz" 
    # Sanchez
    bmSanchezIso  = outputDir + f"/{volumeName}" + "/bifurcationMaskIso_S.nii.gz"
    # MASKED BRAIN
    maskedImageIsoPath  = outputDir + f"/{volumeName}" + "/maskedBrainIso.nii.gz"

    # Noise 
    noisyImagePath = outputDir + f"/{volumeName}" + "/rician"


    # histogram sanchez vs itk
    
    dataItk = itk.imread(dataPath)
    bvTubeTkItk = itk.imread(binaryVesselsPath)
    bvSanchezItk = itk.imread(bvSanchezPath)

    data      = itk.array_view_from_image( dataItk     )
    bvTubeTk  = itk.array_view_from_image( bvTubeTkItk )
    bvSanchez = itk.array_view_from_image( bvSanchezItk)
    
    nbPixelsTubeTK  = np.count_nonzero( bvTubeTk  )
    nbPixelsSanchez = np.count_nonzero( bvSanchez )

    print(volumeName)
    print("-- histograms --")
    print("nbPixels Sanchez:", nbPixelsSanchez )
    print("nbPixels:", nbPixelsTubeTK )
    print("ratio sanchez/tubes", nbPixelsSanchez/nbPixelsTubeTK)

    meanRatio += nbPixelsSanchez/nbPixelsTubeTK

    del data
    del bvTubeTk
    del bvSanchez

    if not os.path.exists( outputDirPath ):
        os.makedirs(outputDirPath)
    
    itk.imwrite(skeletonImage,binarySkelPath)
    itk.imwrite(vesselsImage,binaryVesselsPath)
    itk.imwrite(templateImage,dataPath)
    
    del skeletonImage
    del vesselsImage

    # ---- Processing brain mask -----
    print("loading mask")

    
    def scaleImg(img,scale):
        input_image = img
        input_size = itk.size(input_image)
        input_spacing = itk.spacing(input_image)
        input_origin = itk.origin(input_image)
        Dimension = input_image.GetImageDimension()

        # We will scale the objects in the image by the factor `scale`; that is they
        # will be shrunk (scale < 1.0) or enlarged (scale > 1.0).  However, the number
        # of pixels for each dimension of the output image will equal the corresponding
        # number of pixels in the input image, with cropping or padding as necessary.
        # Furthermore, the physical distance between adjacent pixels will be the same
        # in the input and the output images.  In contrast, if you want to change the
        # resolution of the image without changing the represented physical size of the
        # objects in the image, omit the transform and instead supply:
        #
        # output_size = [int(input_size[d] * scale) for d in range(Dimension)]
        # output_spacing = [input_spacing[d] / scale for d in range(Dimension)]
        # output_origin = [input_origin[d] + 0.5 * (output_spacing[d] - input_spacing[d])
        #                  for d in range(Dimension)]

        output_size = input_size
        output_spacing = input_spacing
        output_origin = input_origin
        scale_transform = itk.ScaleTransform[itk.D, Dimension].New()
        scale_transform_parameters = scale_transform.GetParameters()
        for i in range(len(scale_transform_parameters)):
            scale_transform_parameters[i] = scale
        scale_transform.SetParameters(scale_transform_parameters)
        
        scale_transform_center = [input_size[0]/4,input_size[1]/4,input_size[2]/4]
        scale_transform.SetCenter(scale_transform_center)

        interpolator = itk.LinearInterpolateImageFunction.New(input_image)

        resampled = itk.resample_image_filter(
            input_image,
            transform=scale_transform,
            interpolator=interpolator,
            size=output_size,
            output_spacing=output_spacing,
            output_origin=output_origin,
        )

        return resampled

    scale1 = 1.1
    scale2 = 1.2

    firstLayerItk  = scaleImg(itk_imgMask,scale1)
    secondLayerItk = scaleImg(itk_imgMask,scale2)

    itk.imwrite(secondLayerItk,"shiftDebug1.nii.gz")

    shift = 30
    for i in reversed( range(shift,secondLayerItk.shape[0]) ):
        secondLayerItk[i-shift,:,:] += secondLayerItk[i,:,:] 

    itk.imwrite(secondLayerItk,"shiftDebug2.nii.gz")

    maskingPart = ~( (itk.array_view_from_image(firstLayerItk)>0) | (itk.array_view_from_image(secondLayerItk)>0) )
    itk_imgMask[ maskingPart ] = 0
    itk.imwrite(itk_imgMask,maskErodedImagePath)
    
    def makeErodedVessels(binaryVesselsPath,binarySkelPath,maskErodedImagePath,binaryVesselsErodedPath,binarySkelErodedPath):
        bv_itk = itk.imread(binaryVesselsPath)
        bs_itk = itk.imread(binarySkelPath)
        itk_imgMask = itk.imread(maskErodedImagePath)

        bv = itk.array_view_from_image(bv_itk)
        bs = itk.array_view_from_image(bs_itk)
        m = itk.array_view_from_image(itk_imgMask)

        mask = m<1
        bv[mask] = 0
        bs[mask] = 0

        itk.imwrite(bv_itk,binaryVesselsErodedPath)
        itk.imwrite(bs_itk,binarySkelErodedPath)

    # Tube TK
    #makeErodedVessels(binaryVesselsPath,binarySkelPath,itk_imgMask)
    # Sanchez
    #makeErodedVessels(bvSanchezPath,binarySkelPath,maskErodedImagePath,bvSanchezErodedPath,binarySkelErodedPath)
    
    #exit()
    
    # - steps - 
    # clean skeleton
    # make clean skelton Iso
    # make bifurcations
    print(volumeName)               # instance ID
    print(dataIsoPath)              # data ISO 
    print(binaryVesselsIsoPath)     # gt ISO
    print(maskImageIsoPath)         # mask organ ISO
    print(dilatedVesselsMask)       # mask dilated vessels ISO
    print(bifurcationMaskIso)       # bifurcation ISO
    print(dilatedVesselsMaskMedium) # vessels mask medium
    print(dilatedVesselsMaskSmall)  # vessels mask small
    """
    # TubeTK GT
    """
    commandLine = (f"./MakeIsoBullit {dataPath} {dataCroppedPath} {dataIsoPath} "
                  f"{maskErodedImagePath} {maskImageCroppedPath} {maskImageIsoPath} "
                  f"{binaryVesselsErodedPath} {binaryVesselsCroppedPath} {binaryVesselsIsoPath} "
                  f"{binarySkelErodedPath} {binarySkelCroppedPath} {binarySkelIsoPath} "
                  f"{maskedImageIsoPath} 0")
    os.system(commandLine)
    """
    # Sanchez GT
    """
    commandLine = (f"./MakeIsoBullit {dataPath} {dataCroppedPath} {dataIsoPath} "
                  f"{maskImagePath} {originalCroppedMaskPath} {originalMaskIsoPath} "# f"{maskErodedImagePath} {maskImageCroppedPath} {maskImageIsoPath} "
                  f"{bvSanchezErodedPath} {bvSanchezCroppedPath} {bvSanchezIsoPath} "
                  f"{binarySkelErodedPath} {binarySkelCroppedPath} {binarySkelIsoPath} "
                  f"{maskedImageIsoPath} 0")
    os.system(commandLine)
 
    
    commandLine = (f"./criticalKernelsThinning3D --input {binarySkelIsoPath} --exportImage {binaryCleanedSkelPath} --select dmax --skel 1isthmus --persistence 1 -k"
                    )
    os.system(commandLine)

    # temporary fix because time is wasted otherwise....
    binSkel = itk.imread(binarySkelIsoPath)
    binCleanSkel = itk.imread(binaryCleanedSkelPath)
    binCleanSkel.SetSpacing( binSkel.GetSpacing() )
    binCleanSkel.SetOrigin( binSkel.GetOrigin() )
    itk.imwrite(binCleanSkel,binaryCleanedSkelPath)
    """
    # TubeTK GT
    """
    commandLine = (f"./MakeBullitBifurcationGT {binaryVesselsIsoPath} {binaryCleanedSkelPath} {maskImageIsoPath} {bifurcationMaskIso} ")
    print(commandLine)
    os.system(commandLine)
    
    commandLine = (f"./LabelVesselsBullit {binaryVesselsIsoPath} {binaryCleanedSkelPath} {maskImageIsoPath} labeledSkel.nii.gz {labeledVesselsPath}")
    print(commandLine)
    os.system(commandLine)
    """
    # Sanchez
    """
    commandLine = (f"./MakeBullitBifurcationGT {bvSanchezIsoPath} {binaryCleanedSkelPath} {maskImageIsoPath} {bmSanchezIso} ")
    print(commandLine)
    os.system(commandLine)
    
    commandLine = (f"./LabelVesselsBullit {bvSanchezIsoPath} {binaryCleanedSkelPath} {maskImageIsoPath} labeledSkel.nii.gz {lvSanchezPath}")
    print(commandLine)
    os.system(commandLine)

    # thresholding values
    def makeDilatedMasks(labeledVesselsPath,masksPaths,dilatedMasksPath,dilatedVesselsMaskPath,brainMask):
        volume = itk.imread(labeledVesselsPath)
        spacing = volume.GetSpacing()
        npVolume = itk.array_from_image(volume)

        npVol = npVolume.astype(np.float32) #/ spacing[0]

        i=1
        j=0
        l = masksPaths
        for bMin,bMax in [(0.1,1),(1.1,6),(6,100)]:
            #npVolume[ (npVol >= bMin) & (npVol <= bMax) ] = i

            vol = np.zeros(npVolume.shape,dtype=np.uint8)
            vol[(npVol >= bMin) & (npVol <= bMax)] = 255
            
            img = itk.image_from_array(vol)
            img.SetSpacing( volume.GetSpacing() )
            img.SetOrigin( volume.GetOrigin() )
            itk.imwrite(img,l[j])
            i+=1
            j+=1

        # Make dilated masks for vessels of different sizes
        
        sv = itk.imread(masksPaths[0]) # small
        mv = itk.imread(masksPaths[1]) # medium
        lv = itk.imread(masksPaths[2]) # large

        PixelType = itk.UC
        Dimension = 3
        ImageType = itk.Image[PixelType, Dimension]

        vessels = [lv,mv,sv]
        radius = [7,5,3]
        volumeNames = dilatedMasksPath

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

        dilatedVesselsImg = np.zeros( sv.shape )
        for vn,v,nv in zip(volumeNames,dilatedVessels,dilatedVesselsNumpy):
            nv[ brainMask == 0 ] = 0 

            dilatedVesselsImg += nv

            img = itk.image_from_array( nv.astype(np.uint8) )
            img.SetSpacing( v.GetSpacing() )
            img.SetOrigin( v.GetOrigin() )

            WriterType = itk.ImageFileWriter[ImageType]
            writer = WriterType.New()
            writer.SetFileName(vn)
            writer.SetInput(img)

            writer.Update()

        img = itk.image_from_array( dilatedVesselsImg.astype(np.uint8) )
        img.SetSpacing( sv.GetSpacing() )
        img.SetOrigin( sv.GetOrigin() )

        itk.imwrite(img,dilatedVesselsMaskPath)
    
    # TubeTK
    #masksPaths = [vesselsMaskSmallPath,vesselsMaskMediumPath,vesselsMaskLargePath]
    #dilatedMaskPaths = [dilatedVesselsMaskLarge,dilatedVesselsMaskMedium,dilatedVesselsMaskSmall] 
    #makeDilatedMasks(labeledVesselsPath,masksPaths,dilatedMaskPaths,dilatedVesselsMask)
    # Sanchez
    
    itk_imgMask = itk.imread( maskImageIsoPath ).astype(itk.UC)
    masksPaths = [vmsSanchezPath,vmmSanchezPath,vmlSanchezPath]
    dilatedMaskPaths = [dvmlSanchez,dvmmSanchez,dvmsSanchez] 
    makeDilatedMasks(lvSanchezPath,masksPaths,dilatedMaskPaths, dvmSanchez,itk.array_view_from_image(itk_imgMask)) 
    
    
    generator = Generator()

    sigmaMin = 80
    sigmaMax = 90
    nbGaussian = 1
    ponderationWeight = 0.8

    data = itk.imread(dataIsoPath)
    dataOrigin = data.GetOrigin()
    dataSpacing = data.GetSpacing()

    # clipping data
    maxIntensity = np.max(data)
    clippingIntensity = (0.8 * maxIntensity)
    
    print(maxIntensity, clippingIntensity)
    data[ data> clippingIntensity] = clippingIntensity

    itk_imgnbPixelsSanchezMask = itk.imread( maskImageIsoPath ).astype(itk.UC)
    mask = itk.array_view_from_image(itk_imgMask)

    generator.gaussianVariationsInMask(itk.array_view_from_image(data),dataGAPath,sigmaMin,sigmaMax,nbGaussian,ponderationWeight,dataOrigin,dataSpacing,mask)
    generator.noisyImage(dataGAPath,noisyImagePath,"rician")
    """

print("Mean ratio",meanRatio/nbVolumes)

    
    
