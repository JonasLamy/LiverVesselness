#include "benchmark.h"
#include "itkStatisticsImageFilter.h"


template<class TImageType, class TGroundTruthImageType, class TMaskImageType>
Benchmark<TImageType,TGroundTruthImageType,TMaskImageType>::Benchmark(const Json::Value root, 
                                                std::string inputFileName,
                                                std::string csvFileName,
                                                typename TGroundTruthImageType::Pointer gtImage, 
                                                typename TMaskImageType::Pointer maskImage)
{
    m_rootNode = root;
    m_gt = gtImage;
    m_mask = maskImage;
    m_inputFileName = inputFileName;

    m_vMap["TP"] = std::vector<long>();
    m_vMap["TN"] = std::vector<long>();
    m_vMap["FP"] = std::vector<long>();
    m_vMap["FN"] = std::vector<long>();

    m_mMap["Dice"] = std::vector<double>();
    m_mMap["MCC"] = std::vector<double>();

    m_mMap["sensitivity"] = std::vector<double>();
    m_mMap["accuracy"] = std::vector<double>();
    m_mMap["precision"] = std::vector<double>();
    m_mMap["specificity"] = std::vector<double>();

    // opening resultFileStream
    m_resultFileStream.open(csvFileName, ios::out | ios::trunc); // if the file already exists, we discard content
    if( m_resultFileStream.is_open() )
    {
      m_resultFileStream <<"AlgoID,threshold,TP,TN,FP,FN,sensitivity,specificity,precision,accuracy,dice,MatthewsCorrelation"<<std::endl;
    } 
    else{ 
      throw "Error opening csv file....";
    }
}

template<class TImageType, class TGroundTruthImageType, class TMaskImageType>
Benchmark<TImageType,TGroundTruthImageType,TMaskImageType>::~Benchmark()
{
    if( m_resultFileStream.is_open() )
    {
      m_resultFileStream.close();
    }
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
  int nbAlgorithms = 0;
  for (auto &algoName : algoNames)
  {
    std::cout << "Algorithm n°" << nbAlgorithms << " " << algoName << std::endl;
    nbAlgorithms++;

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
                << "--output " << outputName << " ";

        for (auto &arg : arguments)
        {
          std::string m = arg.getMemberNames()[0]; // only one name in the array
          sStream << "--" << m << " " << arg[m].asString() << " ";
        }
        launchScript(sStream.str(),outputName);
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
              << "--output " << outputName << " ";

      for (auto &arg : arguments)
      {
        std::string m = arg.getMemberNames()[0]; // only one name in the array
        sStream << m << " " << arg[m].asString() << " ";
      }
      launchScript(sStream.str(),outputName);
    }
  }
}


template<class TImageType, class TGroundTruthImageType, class TMaskImageType>
void Benchmark<TImageType,TGroundTruthImageType,TMaskImageType>::launchScript(const std::string &commandLine,const std::string &outputName)
{
  typedef itk::BinaryThresholdImageFilter<TImageType,TGroundTruthImageType> ThresholdFilterType;

  std::cout<<commandLine<<std::endl;
  // starting external algorithm
  if( m_inputIsDicom == true )
  {
    std::string commandLineDicom = commandLine + " --inputIsDicom";
    system(commandLineDicom.c_str() );
  }
  else
  {
    system(commandLine.c_str());
  }
  std::cout<<"opening result"<<std::endl;
  auto outputImage = vUtils::readImage<TImageType>(outputName,false);
  
  /*
  std::cout<<"comparing output to ground truth....\n";
  if( outputImage->GetLargestPossibleRegion().GetSize() != m_gt->GetLargestPossibleRegion().GetSize() )
    {
      std::cout<<"output from program and groundTruth size does not match...No stats computed"<<std::endl;
      return;
    }
      
  // Computing roc curve for the image segmentation
  double bestThreshold = 0;
  double minDist = 1000;
  for(float i=1.0f; i>=0.0f;i-=0.1f)
  {
    // thresholding for all values ( keeping upper value and adding more incertainty as lower probabilities are accepted )
    auto tFilter = ThresholdFilterType::New();
    tFilter->SetInput( outputImage );
    tFilter->SetInsideValue( 1 );
    tFilter->SetOutsideValue( 0 );

    tFilter->SetLowerThreshold(i);
    tFilter->SetUpperThreshold(1);
    tFilter->Update();
    auto segmentationImage = tFilter->GetOutput();
    
    
    Eval<TGroundTruthImageType,TGroundTruthImageType,TMaskImageType> eval(segmentationImage,m_gt,m_mask);
    std::cout<<"true positive rate : " << eval.sensitivity() << "\n"
            << " false positive rate : " << 1.0f - eval.specificity() << "\n";
    
    // perfect qualifier (0,1), our segmentation (TPR,FPR)
    float euclideanDistance =  (eval.sensitivity()*eval.sensitivity()) + ( 1.0f - eval.specificity() ) * ( 1.0f - eval.specificity() );
    if( minDist >  euclideanDistance )
    {
      minDist = euclideanDistance;
      bestThreshold = i;
    }
    eval.print();
  }
  std::cout<<"best threshold from ROC:"<<bestThreshold<<std::endl;
  */
  /* 
  
  vMap["TP"].push_back( eval.TP() );
  vMap["TN"].push_back( eval.TN() );
  vMap["FP"].push_back( eval.FP() );
  vMap["FN"].push_back( eval.FN() );

  mMap["Dice"].push_back( eval.dice() );
  mMap["MCC"].push_back( eval.matthewsCorrelation() );

  mMap["sensitivity"].push_back( eval.sensitivity() );
  mMap["accuracy"].push_back( eval.accuracy() );
  mMap["precision"].push_back( eval.precision() );
  mMap["specificity"].push_back( eval.specificity() );

  */
      
  std::cout<<"done"<<std::endl;
}