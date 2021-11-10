#include <string>
#include <itkRescaleIntensityImageFilter.h>
#include "utils.h"


#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkHessian3DToVesselnessMeasureImageFilter.h"
#include "itkMultiScaleHessianBasedMeasureImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkMaskImageFilter.h"

#include "CLI11.hpp"


constexpr unsigned int Dimension = 3;
using PixelType = double;
using ImageType = itk::Image< PixelType, Dimension >;

using MaskPixelType = uint8_t;
using MaskImageType = itk::Image<MaskPixelType, Dimension>;

int main(int argc, char** argv)
{
      // parse command line using CLI ----------------------------------------------
  CLI::App app;
  app.description("Apply intensity rescale");
  std::string inputFile ;
  std::string outputFile;
  bool isInputDicom = false;
  
  std::string maskFile;
  app.add_option("--output,-o",outputFile, "ouputName : output img");
  app.add_option("-i,--input,1", inputFile, "inputName : input img" )
  ->required()
  ->check(CLI::ExistingFile);
  
  app.add_flag("--inputIsDicom,-d",isInputDicom ,"specify dicom input");
  app.add_option("--mask,-k",maskFile,"mask response by image")
  ->check(CLI::ExistingFile);
  
  app.get_formatter()->column_width(40);
  CLI11_PARSE(app, argc, argv);

  // END parse command line using CLI ----------------------------------------------
  
  // load image
  auto image = vUtils::readImage<ImageType>(inputFile,isInputDicom);
  // rescale
  auto rescaleFilter = itk::RescaleIntensityImageFilter<ImageType,ImageType>::New();

  rescaleFilter->SetInput(image);
  rescaleFilter->SetOutputMinimum(0.0f);
  rescaleFilter->SetOutputMaximum(1.0f);
  
  // mask if necessary
  ImageType::Pointer resultImage;
  typedef itk::Image<uint8_t, Dimension> MaskImageType;
  MaskImageType::Pointer maskImage;
  if( !maskFile.empty() )
  {
    maskImage = vUtils::readImage<MaskImageType>(maskFile,isInputDicom);
    
    auto maskFilter = itk::MaskImageFilter<ImageType,MaskImageType>::New();
    maskFilter->SetInput( rescaleFilter->GetOutput() );
    maskFilter->SetMaskImage(maskImage);
    maskFilter->SetMaskingValue(0);
    maskFilter->SetOutsideValue(0);
    
    maskFilter->Update();

    resultImage = maskFilter->GetOutput();
  }
  else{
    resultImage->Update();
    resultImage = rescaleFilter->GetOutput();
  }

  using imageWriterType = itk::Image<PixelType,Dimension>;
  typedef  itk::ImageFileWriter< imageWriterType  > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput( resultImage );
  writer->SetFileName( std::string(outputFile) );
  writer->Update();

  return 0;
}
