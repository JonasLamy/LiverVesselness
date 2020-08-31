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
  Eval(const typename TImageType::Pointer segmentation, const typename TGroundTruthImageType::Pointer gt, const typename TMaskImageType::Pointer mask);
  Eval(long tp,long tn, long fp, long fn);

  long TP(){return m_truePositive;}
  long TN(){return m_trueNegative;}
  long FP(){return m_falsePositive;}
  long FN(){return m_falseNegative;}
  // testing purpose
  void setTP(long tp){m_truePositive = tp;}
  void setTN(long tn){m_trueNegative = tn;}
  void setFP(long fp){m_falsePositive = fp;}
  void setFN(long fn){m_falseNegative = fn;}

  long foreground(){return m_foreground;}
  long background(){return m_background;}

  double sensitivity();
  double specificity();
  double precision();
  double accuracy();
  double dice();
  double hausdorffDistance();
  long double matthewsCorrelation();
  double sparsity();

 private:
  void countMatchesBinary(const typename TImageType::Pointer img, const typename TGroundTruthImageType::Pointer gt, const typename TMaskImageType::Pointer mask);
  
  long m_truePositive;
  long m_trueNegative;
  long m_falsePositive;
  long m_falseNegative;
  float m_epsilon;
  double m_hausdorff_distance;
  long m_background;
  long m_foreground;
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
    << std::endl;
    //<< eval.sparsity() <<std::endl;

    return out;
}
#include "bench_evaluation.hxx"

#endif // bench_evaluation_h
