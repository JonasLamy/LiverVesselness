#ifndef _BENCHMARK_H
#define _BENCHMARK_H

#include "bench_evaluation.h"
#include "utils.h"

#include <json/json.h>

#include <string>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <math.h>

#include "itkImageFileReader.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkImageFileWriter.h"


template<class TImageType, class TGroundTruthImageType, class TMaskImageType>
class Benchmark
{
public:
    Benchmark(const Json::Value root,
                std::string inputFileName,
                typename TGroundTruthImageType::Pointer gtImage,                
                const std::vector<typename TMaskImageType::Pointer> & maskList,
                std::vector<std::ofstream> & resultsMaskList);


    ~Benchmark();   
    void SetDicomInput(){m_inputIsDicom = true;}
    void SetNiftiInput(){m_inputIsDicom = false;}
    void SetComputeMetricsOnly(bool computeMetricsOnly){m_computeMetricsOnly = computeMetricsOnly;}
    void SetRemoveResultsVolume(bool removeResultsVolume){ m_removeResultsVolume = removeResultsVolume;}
    void SetOutputDirectory(const std::string &outputDir){m_outputDir = outputDir; }
    void SetPatientDirectory(const std::string &patient){m_patient = patient;}
    void SetEnhancementMaskName(const std::string &maskEnhancementFileName){m_maskEnhancementFileName = maskEnhancementFileName;}
    void SetNbThresholds(int nbThresholds){m_nbThresholds=nbThresholds;}
    void SetRescaleFilters(bool rescaleFilter){m_rescaleFilters=rescaleFilter;}
    void run();
    
private:
    std::string m_inputFileName;
    std::string m_maskEnhancementFileName;

    std::string m_outputDir;
    std::string m_patient;

    int m_nbThresholds;
    static int m_nbAlgorithms;
    bool m_inputIsDicom;
    bool m_computeMetricsOnly;
    bool m_removeResultsVolume;
    bool m_rescaleFilters;
    Json::Value m_rootNode;

    std::vector< std::ofstream*> m_resultsMaskList;
    std::vector<typename TMaskImageType::Pointer> m_maskList;
    typename TGroundTruthImageType::Pointer m_gt;

    // *old version * void launchScript(int algoID,const std::string &commandLine,const std::string &outputDir, const std::string &outputName);
    // test function for speedy confusion matrix computation
    void launchScriptFast(int algoID,const std::string &commandLine,const std::string &inputVolumePath,const std::string &outputDir, const std::string &outputName);
    
    void computeMetrics(
                        const std::string & outputName,
                        //typename TImageType::Pointer inputImage,
                        typename TImageType::Pointer outputImage,
                        typename TGroundTruthImageType::Pointer gt,
                        typename TMaskImageType::Pointer mask, 
                        std::ofstream * stream);
    void computeConfusionValues(typename std::list<typename TImageType::PixelType> & foregroundValues,
                                typename std::list<typename TImageType::IndexType> & foregroundIndexes,
                                float threshold,
                                long & TP_f,
                                long & TN_f, 
                                long & FP_f, 
                                long & FN_f);
};

template<class TImageType, class TGroundTruthImageType, class TMaskImageType>
int Benchmark<TImageType,TGroundTruthImageType,TMaskImageType>::m_nbAlgorithms = 0;

#include "benchmark.hxx"
#endif