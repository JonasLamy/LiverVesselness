#ifndef _BENCHMARK_H
#define _BENCHMARK_H

#include "bench_evaluation.h"
#include "utils.h"

#include <json/json.h>

#include <string>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <map>

#include "itkImageFileReader.h"
#include "itkBinaryThresholdImageFilter.h"


template<class TImageType, class TGroundTruthImageType, class TMaskImageType>
class Benchmark
{
public:
    Benchmark(const Json::Value root,std::string inputFileName,std::string csvFileName, typename TGroundTruthImageType::Pointer gtImage, typename TMaskImageType::Pointer maskImage);
    ~Benchmark();
    void SetDicomInput(){m_inputIsDicom = true;}
    void SetNiftiInput(){m_inputIsDicom = false;}

    void run();
private:
    std::string m_inputFileName;
    bool m_inputIsDicom;
    Json::Value m_rootNode;

    VoxelsMap m_vMap;
    MetricsMap m_mMap;

    std::ofstream m_resultFileStream;

    typename TGroundTruthImageType::Pointer m_gt;
    typename TMaskImageType::Pointer m_mask;

    void launchScript(const std::string &commandLine,const std::string &outputName);
    void addResultsToFile(int algoID,int threshold, const Eval<TGroundTruthImageType,TGroundTruthImageType,TMaskImageType>& eval);
};

#include "benchmark.hxx"
#endif