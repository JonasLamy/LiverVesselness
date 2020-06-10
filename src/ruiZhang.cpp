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


#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <string>
#include "itkTimeProbe.h"

#include "utils.h"

int main( int argc, char* argv[] )
{
    bool isInputDicom = false;
    namespace po = boost::program_options;
    // parsing arguments
    po::options_description general_opt("Allowed options are ");
    general_opt.add_options()
    ("help,h", "display this message")
    ("input,i", po::value<std::string>(), "inputName : input img" )
    ("output,o", po::value<std::string>(), "ouputName : output img" )
    ("sigmaMin,m", po::value<float>(), "scale space sigma min")
    ("sigmaMax,M", po::value<float>(), "scale space sigma max")
    ("tau,t", po::value<float>()->default_value(0.75), "Jerman's tau" )
    ("nbSeeds,s",po::value<int>()->default_value(5),"number of kmean seeds")
    ("nbSigmaSteps,n",po::value<int>(),"nb steps sigma")
    ("inputIsDicom,d",po::bool_switch(&isInputDicom),"specify dicom input")
    ("mask,k",po::value<std::string>()->default_value(""),"mask response by image");

    bool parsingOK = true;
    po::variables_map vm;

    try{
      po::store(po::parse_command_line(argc,argv,general_opt),vm);
    }catch(const std::exception& ex)
    {
      parsingOK = false;
      std::cout<<"Error checking program option"<<ex.what()<< std::endl;
    }

    po::notify(vm);
    if( !parsingOK || vm.count("help") || argc<=1 )
    {
      std::cout<< general_opt << std::endl;
    
      return 0;
    }

    std::string inputFile = vm["input"].as<std::string>();
    std::string outputFile = vm["output"].as<std::string>();
    float sigmaMin = vm["sigmaMin"].as<float>();
    float sigmaMax = vm["sigmaMax"].as<float>();
    int nbSigmaSteps = vm["nbSigmaSteps"].as<int>();
    float tau = vm["tau"].as<float>();
    unsigned int nbClasses = vm["nbSeeds"].as<int>();
    std::string maskFile = vm["mask"].as<std::string>();

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
    //multiScaleEnhancementFilter->SetSigmaStepMethodToLogarithmic();
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
