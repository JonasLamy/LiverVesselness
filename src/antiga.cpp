#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkHessianToObjectnessMeasureImageFilter.h"
#include "itkMultiScaleHessianBasedMeasureImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkMaskImageFilter.h"

#include "CLI11.hpp"

#include <string>

#include "utils.h"

int main( int argc, char* argv[] )
{
  // parse command line using CLI ----------------------------------------------
  CLI::App app;
  app.description("Apply the Antiga algorithm");
  std::string inputFile ;
  std::string outputFile;
  double sigmaMin;
  double sigmaMax;
  float alpha {0.5};
  float beta {0.5};
  float gamma {5.0};
  bool isInputDicom = false;
  int nbSigmaSteps;
  std::string maskFile;
  app.add_option("-i,--input,1", inputFile, "inputName : input img")
  ->required()
  ->check(CLI::ExistingFile);
  
  app.add_option("--output,-o",outputFile, "ouputName : output img");
  app.add_option("--sigmaMin,-m", sigmaMin, "scale space sigma min");
  app.add_option("--sigmaMax,-M", sigmaMax, "scale space sigma max");
  app.add_option("--nbSigmaSteps,-n",nbSigmaSteps,  "nb steps sigma");
  app.add_option("--alpha,-a", alpha, "Frangi's alpha" ,true);
  app.add_option("--beta,-b", beta, "Frangi's beta" ,true);
  app.add_option("--gamma", beta, "max vessel intensity" ,true);
  app.add_option("--sigmaMin,-m",sigmaMin,"scale space sigma min");
  app.add_option("--sigmaMax,-m",sigmaMax,"scale space sigma max");
  app.add_flag("--inputIsDicom,-d",isInputDicom ,"specify dicom input");
  app.add_option("--mask,-k",maskFile,"mask response by image")
  ->check(CLI::ExistingFile);
  
  app.get_formatter()->column_width(40);
  CLI11_PARSE(app, argc, argv);
  // END parse command line using CLI ----------------------------------------------
  
  
  constexpr unsigned int Dimension = 3;
  using PixelType = float;
  using ImageType = itk::Image< PixelType, Dimension >;
  
  ImageType::Pointer image = vUtils::readImage<ImageType>(inputFile,isInputDicom);
  
  // Antiga vesselness operator
  
  using HessianPixelType = itk::SymmetricSecondRankTensor< double, Dimension >;
  using HessianImageType = itk::Image< HessianPixelType, Dimension >;
  using ObjectnessFilterType = itk::HessianToObjectnessMeasureImageFilter< HessianImageType, ImageType >;
  
  ObjectnessFilterType::Pointer objectnessFilter = ObjectnessFilterType::New();
  objectnessFilter->SetBrightObject( true );
  objectnessFilter->SetScaleObjectnessMeasure( false );
  objectnessFilter->SetAlpha( alpha );
  objectnessFilter->SetBeta( beta );
  objectnessFilter->SetGamma( gamma );
  
  using MultiScaleEnhancementFilterType = itk::MultiScaleHessianBasedMeasureImageFilter< ImageType, HessianImageType, ImageType >;
  MultiScaleEnhancementFilterType::Pointer multiScaleEnhancementFilter =  MultiScaleEnhancementFilterType::New();
  multiScaleEnhancementFilter->SetInput( image );
  multiScaleEnhancementFilter->SetHessianToMeasureFilter( objectnessFilter );
  multiScaleEnhancementFilter->SetSigmaStepMethodToLogarithmic();
  multiScaleEnhancementFilter->SetSigmaMinimum( sigmaMin );
  multiScaleEnhancementFilter->SetSigmaMaximum( sigmaMax );
  multiScaleEnhancementFilter->SetNumberOfSigmaSteps( nbSigmaSteps );
  
  // For antiga, no global parameters (like jerman's lambda_max)
  // we still have to mask the answer though
  
  using imageWriterType = itk::Image<PixelType,Dimension>;
  typedef  itk::ImageFileWriter< imageWriterType  > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName( std::string(outputFile) );
  
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
    
    writer->SetInput( maskFilter->GetOutput() );
  }
  else
  {
    writer->SetInput( multiScaleEnhancementFilter->GetOutput() );
  }
  
  writer->Update();
  
  return EXIT_SUCCESS;
}
