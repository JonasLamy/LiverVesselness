#include "benchmark.h"
#include "itkStatisticsImageFilter.h"
#include "itkImageFileWriter.h"


template<class TImageType, class TGroundTruthImageType, class TMaskImageType>
Benchmark<TImageType,TGroundTruthImageType,TMaskImageType>::Benchmark(const Json::Value root,
                                                                      std::string inputFileName,
                                                                      typename TGroundTruthImageType::Pointer gtImage,
                                                                      const std::vector<typename TMaskImageType::Pointer> & maskList,
                                                                      std::vector<std::ofstream> & resultsMaskList)
{

    // filter options
    m_removeResultsVolume = false;
    m_computeMetricsOnly = false;
    m_maskEnhancementFileName = "";
    
    // json root node
    m_rootNode = root;
    
    // benchmark images
    m_gt = gtImage;
    m_maskList = maskList;
    m_inputFileName = inputFileName;

    // csv file streams
    for(int i=0; i<resultsMaskList.size();i++)
    {
      m_resultsMaskList.push_back( &resultsMaskList[i]);
    }
     
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

          if( (m.compare("sigmaMin") == 0) || (m.compare("sigmaMax") == 0) || ( m.compare("scaleMin") == 0 ) )
          {
            auto imgForHeaderData = vUtils::readImage<TImageType>(m_inputFileName, m_inputIsDicom);
            auto spacing = imgForHeaderData->GetSpacing();

            float scaled_spacing = std::stof( arg[m].asString() ) / spacing[0];
            if( m.compare("scaleMin") == 0 ){ sStream << "--" << m << " " << (int)scaled_spacing << " "; } // rorpo is a special case wuth int parameter
            else{ sStream << "--" << m << " " << std::setprecision(3) << scaled_spacing << " "; } // usual scale Space
            
          }
          else
          { 
            sStream << "--" << m << " " << arg[m].asString() << " ";
          }
        }

        if( !m_maskEnhancementFileName.empty() )
        {
          sStream << "--mask " << m_maskEnhancementFileName << " ";
        }
        std::cout<<sStream.str()<<std::endl;
        launchScriptFast(m_nbAlgorithms,sStream.str(),m_inputFileName,m_outputDir,outputName);

        if(m_removeResultsVolume)
        {
          remove( (m_outputDir+ "/" + outputName).c_str() );
        }

       m_nbAlgorithms++;
      }
    }
    else{
      throw "Bad parameters file, arguments formatting";
    }
    
  }
}

template<class TImageType, class TGroundTruthImageType, class TMaskImageType>
void Benchmark<TImageType,TGroundTruthImageType,TMaskImageType>::launchScriptFast(int algoID,
                                                                              const std::string &commandLine,
                                                                              const std::string &inputVolumePath,
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
  auto inputImage = vUtils::readImage<TImageType>(inputVolumePath,false);
  
  std::cout<<"comparing output to ground truth....\n";
  if( outputImage->GetLargestPossibleRegion().GetSize() != m_gt->GetLargestPossibleRegion().GetSize() )
    {
      std::cout<<"output from program and groundTruth size does not match...No stats computed"<<std::endl;
      return;
    }

  for(int i=0; i<m_maskList.size();i++)
  {
    computeMetrics(outputName,inputImage,outputImage,m_gt,m_maskList[i],m_resultsMaskList[i]);
  }

  std::cout<<"done"<<std::endl;
}

template<class TImageType, class TGroundTruthImageType, class TMaskImageType>
void Benchmark<TImageType,TGroundTruthImageType,TMaskImageType>::computeMetrics(const std::string & outputName,
                                                                                typename TImageType::Pointer inputImage,
                                                                                typename TImageType::Pointer outputImage,
                                                                                typename TGroundTruthImageType::Pointer gt,
                                                                                typename TMaskImageType::Pointer mask, 
                                                                                std::ofstream * stream)
{
  // compute the zeros, iterate over the region of interest
  itk::ImageRegionConstIteratorWithIndex<TImageType> itInput(inputImage,inputImage->GetLargestPossibleRegion());
  itk::ImageRegionConstIteratorWithIndex<TImageType> itFilter(outputImage,outputImage->GetLargestPossibleRegion());
  itk::ImageRegionConstIteratorWithIndex<TMaskImageType> itMask(mask,mask->GetLargestPossibleRegion());
  itk::ImageRegionConstIteratorWithIndex<TGroundTruthImageType> itGt(gt,gt->GetLargestPossibleRegion());

  using ImageCalculatorFilterType = itk::MinimumMaximumImageCalculator<TGroundTruthImageType>;
  auto minMaxFilter = ImageCalculatorFilterType::New();
  minMaxFilter->SetImage(gt);
  minMaxFilter->Compute();


  // positives values are stored
  typename std::list<typename TImageType::PixelType> foregroundValues;
  typename std::list<typename TImageType::IndexType> foregroundIndexes;

  typename std::list<typename TImageType::IndexType> backgroundIndexes;

  itFilter.GoToBegin();
  itMask.GoToBegin();
  itGt.GoToBegin();

  double minInput = minMaxFilter->GetMinimum();
  double maxInput = minMaxFilter->GetMaximum();

  std::cout<<"min : "<<minInput<<" max :"<<maxInput<<std::endl; 

  double intensityRange = maxInput - minInput;

  // SNR and PSNR power elements
  long double powerImage = 0;
  // The noisy image correspond in our case to the filter output
  long double powerFilter = 0;
  long double MSE = 0;
  long long nbPixels = 0;
  // Filter output and image range are not in the same domain.....
  double scaledFilterValue = 0;
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
      

      scaledFilterValue = ( intensityRange * itFilter.Value() ) + minInput; 
      
      powerImage += itGt.Value() * itGt.Value(); 
      powerFilter += scaledFilterValue * scaledFilterValue;

      MSE += ( scaledFilterValue - itGt.Value() ) * ( scaledFilterValue - itGt.Value() ); // squared for mean squared error for snr
    }
    
    

    ++itInput;
    ++itMask;
    ++itFilter;
    ++itGt;

    nbPixels++;
  }

  // Finishing the computation of the SNR / PSNR

  powerImage /= nbPixels * 2;
  powerFilter /= nbPixels * 2;

  MSE /= nbPixels * 2;

  double snr = 10*std::log10(powerImage/powerFilter);
  double psnr = 10*std::log10( (maxInput*maxInput) / MSE);
  
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

    TN_f=0;
    //FP_f=0;
    FN_f=0;  

    computeConfusionValues(foregroundValues,foregroundIndexes,i/maxBoundf,TP_f,TN_f,FP_f,FN_f);

    Eval<TImageType, TGroundTruthImageType, TMaskImageType> eval(snr,psnr,TP_f, TN_f+TN_b, FP_f, FN_f+FN_b);
    (*stream)<<m_patient<<","<<outputName<<","<<i/maxBoundf<<","<< eval;

    if( i%10 == 0)
    {
      std::cout<<"threshold:"<<i/maxBoundf<<std::endl;
      std::cout<<"true positive rate : " << eval.sensitivity() << "\n"
            << " false positive rate : " << 1.0f - eval.specificity() << "\n";
    }
  }

  // special case, last threshold

  TN_f=0;
  //FP_f=0;
  FN_f=0;  

  computeConfusionValues(foregroundValues,foregroundIndexes,0,TP_f,TN_f,FP_f,FN_f);

  Eval<TImageType, TGroundTruthImageType, TMaskImageType> eval(snr,psnr,TP_f+FN_b, TN_f, FP_f+TN_b, FN_f);
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


/*
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
*/