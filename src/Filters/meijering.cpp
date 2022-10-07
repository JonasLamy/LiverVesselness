#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkMultiScaleHessianBasedMeasureImageFilter.h"
#include "itkHessianToMeijeringMeasureImageFilter.h"

#include "itkStatisticsImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

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
  float alpha {-0.66};
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
  app.add_option("--alpha,-a", alpha, "Meijering's alpha" ,true);
  
  app.add_flag("--inputIsDicom,-d",isInputDicom ,"specify dicom input");
  app.add_option("--mask,-k",maskFile,"mask response by image")
  ->check(CLI::ExistingFile);
  
  app.get_formatter()->column_width(40);
  CLI11_PARSE(app, argc, argv);
  // END parse command line using CLI ----------------------------------------------
  
  
  constexpr unsigned int Dimension = 3;
  using PixelType = double;
  using ImageType = itk::Image< PixelType, Dimension >;
  
  using MaskImageType = itk::Image<uint8_t,Dimension>;
  
  ImageType::Pointer image = vUtils::readImage<ImageType>(inputFile,isInputDicom);
  MaskImageType::Pointer maskImage;
  
  using HessianPixelType = itk::SymmetricSecondRankTensor< double, Dimension >;
  using HessianImageType = itk::Image< HessianPixelType, Dimension >;
  using MaskImageType = itk::Image<uint8_t, Dimension>;
  
  using OutputImageType = itk::Image< double, Dimension >;
  
  using MeijeringFilterType = itk::HessianToMeijeringMeasureImageFilter<HessianImageType, OutputImageType,MaskImageType>;
  auto meijeringFilter = MeijeringFilterType::New();
  meijeringFilter->SetAlpha(alpha);
  
  if( !maskFile.empty() )
  {
    maskImage = vUtils::readImage<MaskImageType>(maskFile,isInputDicom);
    meijeringFilter->SetMaskImage(maskImage);
  }
  
  using MultiScaleEnhancementFilterType = itk::MultiScaleHessianBasedMeasureImageFilter< ImageType, HessianImageType, OutputImageType >;
  MultiScaleEnhancementFilterType::Pointer multiScaleEnhancementFilter =  MultiScaleEnhancementFilterType::New();
  multiScaleEnhancementFilter->SetInput( image );
  multiScaleEnhancementFilter->SetHessianToMeasureFilter( meijeringFilter );
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
  
  //using RescaleFilterType = itk::RescaleIntensityImageFilter< ImageType, ImageType >;
  //RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
  
 
  //rescaleFilter->SetInput(multiScaleEnhancementFilter->GetOutput());
  //rescaleFilter->SetOutputMinimum(0.0f);
  //rescaleFilter->SetOutputMaximum(1.0f);


  using imageWriterType = OutputImageType;
  typedef  itk::ImageFileWriter< imageWriterType  > WriterType;
  WriterType::Pointer writer = WriterType::New();
  //writer->SetInput( rescaleFilter->GetOutput() );
  writer->SetInput( multiScaleEnhancementFilter->GetOutput() );
  writer->SetFileName( std::string(outputFile) );
  writer->Update();
  
  
  return EXIT_SUCCESS;
}
