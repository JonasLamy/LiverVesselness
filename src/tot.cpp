/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
 
//  Software Guide : BeginLatex
//
//  This example illustrates how to read a DICOM series into a volume and then
//  save this volume into another DICOM series using the exact same header
//  information. It makes use of the GDCM library.
//
//  The main purpose of this example is to show how to properly propagate the
//  DICOM specific information along the pipeline to be able to correctly write
//  back the image using the information from the input DICOM files.
//
//  Please note that writing DICOM files is quite a delicate operation since we
//  are dealing with a significant amount of patient specific data. It is your
//  responsibility to verify that the DICOM headers generated from this code
//  are not introducing risks in the diagnosis or treatment of patients. It is
//  as well your responsibility to make sure that the privacy of the patient is
//  respected when you process data sets that contain personal information.
//  Privacy issues are regulated in the United States by the HIPAA
//  norms\footnote{The Health Insurance Portability and Accountability Act of
//  1996. \url{http://www.cms.hhs.gov/hipaa/}}. You would probably find similar
//  legislation in every country.
//
//  \index{HIPAA!Privacy}
//  \index{HIPAA!Dicom}
//  \index{Dicom!HIPPA}
//
//  When saving datasets in DICOM format it must be made clear whether these
//  datasets have been processed in any way, and if so, you should inform the
//  recipients of the data about the purpose and potential consequences of the
//  processing. This is fundamental if the datasets are intended to be used for
//  diagnosis, treatment or follow-up of patients. For example, the simple
//  reduction of a dataset from a 16-bits/pixel to a 8-bits/pixel
//  representation may make it impossible to detect certain pathologies and
//  as a result will expose the patient to the risk of remaining untreated for a
//  long period of time while her/his pathology progresses.
//
//  You are strongly encouraged to get familiar with the report on medical
//  errors ``To Err is Human'', produced by the U.S. Institute of
//  Medicine~\cite{ToErrIsHuman2001}. Raising awareness about the high
//  frequency of medical errors is a first step in reducing their occurrence.
//
//  \index{Medical Errors}
//
//  Software Guide : EndLatex
 
// Software Guide : BeginLatex
//
// After all these warnings, let us now go back to the code and get familiar
// with the use of ITK and GDCM for writing DICOM Series. The first step that
// we must take is to include the header files of the relevant classes. We
// include the GDCMImageIO class, the GDCM filenames generator, as well as
// the series reader and writer.
//
// Software Guide : EndLatex
 
// Software Guide : BeginCodeSnippet
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageSeriesReader.h"
#include "itkImageSeriesWriter.h"

#include "itkChangeLabelImageFilter.h"
// Software Guide : EndCodeSnippet
 
#include <vector>
#include "itksys/SystemTools.hxx"
 
int
main(int argc, char * argv[])
{
  if (argc < 3)
  {
    std::cerr << "Usage: " << argv[0] << " DicomDirectory  OutputDicomDirectory"
              << std::endl;
    return EXIT_FAILURE;
  }
  using PixelType = uint8_t;
  constexpr unsigned int Dimension = 3;
 
  using ImageType = itk::Image<PixelType, Dimension>;
  using ReaderType = itk::ImageSeriesReader<ImageType>;
  using ImageIOType = itk::GDCMImageIO;
  using NamesGeneratorType = itk::GDCMSeriesFileNames;
 
  ImageIOType::Pointer        gdcmIO = ImageIOType::New();

  NamesGeneratorType::Pointer namesGenerator = NamesGeneratorType::New();
  namesGenerator->SetInputDirectory(argv[1]);
  const ReaderType::FileNamesContainer & filenames = namesGenerator->GetInputFileNames();

  std::size_t numberOfFileNames = filenames.size();
  std::cout << numberOfFileNames << std::endl;
  for (unsigned int fni = 0; fni < numberOfFileNames; ++fni)
  {
    std::cout << "filename # " << fni << " = ";
    std::cout << filenames[fni] << std::endl;
  }
 
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetImageIO(gdcmIO);
  reader->SetFileNames(filenames);
  
  try
  {
    reader->Update();
  }
  catch (itk::ExceptionObject & excp)
  {
    std::cerr << "Exception thrown while writing the image" << std::endl;
    std::cerr << excp << std::endl;
    return EXIT_FAILURE;
  }

  // Applying filters

  using ChangeLabelImageFilterType = itk::ChangeLabelImageFilter<ImageType,ImageType>;
  auto changeLabelImageFilter = ChangeLabelImageFilterType::New();

  changeLabelImageFilter->SetChange(1,255);
  changeLabelImageFilter->SetInput(reader->GetOutput());
  changeLabelImageFilter->Update();
  auto imgOutput = changeLabelImageFilter->GetOutput();
 // end applying filters

  const char * outputDirectory = argv[2];
  itksys::SystemTools::MakeDirectory(outputDirectory);

  using OutputPixelType = uint8_t;
  constexpr unsigned int OutputDimension = 2;
  using Image2DType = itk::Image<OutputPixelType, OutputDimension>;
  using SeriesWriterType = itk::ImageSeriesWriter<ImageType, Image2DType>;
  
  SeriesWriterType::Pointer seriesWriter = SeriesWriterType::New();
 
  seriesWriter->SetInput(imgOutput);
  seriesWriter->SetImageIO(gdcmIO);
  namesGenerator->SetOutputDirectory(outputDirectory);
 
  seriesWriter->SetFileNames(namesGenerator->GetOutputFileNames());

  const ReaderType::FileNamesContainer & filenamesOut = namesGenerator->GetOutputFileNames();

  numberOfFileNames = filenamesOut.size();
  std::cout << numberOfFileNames << std::endl;
  for (unsigned int fni = 0; fni < numberOfFileNames; ++fni)
  {
    std::cout << "filenameOut # " << fni << " = ";
    std::cout << filenamesOut[fni] << std::endl;
  }
  
 
  seriesWriter->SetMetaDataDictionaryArray(reader->GetMetaDataDictionaryArray());
  try
  {
    seriesWriter->Update();
  }
  catch (itk::ExceptionObject & excp)
  {
    std::cerr << "Exception thrown while writing the series " << std::endl;
    std::cerr << excp << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
