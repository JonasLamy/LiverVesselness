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

        if( !m_maskFileName.empty() )
        {
          sStream << "--mask " << m_maskFileName << " ";
        }

        launchScriptFast(m_nbAlgorithms,sStream.str(),m_outputDir,outputName);

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
        sStream << "--" << m << " " << arg[m].asString() << " ";
      }

      if( !m_maskFileName.empty() )
      {
        sStream << "--mask " << m_maskFileName << " ";
      }

      launchScriptFast(m_nbAlgorithms,sStream.str(),m_outputDir,outputName);
      
      if(m_removeResultsVolume)
      {
        remove( (m_outputDir+ "/" + outputName).c_str());
      }
      
      m_nbAlgorithms++;
    }
  }
}

template<class TImageType, class TGroundTruthImageType, class TMaskImageType>
void Benchmark<TImageType,TGroundTruthImageType,TMaskImageType>::launchScript(int algoID,
                                                                              const std::string &commandLine,
                                                                              const std::string &outputDir,
                                                                              const std::string &outputName)
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


  // trying out new way of tresholding for complete ROC curve...
  int step = 1;
  int maxBound = m_nbThresholds;
  float maxBoundf = m_nbThresholds;
  for(int i=maxBound; i>=0; i-=step)
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
    
    Eval<TGroundTruthImageType,TGroundTruthImageType,TMaskImageType> eval(segmentationImage,m_gt,m_maskLiver);
    Eval<TGroundTruthImageType,TGroundTruthImageType,TMaskImageType> evalBifurcations(segmentationImage,m_gt,m_maskBifurcation);
    Eval<TGroundTruthImageType,TGroundTruthImageType,TMaskImageType> evalDilatedVessels(segmentationImage,m_gt,m_maskVesselsDilated);

    if( i%10 == 0)
    {
      std::cout<<"threshold:"<<i/maxBoundf<<std::endl;
      std::cout<<"true positive rate : " << eval.sensitivity() << "\n"
            << " false positive rate : " << 1.0f - eval.specificity() << "\n";
    }


    // writing results to file
    (*m_resultMaskLiver)<<m_patient<<","<<outputName<<","<<i/maxBoundf<<","<< eval;
    (*m_resultMaskBifurcation)<<m_patient<<","<<outputName<<","<<i/maxBoundf<<","<< evalBifurcations;
    (*m_resultMaskVesselsDilated)<<m_patient<<","<<outputName<<","<<i/maxBoundf<<","<< evalDilatedVessels;
    
  }   
  std::cout<<"done"<<std::endl;
}


template<class TImageType, class TGroundTruthImageType, class TMaskImageType>
void Benchmark<TImageType,TGroundTruthImageType,TMaskImageType>::launchScriptFast(int algoID,
                                                                              const std::string &commandLine,
                                                                              const std::string &outputDir,
                                                                              const std::string &outputName)
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

  computeMetrics(outputName,outputImage,m_gt,m_maskLiver,m_resultMaskLiver);
  computeMetrics(outputName,outputImage,m_gt,m_maskBifurcation,m_resultMaskBifurcation);
  computeMetrics(outputName,outputImage,m_gt,m_maskVesselsDilated,m_resultMaskVesselsDilated);
  
  std::cout<<"done"<<std::endl;
}

template<class TImageType, class TGroundTruthImageType, class TMaskImageType>
void Benchmark<TImageType,TGroundTruthImageType,TMaskImageType>::computeMetrics(const std::string & outputName,TImageType* outputImage,TGroundTruthImageType* gt,TMaskImageType* mask, std::ofstream * stream)
{
  // compute the 0 zeros, iterate over the region of interest
  itk::ImageRegionConstIteratorWithIndex<TImageType> itFilter(outputImage,outputImage->GetLargestPossibleRegion());
  itk::ImageRegionConstIteratorWithIndex<TMaskImageType> itMask(mask,mask->GetLargestPossibleRegion());

  // positives values are stored
  typename std::list<typename TImageType::PixelType> foregroundValues;
  typename std::list<typename TImageType::IndexType> foregroundIndexes;
  
  typename std::list<typename TImageType::IndexType> backgroundIndexes;

  itFilter.GoToBegin();
  itMask.GoToBegin();
  while( !itFilter.IsAtEnd() )
  {
    if(itMask.Get() > 0 )
    {
      if(itFilter.Value() > 0)
      {
        foregroundValues.push_back( itFilter.Value() );
        foregroundIndexes.push_back( itFilter.GetIndex() );
      }
      else
      {
        backgroundIndexes.push_back( itFilter.GetIndex() );
      }  
    }
    ++itMask;
    ++itFilter;
  }

  std::cout<<"foreground length :"<<foregroundValues.size()<<std::endl;
  std::cout<<"background length :"<<backgroundIndexes.size()<<std::endl;
  
  long TN_b=0;
  long FN_b=0;

  // for a given threshold, loop through the list
  typename std::list<typename TImageType::IndexType>::iterator itBackgroundIndex = backgroundIndexes.begin();

  while( itBackgroundIndex != backgroundIndexes.end() )
  {

    if(gt->GetPixel(*itBackgroundIndex) == 0)
    {
      TN_b++;
    }
    else
    {
      FN_b++;
    }

    itBackgroundIndex++;
  }
  // list not necessary anymore, we clear it
  backgroundIndexes.clear();
  
  // looping over thresholds
  int step = 1;
  int maxBound = m_nbThresholds;
  float maxBoundf = m_nbThresholds;

  // at that point we have a list of potential thresholdable values and the number of voxels of value 0 in the ROI
  long TP_f=0;
  long TN_f=0;
  long FP_f=0;
  long FN_f=0;
  for(int i=maxBound; i>0; i-=step)
  {
    std::cout<<"threshold:"<<i<<std::endl;
    std::cout<<"threshold floating:"<<i/maxBoundf<<std::endl;

    TN_f=0;
    //FP_f=0;
    FN_f=0;  

    computeConfusionValues(foregroundValues,foregroundIndexes,i/maxBoundf,TP_f,TN_f,FP_f,FN_f);

    Eval<TImageType, TGroundTruthImageType, TMaskImageType> eval(TP_f, TN_f+TN_b, FP_f, FN_f+FN_b);
    (*stream)<<m_patient<<","<<outputName<<","<<i/maxBoundf<<","<< eval;

    if( i%10 == 0)
    {
      std::cout<<"threshold:"<<i/maxBoundf<<std::endl;
      std::cout<<"true positive rate : " << eval.sensitivity() << "\n"
            << " false positive rate : " << 1.0f - eval.specificity() << "\n";
    }
  }

  TN_f=0;
  //FP_f=0;
  FN_f=0;  

  std::cout << TP_f<<","<<TN_f<<","<<FP_f<<","<<FN_f<<std::endl;
  computeConfusionValues(foregroundValues,foregroundIndexes,0,TP_f,TN_f,FP_f,FN_f);

  Eval<TImageType, TGroundTruthImageType, TMaskImageType> eval(TP_f+FN_b, TN_f, FP_f+TN_b, FN_f);
  (*stream)<<m_patient<<","<<outputName<<","<<0<<","<< eval;
}

template<class TImageType, class TGroundTruthImageType, class TMaskImageType>
void Benchmark<TImageType,TGroundTruthImageType,TMaskImageType>::computeConfusionValues(typename std::list<typename TImageType::PixelType> & foregroundValues,
                                                                                        typename std::list<typename TImageType::IndexType> & foregroundIndexes,
                                                                                        float threshold,
                                                                                        long & TP_f,
                                                                                        long & TN_f, 
                                                                                        long & FP_f, 
                                                                                        long & FN_f)
{
    // for a given threshold, loop through the list
    typename std::list<typename TImageType::PixelType>::iterator itResponse = foregroundValues.begin();
    typename std::list<typename TImageType::IndexType>::iterator itIndex = foregroundIndexes.begin();
    while( itResponse != foregroundValues.end() )
    {
      // check for threshold
      if(*itResponse >= threshold)
      {
        // the values is thresholded to 1
        if (m_gt->GetPixel(*itIndex) > 0)
				{
					TP_f++;
				}
				else
				{
					FP_f++;
				}

        itResponse = foregroundValues.erase(itResponse);
        itIndex = foregroundIndexes.erase(itIndex);
      }
      else
      {
        if (m_gt->GetPixel(*itIndex) > 0)
				{
					FN_f++;
				}
				else
				{
					TN_f++;
				}

        // items are kept for further thresholds
        itResponse++;
        itIndex++;
      }
    }
}