#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkHessian3DToVesselnessMeasureImageFilter.h"
#include "itkMultiScaleHessianBasedMeasureImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkMaskImageFilter.h"

#include "CLI11.hpp"

#include <string>
#include <itkRescaleIntensityImageFilter.h>
#include "utils.h"

int main( int argc, char* argv[] )
{
  bool isInputDicom;
  
  // parse command line using CLI ----------------------------------------------
  CLI::App app;
  CLI::App app;
  app.description("Apply the Sato algorithm");
  std::string inputFile ;
  std::string outputFile;
  double sigmaMin;
  double sigmaMax;
  float alpha {0.5};
  float beta {0.5};
  
  int nbSigmaSteps;
  double fixedSigma;
  std::string maskFile;
  app.add_option("-i,--input,1", inputFile, "inputName : input img" )
  ->required()
  ->check(CLI::ExistingFile);
  
  app.add_option("--output,-o",outputFile, "ouputName : output img");
  app.add_option("--sigmaMin,-m", sigmaMin, "scale space sigma min");
  app.add_option("--sigmaMax,-M", sigmaMax, "scale space sigma max");
  app.add_option("--nbSigmaSteps,-n",nbSigmaSteps,  "nb steps sigma");
  app.add_option("--alpha1,-a", alpha, "Sato's alpha1" ,true);
  app.add_option("--alpha2,-b", beta, "Sato's alpha2" ,true);
  
  app.add_option("--sigma,-s",fixedSigma,"sigma for smoothing");
  app.add_flag("--inputIsDicom,-d",isInputDicom ,"specify dicom input");
  app.add_option("--mask,-k",maskFile,"mask response by image")
  ->check(CLI::ExistingFile);
  
  
  
  constexpr unsigned int Dimension = 3;
  using PixelType = float;
  using ImageType = itk::Image< PixelType, Dimension >;
  
  auto inputImage = vUtils::readImage<ImageType>(inputFile,isInputDicom);
  
  // Antiga vesselness operator
  
  using HessianPixelType = itk::SymmetricSecondRankTensor< double, Dimension >;
  using HessianImageType = itk::Image< HessianPixelType, Dimension >;
  using ObjectnessFilterType = itk::Hessian3DToVesselnessMeasureImageFilter<PixelType>;
  
  
  ObjectnessFilterType::Pointer objectnessFilter = ObjectnessFilterType::New();
  objectnessFilter->SetAlpha1( alpha );
  objectnessFilter->SetAlpha2( beta );
  
  using MultiScaleEnhancementFilterType = itk::MultiScaleHessianBasedMeasureImageFilter< ImageType, HessianImageType, ImageType >;
  MultiScaleEnhancementFilterType::Pointer multiScaleEnhancementFilter =  MultiScaleEnhancementFilterType::New();
  multiScaleEnhancementFilter->SetInput( inputImage );
  multiScaleEnhancementFilter->SetNonNegativeHessianBasedMeasure(true);
  multiScaleEnhancementFilter->SetHessianToMeasureFilter( objectnessFilter );
  multiScaleEnhancementFilter->SetSigmaStepMethodToLogarithmic();
  multiScaleEnhancementFilter->SetSigmaMinimum( sigmaMin );
  multiScaleEnhancementFilter->SetSigmaMaximum( sigmaMax );
  multiScaleEnhancementFilter->SetNumberOfSigmaSteps( nbSigmaSteps );
  
  // end Antiga vesselness operator
  using RescaleFilterType = itk::RescaleIntensityImageFilter< ImageType, ImageType >;
  RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
  
  typedef itk::Image<uint8_t, Dimension> MaskImageType;
  MaskImageType::Pointer maskImage;
  if( !maskFile.empty() )
  {
    maskImage = vUtils::readImage<MaskImageType>(maskFile,isInputDicom);
    
    auto maskFilter = itk::MaskImageFilter<ImageType,MaskImageType>::New();
    maskFilter->SetInput( multiScaleEnhancementFilter->GetOutput() );
    maskFilter->SetMaskImage(maskImage);
    maskFilter->SetMaskingValue(0);
    maskFilter->SetOutsideValue(0);
    
    maskFilter->Update();
    rescaleFilter->SetInput( maskFilter->GetOutput() );
  }
  else{
    rescaleFilter->SetInput(multiScaleEnhancementFilter->GetOutput());
  }
  
  using OutputImageType = ImageType;
  rescaleFilter->SetOutputMinimum(0.0f);
  rescaleFilter->SetOutputMaximum(1.0f);
  
  using imageWriterType = itk::Image<PixelType,Dimension>;
  typedef  itk::ImageFileWriter< imageWriterType  > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput( rescaleFilter->GetOutput() );
  writer->SetFileName( std::string(outputFile) );
  writer->Update();
  
  
  return EXIT_SUCCESS;
}
