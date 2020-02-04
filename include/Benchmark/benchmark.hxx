#include "benchmark.h"
#include "itkStatisticsImageFilter.h"
#include "itkImageFileWriter.h"


template<class TImageType, class TGroundTruthImageType, class TMaskImageType>
Benchmark<TImageType,TGroundTruthImageType,TMaskImageType>::Benchmark(const Json::Value root, 
                                                std::string inputFileName,
                                                std::ofstream &csvFileStream,
                                                typename TGroundTruthImageType::Pointer gtImage, 
                                                typename TMaskImageType::Pointer maskImage)
{
    m_removeResultsVolume = false;
    m_computeMetricsOnly = false;
    m_rootNode = root;
    m_gt = gtImage;
    m_mask = maskImage;
    m_inputFileName = inputFileName;

    m_resultFileStream = &csvFileStream;
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

    if (algo.isArray()) // the algorithm contains several sets of parameters
    {
      for (auto &p : algo)
      {
        const std::string outputName = p["Output"].asString();
        const Json::Value arguments = p["Arguments"];

        std::stringstream sStream;
        sStream << "./" << algoName << " "
                << "--input"
                << " " << m_inputFileName << " "
                << "--output " << m_outputDir+ "/" + outputName << " ";

        for (auto &arg : arguments)
        {
          std::string m = arg.getMemberNames()[0]; // only one name in the array
          sStream << "--" << m << " " << arg[m].asString() << " ";
        }
        launchScript(m_nbAlgorithms,sStream.str(),m_outputDir,outputName);

        if(m_removeResultsVolume)
        {
          remove( (m_outputDir+ "/" + outputName).c_str() );
        }

       m_nbAlgorithms++;
      }
    }
    else // the algorithm contains only one set of parameters
    {
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
      launchScript(m_nbAlgorithms,sStream.str(),m_outputDir,outputName);
      
      if(m_removeResultsVolume)
      {
        remove( (m_outputDir+ "/" + outputName).c_str());
      }
      
      m_nbAlgorithms++;
    }
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
  int step = 2;
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
    
    Eval<TGroundTruthImageType,TGroundTruthImageType,TMaskImageType> eval(segmentationImage,m_gt,m_mask,std::to_string(i/maxBoundf));
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
    (*m_resultFileStream)<<m_patient<<","<<outputName<<","<<i/maxBoundf<<","<< eval;

    // TODO uncomment for debug
    /*
    auto writer = itk::ImageFileWriter<TGroundTruthImageType>::New();
    writer->SetFileName( std::string("toto/") + std::to_string(i) + std::string(".nii") );
	  writer->SetInput(segmentationImage);
	  writer->Update();
    */
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

  std::cout<<"done"<<std::endl;
}