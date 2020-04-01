#ifndef bench_evaluation_h
#define bench_evaluation_h

#include "itkImage.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionConstIterator.h"
#include "itkHausdorffDistanceImageFilter.h"
#include <iostream>

// map to store values
using VoxelsMap = std::map<std::string,std::vector<long> >;
using MetricsMap = std::map<std::string,std::vector<double> >;

template<typename TImageType, typename TGroundTruthImageType, typename TMaskImageType>
class Eval{
 public:
  Eval(const typename TImageType::Pointer segmentation, const typename TGroundTruthImageType::Pointer gt, const typename TMaskImageType::Pointer mask,const std::string &id, const char* evalName = "eval");

  long TP(){return m_truePositive;}
  long TN(){return m_trueNegative;}
  long FP(){return m_falsePositive;}
  long FN(){return m_falseNegative;}


  double sensitivity();
  double specificity();
  double precision();
  double accuracy();
  double dice();
  double hausdorffDistance();
  long double matthewsCorrelation();


  static void roc(VoxelsMap &vMap); // not used or implemented here
    
  void print();
 private:
  void countMatchesBinary(const typename TImageType::Pointer img, const typename TGroundTruthImageType::Pointer gt, const typename TMaskImageType::Pointer mask, const std::string &id);
  
  long m_truePositive;
  long m_trueNegative;
  long m_falsePositive;
  long m_falseNegative;
  float m_epsilon;
  double m_hausdorff_distance;
  std::string m_evalName;
};

template<typename TImageType, typename TGroundTruthImageType, typename TMaskImageType>
std::ostream& operator <<(std::ostream& out,Eval<TImageType,TGroundTruthImageType,TMaskImageType> & eval)
{
  out<< eval.TP() << ","
		<< eval.TN() << ","
		<< eval.FP() << ","
		<< eval.FN() << ","
		<< eval.sensitivity() << ","
		<< eval.specificity() << ","
		<< eval.precision() << ","
		<< eval.accuracy() << ","
		<< eval.dice() << ","
		<< eval.matthewsCorrelation() << "," 
    << eval.hausdorffDistance() << std::endl;

    return out;
}
#include "bench_evaluation.hxx"

#endif // bench_evaluation_h
