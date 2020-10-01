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
  //multiScaleEnhancementFilter->SetSigmaStepMethodToLogarithmic();
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
