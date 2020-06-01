#ifndef _BENCHMARK_H
#define _BENCHMARK_H

#include "bench_evaluation.h"
#include "utils.h"

#include <json/json.h>

#include <string>
#include <fstream>
#include <iostream>
#include <stdlib.h>

#include "itkImageFileReader.h"
#include "itkBinaryThresholdImageFilter.h"


template<class TImageType, class TGroundTruthImageType, class TMaskImageType>
class Benchmark
{
public:
    Benchmark(const Json::Value root,
                std::string inputFileName,
                typename TGroundTruthImageType::Pointer gtImage,
                std::ofstream & csvFileStream,
                typename TMaskImageType::Pointer maskImage,
                std::ofstream & csvFileVesselsDilated,
                typename TMaskImageType::Pointer maskVesselsDilated,
                std::ofstream & csvFileBifurcation,
                typename TMaskImageType::Pointer maskBifurcation);


    ~Benchmark();   
    void SetDicomInput(){m_inputIsDicom = true;}
    void SetNiftiInput(){m_inputIsDicom = false;}
    void SetComputeMetricsOnly(bool computeMetricsOnly){m_computeMetricsOnly = computeMetricsOnly;}
    void SetremoveResultsVolume(bool removeResultsVolume){ m_removeResultsVolume = removeResultsVolume;}
    void SetOutputDirectory(const std::string &outputDir){m_outputDir = outputDir; }
    void SetPatientDirectory(const std::string &patient){m_patient = patient;}
    void SetMaskName(const std::string &maskFileName){m_maskFileName = maskFileName;}
    void SetNbThresholds(int nbThresholds){m_nbThresholds=nbThresholds;}
    void run();
    
private:
    std::string m_inputFileName;
    std::string m_maskFileName;

    std::string m_outputDir;
    std::string m_patient;

    int m_nbThresholds;
    static int m_nbAlgorithms;
    bool m_inputIsDicom;
    bool m_computeMetricsOnly;
    bool m_removeResultsVolume;
    Json::Value m_rootNode;

    std::ofstream * m_resultMaskLiver;
    std::ofstream * m_resultMaskVesselsDilated;
    std::ofstream * m_resultMaskBifurcation;

    typename TGroundTruthImageType::Pointer m_gt;
    typename TMaskImageType::Pointer m_maskLiver;
    typename TMaskImageType::Pointer m_maskVesselsDilated;
    typename TMaskImageType::Pointer m_maskBifurcation;

    void launchScript(int algoID,const std::string &commandLine,const std::string &outputDir, const std::string &outputName);
    //void addResultsToFile(int algoID,int threshold, const Eval<TGroundTruthImageType,TGroundTruthImageType,TMaskImageType>& eval);
};

template<class TImageType, class TGroundTruthImageType, class TMaskImageType>
int Benchmark<TImageType,TGroundTruthImageType,TMaskImageType>::m_nbAlgorithms = 0;

#include "benchmark.hxx"
#endif