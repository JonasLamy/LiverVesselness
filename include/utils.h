#ifndef _UTILS_H
#define _UTILS_H

#include <string>

#include "itkImage.h"
#include "itkImageIOBase.h"

#include "itkImageFileReader.h"

#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageSeriesReader.h"

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
}

#endif