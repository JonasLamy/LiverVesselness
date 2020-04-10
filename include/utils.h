#ifndef _UTILS_H
#define _UTILS_H

#include <string>

#include "itkImage.h"
#include "itkImageIOBase.h"

#include "itkImageFileReader.h"

#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageSeriesReader.h"
#include "itkImageSeriesWriter.h"

#include "itkIdentityTransform.h"
#include "itkBSplineInterpolateImageFunction.h"
#include "itkResampleImageFilter.h"

#include <vector>
#include <sys/stat.h>
#include <stdint.h>

namespace vUtils{

    bool isDir(std::string fileName) {
        struct stat buf;
        stat(fileName.c_str(), &buf);
        return S_ISDIR(buf.st_mode);
    }

    template <typename TImageType>
    typename TImageType::Pointer readImage(std::string fileName, bool isDicom)
    {
        if(isDicom)
        {
            using SeriesReaderType = itk::ImageSeriesReader< TImageType >;
            using NameGeneratorType = itk::GDCMSeriesFileNames;
            using SeriesIdContainer = std::vector<std::string>;

            NameGeneratorType::Pointer nameGenerator = NameGeneratorType::New();
            nameGenerator->SetUseSeriesDetails(true);
            nameGenerator->AddSeriesRestriction("0008|0021");
            nameGenerator->SetGlobalWarningDisplay(false);
            nameGenerator->SetDirectory(fileName);

            try
            {
                using SeriesIdContainer = std::vector<std::string>;
                const SeriesIdContainer & seriesUID = nameGenerator->GetSeriesUIDs();
                auto seriesItr = seriesUID.begin();
                auto seriesEnd = seriesUID.end();

                if( seriesItr == seriesEnd)
                {
                    std::cout<< "No Dicoms in :"<<fileName <<"..."<< std::endl;
                }

                
                std::string seriesIdentifier;
                seriesIdentifier = seriesItr->c_str();

                using FileNamesContainer = std::vector<std::string>;
                FileNamesContainer fileNames = nameGenerator->GetFileNames(seriesIdentifier);   

                using ReaderType = itk::ImageSeriesReader<TImageType>;
                typename ReaderType::Pointer reader = ReaderType::New();
                using ImageIOType = itk::GDCMImageIO;
                ImageIOType::Pointer dicomIO = ImageIOType::New();
                reader->SetImageIO(dicomIO);
                reader->SetFileNames(fileNames);
                reader->ForceOrthogonalDirectionOff();// properly read CTs with gantry tilt
                reader->Update();
                return reader->GetOutput();
            }
            catch( itk::ExceptionObject & ex)
            {
                std::cout<< ex << std::endl;
            }
        }

        typedef itk::ImageFileReader< TImageType > ImageReaderType;
        auto reader = ImageReaderType::New();
        reader->SetFileName(fileName);
        reader->Update();
        return reader->GetOutput();
    }

    template<typename TImageType>
    typename TImageType::Pointer makeIso(typename TImageType::Pointer inputImage,bool isMask, bool identitySpacing )
    {
        typename TImageType::SizeType size = inputImage->GetLargestPossibleRegion().GetSize();
        typename TImageType::SpacingType spacing = inputImage->GetSpacing();

        typename TImageType::SpacingType newSpacing;
        double minSpacing = std::min( spacing[0],std::min(spacing[1],spacing[2]) );

        if(identitySpacing)
        {
            newSpacing[0] = 1;
            newSpacing[1] = 1;
            newSpacing[2] = 1;
        }
        else
        {
            newSpacing[0] = 0.56;//minSpacing;
            newSpacing[1] = 0.56;//minSpacing;
            newSpacing[2] = 0.56;//minSpacing;
        }

        std::cout<<"new spacing:"<<newSpacing<<std::endl;
        typename TImageType::SizeType newSize;
        newSize[0] = long(size[0] * spacing[0]) / newSpacing[0] + 1;
        newSize[1] = long(size[1] * spacing[1]) / newSpacing[1] + 1;
        newSize[2] = long(size[2] * spacing[2]) / newSpacing[2] + 1;

        auto idTransform = itk::IdentityTransform<double,3>::New();
        auto interpolator = itk::BSplineInterpolateImageFunction<TImageType>::New();

        if( isMask)
        {
            interpolator->SetSplineOrder(0);
        }
        else
        {
            interpolator->SetSplineOrder(3);
        }
        
        auto resampler = itk::ResampleImageFilter<TImageType,TImageType>::New();
        resampler->SetInput(inputImage);
        resampler->SetTransform(idTransform);
        resampler->SetInterpolator(interpolator);
        resampler->SetSize(newSize);
        resampler->SetOutputSpacing(newSpacing);
        resampler->SetOutputOrigin(inputImage->GetOrigin());
        resampler->Update();

        return resampler->GetOutput();
    }
}

#include "utils.hxx"

#endif