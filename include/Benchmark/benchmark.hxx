#include "benchmark.h"
#include "itkStatisticsImageFilter.h"
#include "itkImageFileWriter.h"


template<class TImageType, class TGroundTruthImageType, class TMaskImageType>
Benchmark<TImageType,TGroundTruthImageType,TMaskImageType>::Benchmark(const Json::Value root,
                                                                      std::string inputFileName,
                                                                      typename TGroundTruthImageType::Pointer gtImage,
                                                                      std::ofstream & csvFileStream,
                                                                      typename TMaskImageType::Pointer maskImage,
                                                                      std::ofstream & csvFileVesselsDilated,
                                                                      typename TMaskImageType::Pointer maskVesselsDilated,
                                                                      std::ofstream & csvFileBifurcation,
                                                                      typename TMaskImageType::Pointer maskBifurcation)
{

    // filter options
    m_removeResultsVolume = false;
    m_computeMetricsOnly = false;
    m_maskFileName = "";
    
    // json root node
    m_rootNode = root;
    
    // benchmark images
    m_gt = gtImage;
    m_maskLiver = maskImage;
    m_maskVesselsDilated = maskVesselsDilated;
    m_maskBifurcation = maskBifurcation;
    m_inputFileName = inputFileName;

    // csv file streams
    m_resultMaskLiver = &csvFileStream;
    m_resultMaskVesselsDilated = &csvFileVesselsDilated;
    m_resultMaskBifurcation = &csvFileBifurcation;
}

template<class TImageType, class TGroundTruthImageType, class TMaskImageType>
Benchmark<TImageType,TGroundTruthImageType,TMaskImageType>::~Benchmark()
{
}

template<class TImageType, class TGroundTruthImageType, class TMaskImageType>
void Benchmark<TImageType,TGroundTruthImageType,TMaskImageType>::run()
{

  // ------------------------
  // Starting benchmark
  // -------------------------

  std::cout<<"starting benchmark"<<std::endl;
  std::cout<<"------------------"<<std::endl;

  Json::Value::Members algoNames = m_rootNode.getMemberNames();
  for (auto &algoName : algoNames)
  {
    std::cout << "Algorithm n°" << m_nbAlgorithms << " " << algoName << std::endl;

    const Json::Value algo = m_rootNode[algoName];

    const std::string outputName = algo["Output"].asString();
    const Json::Value arguments = algo["Arguments"];

    std::stringstream sStream;
    sStream << "./" << algoName << " "
            << "--input"
            << " " << m_inputFileName << " "
            << "--output " << m_outputDir+ "/" + outputName << " ";

    for (auto &arg : arguments)
    {
      std::string m = arg.getMemberNames()[0]; // only one name in the array
      sStream << m << " " << arg[m].asString() << " ";
    }

    if( !m_maskFileName.empty() )
    {
      sStream << "--mask " << m_maskFileName << " ";
    }

    launchScript(m_nbAlgorithms,sStream.str(),m_outputDir,outputName);
    
    if(m_removeResultsVolume)
    {
      remove( (m_outputDir+ "/" + outputName).c_str());
    }
    
    m_nbAlgorithms++;
  }
}


template<class TImageType, class TGroundTruthImageType, class TMaskImageType>
void Benchmark<TImageType,TGroundTruthImageType,TMaskImageType>::launchScript(int algoID,const std::string &commandLine,const std::string &outputDir, const std::string &outputName)
{
  typedef itk::BinaryThresholdImageFilter<TImageType,TGroundTruthImageType> ThresholdFilterType;

  if( m_computeMetricsOnly )
  {
    std::cout<<"computing metrics only..."<<std::endl;
  }
  else
  {
    std::cout<<commandLine<<std::endl;
    // starting external algorithm
    if( m_inputIsDicom == true )
    {
      std::string commandLineDicom = commandLine + " --inputIsDicom";
      int returnValue = system(commandLineDicom.c_str() );
    }
    else
    {
      int returnValue = system(commandLine.c_str());
    }
  }

  std::cout<<"opening result"<<std::endl;
  auto outputImage = vUtils::readImage<TImageType>(m_outputDir+ "/" + outputName,false);
  
  std::cout<<"comparing output to ground truth....\n";
  if( outputImage->GetLargestPossibleRegion().GetSize() != m_gt->GetLargestPossibleRegion().GetSize() )
    {
      std::cout<<"output from program and groundTruth size does not match...No stats computed"<<std::endl;
      return;
    }



  // Computing roc curve for the image segmentation
  double bestThreshold = 0;
  double minDist = 1000;

  // trying out new way of tresholding for complete ROC curve...
  int step = 1;
  int maxBound = 100;
  float maxBoundf = 100.0f;
  for(int i=maxBound; i>0; i-=step)
  {
    // thresholding for all values ( keeping upper value and adding more incertainty as lower probabilities are accepted )
    auto tFilter = ThresholdFilterType::New();
    tFilter->SetInput( outputImage );
    tFilter->SetInsideValue( 1 );
    tFilter->SetOutsideValue( 0 );

    tFilter->SetLowerThreshold(i/maxBoundf );
    tFilter->SetUpperThreshold(1.01f);
    tFilter->Update();
    auto segmentationImage = tFilter->GetOutput();
    
    Eval<TGroundTruthImageType,TGroundTruthImageType,TMaskImageType> eval(segmentationImage,m_gt,m_maskLiver,std::to_string(i/maxBoundf));
    Eval<TGroundTruthImageType,TGroundTruthImageType,TMaskImageType> evalBifurcations(segmentationImage,m_gt,m_maskBifurcation,std::to_string(i/maxBoundf),"bifurcations");
    Eval<TGroundTruthImageType,TGroundTruthImageType,TMaskImageType> evalDilatedVessels(segmentationImage,m_gt,m_maskVesselsDilated,std::to_string(i/maxBoundf),"vessels");

    if( i%10 == 0)
    {
      std::cout<<"threshold:"<<i/maxBoundf<<std::endl;
      std::cout<<"true positive rate : " << eval.sensitivity() << "\n"
            << " false positive rate : " << 1.0f - eval.specificity() << "\n";
    }
    // perfect qualifier (0,1), our segmentation (TPR,FPR)
    float euclideanDistance =  (eval.sensitivity()*eval.sensitivity()) + ( 1.0f - eval.specificity() ) * ( 1.0f - eval.specificity() );
    if( minDist >  euclideanDistance )
    {
      minDist = euclideanDistance;
      bestThreshold = i/maxBoundf;
    }

    // writing results to file
    (*m_resultMaskLiver)<<m_patient<<","<<outputName<<","<<i/maxBoundf<<","<< eval;
    (*m_resultMaskBifurcation)<<m_patient<<","<<outputName<<","<<i/maxBoundf<<","<< evalBifurcations;
    (*m_resultMaskVesselsDilated)<<m_patient<<","<<outputName<<","<<i/maxBoundf<<","<< evalDilatedVessels;

  }   
  std::cout<<"done"<<std::endl;
}

  // computing the special case 0 because looping on float is annoying....
  /*
  auto tFilter = ThresholdFilterType::New();
  tFilter->SetInput( outputImage );
  tFilter->SetInsideValue( 1 );
  tFilter->SetOutsideValue( 0 );

  tFilter->SetLowerThreshold( 0 );
  tFilter->SetUpperThreshold(1.01f);
  tFilter->Update();
  auto segmentationImage = tFilter->GetOutput();
  */
  // TODO uncomment for debug
  /*
  auto writer = itk::ImageFileWriter<TGroundTruthImageType>::New();
  writer->SetFileName( std::string("0.nii") );
	writer->SetInput(segmentationImage);
	writer->Update();
  */
  /*
  Eval<TGroundTruthImageType,TGroundTruthImageType,TMaskImageType> eval(segmentationImage,m_gt,m_mask,std::to_string(0));
  std::cout<<"true positive rate : " << eval.sensitivity() << "\n"
          << " false positive rate : " << 1.0f - eval.specificity() << "\n";
  
  // perfect qualifier (0,1), our segmentation (TPR,FPR)
  float euclideanDistance =  (eval.sensitivity()*eval.sensitivity()) + ( 1.0f - eval.specificity() ) * ( 1.0f - eval.specificity() );
  if( minDist >  euclideanDistance )
  {
    minDist = euclideanDistance;
    bestThreshold = 0;
  }
  (*m_resultFileStream)<<m_patient<<","<<outputName<<","<<0<<","<< eval;
  */