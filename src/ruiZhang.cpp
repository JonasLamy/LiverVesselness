#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkHessianToRuiZhangMeasureImageFilter.h"
#include "itkMultiScaleHessianBasedMeasureImageFilter.h"
#include "itkSigmoidImageFilter.h"
#include "itkImageDuplicator.h"
#include "itkImageRegionIterator.h"

#include "itkStatisticsImageFilter.h"

#include "itkKdTree.h"
#include "itkKdTreeBasedKmeansEstimator.h"
#include "itkWeightedCentroidKdTreeGenerator.h"

#include "itkMinimumDecisionRule.h"
#include "itkEuclideanDistanceMetric.h"
#include "itkImageToListSampleAdaptor.h"

#include "CLI11.hpp"

#include <string>
#include "itkTimeProbe.h"

#include "utils.h"

int main( int argc, char* argv[] )
{
  
  // parse command line using CLI ----------------------------------------------
  CLI::App app;
  app.description("Apply the ruiZhang algorithm");
  std::string inputFile ;
  std::string outputFile;
  double sigmaMin;
  double sigmaMax;
  float tau {0.75};
  int nbClasses {5};
  bool isInputDicom = false;
  
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
  app.add_option("--tau,-a", tau, "Jerman's tau" ,true);
  app.add_option("--nbSeeds,-s", nbClasses, "Sato's alpha2" ,true);
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
  
  auto img = vUtils::readImage<ImageType>(inputFile,isInputDicom);
  MaskImageType::Pointer maskImage;
  
  auto duplicator = itk::ImageDuplicator<ImageType>::New();
  ImageType::Pointer maskedImage;
  auto stats = itk::StatisticsImageFilter<ImageType>::New();
  
  
  typedef itk::Statistics::ImageToListSampleAdaptor< ImageType >   AdaptorType;
  AdaptorType::Pointer adaptor = AdaptorType::New();
  // Create the K-d tree structure
  typedef itk::Statistics::WeightedCentroidKdTreeGenerator< AdaptorType > TreeGeneratorType;
  TreeGeneratorType::Pointer treeGenerator = TreeGeneratorType::New();
  typedef TreeGeneratorType::KdTreeType TreeType;
  typedef itk::Statistics::KdTreeBasedKmeansEstimator<TreeType> EstimatorType;
  EstimatorType::Pointer estimator = EstimatorType::New();
  
  if( !maskFile.empty() )
  {
    maskImage = vUtils::readImage<MaskImageType>(maskFile,isInputDicom);
    // creating the mask manually.
    
    duplicator->SetInputImage(img);
    duplicator->Update();
    maskedImage = duplicator->GetOutput();
    itk::ImageRegionIterator<ImageType> itMasked(maskedImage,maskedImage->GetLargestPossibleRegion());
    itk::ImageRegionIterator<MaskImageType> itMask(maskImage,maskedImage->GetLargestPossibleRegion());
    itMask.GoToBegin();
    itMasked.GoToBegin();
    
    while(!itMasked.IsAtEnd())
    {
      if( itMask.Get() == 0)
      {
        itMasked.Set(0);
      }
      ++itMasked;
      ++itMask;
    }
    
    adaptor->SetImage( maskedImage );
    stats->SetInput( maskedImage );
  }
  else
  {
    adaptor->SetImage( img );
    stats->SetInput( img );
  }
  
  stats->Update();
  adaptor->Update();
  
  double min = stats->GetMinimum(); // 0/4
  double max = stats->GetMaximum(); // 4/4
  
  EstimatorType::ParametersType initialMeans(nbClasses);
  
  
  
  std::cout<<"min seed: "<<min<<std::endl;
  
  initialMeans[0] = min;
  double step = (max - min) / nbClasses;
  for(int i=1; i<nbClasses-1;i++)
  {
    initialMeans[i] = step * i + min;
    std::cout<<"seed: "<<step*i+min<<std::endl;
  }
  initialMeans[nbClasses-1] = max;
  
  std::cout<<"max seed: "<<max<<std::endl;
  itk::TimeProbe clock;
  
  clock.Start();
  treeGenerator->SetSample( adaptor );
  treeGenerator->SetBucketSize( 16 );
  treeGenerator->Update();
  estimator->SetParameters( initialMeans );
  estimator->SetKdTree( treeGenerator->GetOutput() );
  estimator->SetMaximumIteration( 200 );
  estimator->SetCentroidPositionChangesThreshold(0.0);
  estimator->StartOptimization();
  clock.Stop();
  
  std::cout<<"clock:"<<clock.GetTotal()<<std::endl;
  EstimatorType::ParametersType estimatedMeans = estimator->GetParameters();
  
  for ( unsigned int i = 0 ; i < nbClasses ; ++i )
  {
    std::cout << "cluster[" << i << "] " << std::endl;
    std::cout << "    estimated mean : " << estimatedMeans[i] << std::endl;
  }
  
  double alpha = (estimatedMeans[nbClasses-1] - estimatedMeans[nbClasses-2])/2.0;
  double beta  = (estimatedMeans[nbClasses-1] + estimatedMeans[nbClasses-1])/2.0;
  
  // filtering with sigmoid
  
  typedef itk::SigmoidImageFilter<ImageType,ImageType> SigmoidFilterType;
  auto sigmoidFilter = SigmoidFilterType::New();
  
  sigmoidFilter->SetOutputMaximum(1);
  sigmoidFilter->SetOutputMinimum(0);
  
  sigmoidFilter->SetAlpha(alpha);
  sigmoidFilter->SetBeta(beta);
  
  sigmoidFilter->SetInput( img );
  
  using HessianPixelType = itk::SymmetricSecondRankTensor< double, Dimension >;
  using HessianImageType = itk::Image< HessianPixelType, Dimension >;
  
  using OutputImageType = itk::Image< double, Dimension >;
  
  using RuiZhangFilterType = itk::HessianToRuiZhangMeasureImageFilter<HessianImageType, OutputImageType,MaskImageType>;
  auto ruiZhangFilter = RuiZhangFilterType::New();
  ruiZhangFilter->SetTau(tau);
  
  if( !maskFile.empty() )
  {
    ruiZhangFilter->SetMaskImage(maskImage);
  }
  
  using MultiScaleEnhancementFilterType = itk::MultiScaleHessianBasedMeasureImageFilter< ImageType, HessianImageType, OutputImageType >;
  MultiScaleEnhancementFilterType::Pointer multiScaleEnhancementFilter =  MultiScaleEnhancementFilterType::New();
  multiScaleEnhancementFilter->SetInput( sigmoidFilter->GetOutput() );
  multiScaleEnhancementFilter->SetHessianToMeasureFilter( ruiZhangFilter );
  multiScaleEnhancementFilter->SetSigmaStepMethodToLogarithmic();
  multiScaleEnhancementFilter->SetSigmaMinimum( sigmaMin );
  multiScaleEnhancementFilter->SetSigmaMaximum( sigmaMax );
  multiScaleEnhancementFilter->SetNumberOfSigmaSteps( nbSigmaSteps );
  
  // Saving image
  
  using imageWriterType = ImageType;
  typedef  itk::ImageFileWriter< imageWriterType  > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput( multiScaleEnhancementFilter->GetOutput() );
  writer->SetFileName( std::string(outputFile) );
  writer->Update();
  
  return EXIT_SUCCESS;
}
