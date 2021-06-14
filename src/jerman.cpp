#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkMultiScaleHessianBasedMeasureImageFilter.h"

#include "itkHessianToJermanMeasureImageFilter.h"
//#include "itkHessianToMeasureImageFilter.h"

#include "itkStatisticsImageFilter.h"

#include "CLI11.hpp"

#include <string>

#include "utils.h"

int main( int argc, char* argv[] )
{
  // parse command line using CLI ----------------------------------------------
  CLI::App app;
  app.description("Apply the Meijering algorithm");
  std::string inputFile ;
  std::string outputFile;
  double sigmaMin;
  double sigmaMax;
  float tau {0.75};
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
  app.add_option("--tau,-t", tau, "Jerman tau parameter" ,true);
  
  app.add_flag("--inputIsDicom,-d",isInputDicom ,"specify dicom input");
  app.add_option("--mask,-k",maskFile,"mask response by image")
  ->check(CLI::ExistingFile);
  
  app.get_formatter()->column_width(40);
  CLI11_PARSE(app, argc, argv);
  // END parse command line using CLI ----------------------------------------------
  
  
  constexpr unsigned int Dimension = 3;
  using PixelType = double;
  using ImageType = itk::Image< PixelType, Dimension >;
  
  using MaskPixelType = uint8_t;
  using MaskImageType = itk::Image<MaskPixelType, Dimension>;
  ImageType::Pointer image = vUtils::readImage<ImageType>(inputFile,isInputDicom);
  MaskImageType::Pointer maskImage;
  
  using HessianPixelType = itk::SymmetricSecondRankTensor< double, Dimension >;
  using HessianImageType = itk::Image< HessianPixelType, Dimension >;
  
  using OutputImageType = itk::Image< double, Dimension >;
  
  using JermanFilterType = itk::HessianToJermanMeasureImageFilter<HessianImageType, OutputImageType, MaskImageType>;
  auto jermanFilter = JermanFilterType::New();
  jermanFilter->SetTau(tau);
  
  if( !maskFile.empty() )
  {
    maskImage = vUtils::readImage<MaskImageType>(maskFile,isInputDicom);
    jermanFilter->SetMaskImage(maskImage);
  }
  
  
  using MultiScaleEnhancementFilterType = itk::MultiScaleHessianBasedMeasureImageFilter< ImageType, HessianImageType, OutputImageType >;
  MultiScaleEnhancementFilterType::Pointer multiScaleEnhancementFilter =  MultiScaleEnhancementFilterType::New();
  multiScaleEnhancementFilter->SetInput( image );
  multiScaleEnhancementFilter->SetHessianToMeasureFilter( jermanFilter );
  multiScaleEnhancementFilter->SetSigmaStepMethodToLogarithmic();
  multiScaleEnhancementFilter->SetSigmaMinimum( sigmaMin );
  multiScaleEnhancementFilter->SetSigmaMaximum( sigmaMax );
  multiScaleEnhancementFilter->SetNumberOfSigmaSteps( nbSigmaSteps );
  
  auto stats = itk::StatisticsImageFilter<OutputImageType>::New();
  stats->SetInput(multiScaleEnhancementFilter->GetOutput());
  stats->Update();
  
  std::cout<<"multiscale"<<std::endl<<"min"
  <<stats->GetMinimum()<<std::endl
  <<"mean:"<<stats->GetMean()<<std::endl
  <<"max:"<<stats->GetMaximum()<<std::endl;
  
  using imageWriterType = OutputImageType;
  typedef  itk::ImageFileWriter< imageWriterType  > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput( multiScaleEnhancementFilter->GetOutput() );
  writer->SetFileName( std::string(outputFile) );
  writer->Update();
  
  
  return EXIT_SUCCESS;
}
