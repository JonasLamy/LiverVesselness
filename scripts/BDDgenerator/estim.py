import itk
import numpy as np
import sys
import matplotlib.pyplot as plt

import fnmatch
import os
from sklearn import mixture
import scipy.stats as stats
from scipy.optimize import curve_fit


import vUtils

def histogram(img,nbBins):
    minV = np.min(img)
    maxV = np.max(img)

    return np.histogram(img, int(maxV-minV/nbBins) ) 

def formatHistogram(histogram):

    histo_values = histogram[0]
    histo_bins = histogram[1][:-1] # without last bin
    return (histo_values,histo_bins)


def weighted_avg_and_std(values, weights):
    # https://stackoverflow.com/questions/2413522/weighted-standard-deviation-in-numpy
    """
    Return the weighted average and standard deviation.

    values, weights -- Numpy ndarrays with the same shape.
    """
    average = np.average(values, weights=weights)
    # Fast and numerically precise:
    variance = np.average((values-average)**2, weights=weights)
    return (average, np.sqrt(variance))

def gauss(x,a,mu,sigma):
    return a*np.exp(-(x-mu)**2 /(2*sigma**2))


def IrcadHistograms(inputDir):
    with open('gaussianModels.csv','w') as file:
        file.write("patient,liver_a,liver_mu,liver_sigma,vessels_a,vessels_mu,vessels_sigma\n")

        # reading ircad database
        count = 0
        for patientDirectory in fnmatch.filter( os.listdir(inputDir),'3D*'):

            print(patientDirectory)

            # input volume
            pathToImage = inputDir +"/"+patientDirectory+ "/patientIso.nii"
            itk_img = vUtils.readData(pathToImage,False)
            img = itk.GetArrayFromImage(itk_img)
            
            # liver mask
            pathToLiverMaskImage = inputDir +"/"+patientDirectory+ "/liverMaskIso.nii"
            itk_liverMaskImg = vUtils.readData(pathToLiverMaskImage,False)
            liverMaskImg = itk.GetArrayFromImage(itk_liverMaskImg)

            pathToVesselsMask = inputDir +"/"+patientDirectory+ "/vesselsIso.nii"
            itk_imgMaskVessels = vUtils.readData(pathToVesselsMask,False)
            imgMaskVessels = itk.GetArrayFromImage(itk_imgMaskVessels)

            # make histogram, no filters
            # what we want are two masks, 
            # the histogram inside the whole liver (take vessels in). the histogram inside the vessels

            binSize = 10

            # histogram whole liver
            wholeLiver = (liverMaskImg > 0) & (imgMaskVessels == 0)
            histo = histogram( img[wholeLiver], binSize )
            histo_plot = formatHistogram(histo)
            
            pOrigin=[408983,100,18]
            popt_liver,pcov = curve_fit(gauss,xdata=histo_plot[1],ydata=histo_plot[0],p0=pOrigin)

            print(popt_liver)
            print(pcov)        
            
            #histogram vessels

            vessels = (imgMaskVessels > 0) & (liverMaskImg > 0)
            histo_vessels = histogram( img[vessels], binSize )
            histo_vessels_plot = formatHistogram(histo_vessels)

            pOrigin=[2688,65,10]
            popt_vessels,pcov = curve_fit(gauss,xdata=histo_vessels_plot[1],ydata=histo_vessels_plot[0],p0=pOrigin)

            print(popt_vessels)
            print(pcov)

            #plot liver
            plt.bar(histo_plot[1],histo_plot[0])
            plt.plot( histo_plot[1], gauss(histo_plot[1],*popt_liver ),color="b" )
            #plot vessels
            plt.bar(histo_vessels_plot[1],histo_vessels_plot[0],color='r')
            plt.plot( histo_plot[1], gauss(histo_plot[1],*popt_vessels ),color="r" )

            plt.title(patientDirectory)
            plt.xlabel("bins")
            plt.ylabel("nb voxels")
            plt.savefig("images_histogram/"+patientDirectory+ ".svg",dpi=300)
            plt.close()

            """
            count +=1
            if(count >= 4):
                break
            """
            
            file.write( patientDirectory +","+ 
            str( popt_liver[0] ) +","+ 
            str( popt_liver[1] ) +","+ 
            str( popt_liver[2] ) +","+ 
            str( popt_vessels[0] ) +","+
            str( popt_vessels[1] ) +","+ 
            str( abs(popt_vessels[2]) ) ) # Warning ABS here because fitting returns negative values...
            
            file.write('\n')

def mriHistograms(inputDir):
    with open('gaussianModels.csv','w') as file:
        file.write("patient,liver_a,liver_mu,liver_sigma,vessels_a,vessels_mu,vessels_sigma\n")

        # reading ircad database
        count = 0
        for patientDirectory in fnmatch.filter( os.listdir(inputDir),'12*'):
            if patientDirectory not in ["12191","12192","12193","12194","12195"]:
                continue
            print(patientDirectory)
            # input volume
            pathToImage = inputDir +"/"+patientDirectory+ "/DICOM/EXP00000"
            itk_img = vUtils.readData(pathToImage,True)
            img = itk.GetArrayFromImage(itk_img)
            
            # mask
            pathToLiverMaskImage = "/DATA/masks_liver/"+patientDirectory+ "_seg.nii"
            itk_MaskImg = vUtils.readData(pathToLiverMaskImage,False)
            maskImg = itk.GetArrayFromImage(itk_MaskImg)

            # make histogram, no filters
            # what we want are two masks, 
            # the histogram inside the whole liver (take vessels in). the histogram inside the vessels

            binSize = 10

            # histogram whole liver
            wholeLiver = (maskImg == 1) 
            histo = histogram( img[wholeLiver], binSize )
            histo_plot = formatHistogram(histo)
            
            a = np.max(histo_plot[0])
            mean,std = weighted_avg_and_std(histo_plot[1],histo_plot[0])

            pOrigin=[a,mean,std]
            popt_liver,pcov = curve_fit(gauss,xdata=histo_plot[1],ydata=histo_plot[0],p0=pOrigin)

            print(popt_liver)
            print(pcov)        
            
            #histogram vessels

            vessels = (maskImg == 2)
            histo_vessels = histogram( img[vessels], binSize )
            histo_vessels_plot = formatHistogram(histo_vessels)

            a = np.max(histo_vessels_plot[0])
            mean,std = weighted_avg_and_std(histo_vessels_plot[1],histo_vessels_plot[0])

            pOrigin=[a,mean,std]
            popt_vessels,pcov = curve_fit(gauss,xdata=histo_vessels_plot[1],ydata=histo_vessels_plot[0],p0=pOrigin)#

            print(popt_vessels)
            print(pcov)

            #plot liver
            plt.bar(histo_plot[1],histo_plot[0])
            plt.plot( histo_plot[1], gauss(histo_plot[1],*popt_liver ),color="b" )
            #plot vessels
            plt.bar(histo_vessels_plot[1],histo_vessels_plot[0],color='r')
            plt.plot( histo_plot[1], gauss(histo_plot[1],*popt_vessels ),color="r" )

            plt.title(patientDirectory)
            plt.xlabel("bins")
            plt.ylabel("nb voxels")
            plt.savefig("images_histogram/"+patientDirectory+ ".svg",dpi=300)
            plt.close()
            
            
            file.write( patientDirectory +","+ 
            str( popt_liver[0] ) +","+ 
            str( popt_liver[1] ) +","+ 
            str( popt_liver[2] ) +","+ 
            str( popt_vessels[0] ) +","+
            str( popt_vessels[1] ) +","+ 
            str( abs(popt_vessels[2]) ) ) # Warning ABS here because fitting returns negative values...
            
            file.write('\n')


def makeMasks(inputDir):
    
    # reading ircad database
    count = 0
    for patientDirectory in fnmatch.filter( os.listdir(inputDir),'3D*'):
        print(patientDirectory)

        # input volume
        pathToImage = inputDir +"/"+patientDirectory+ "/vesselsIso.nii"
        itk_img = vUtils.readData(pathToImage,False)
        img = itk.GetArrayFromImage(itk_img)

        print( itk_img.GetSpacing() )
           
inputDir = "/DATA/ircad_iso_v2"
inputMRIDir = "/DATA/mriRvesselX"

#IrcadHistograms(inputDir)
#mriHistograms(inputMRIDir)
makeMasks(inputDir)
