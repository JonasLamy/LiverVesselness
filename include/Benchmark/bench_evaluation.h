#ifndef bench_evaluation_h
#define bench_evaluation_h

#include "itkImage.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionConstIterator.h"
#include "itkHausdorffDistanceImageFilter.h"
#include <iostream>
#include <cmath>
#include "utils.h"


struct ConfusionMatrix{
  long TP;
  long TN;
  long FP;
  long FN;
};

template<typename TImageType, typename TGroundTruthImageType, typename TMaskImageType>
class Eval{
 public:
  Eval(const typename TImageType::Pointer segmentation, const typename TGroundTruthImageType::Pointer gt, const typename TMaskImageType::Pointer mask);
  Eval(double snr,double psnr,long tp=0,long tn=0, long fp=0, long fn=0);

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
  double snr();
  double psnr();
  double sparsity();

 private:
  void countMatchesBinary(const typename TGroundTruthImageType::Pointer gt, const typename TMaskImageType::Pointer mask);
  void  countMatches(const typename TImageType::Pointer segmentation,const typename TGroundTruthImageType::Pointer gt,const typename TMaskImageType::Pointer mask);

  long m_truePositive;
  long m_trueNegative;
  long m_falsePositive;
  long m_falseNegative;
  float m_epsilon;
  double m_hausdorff_distance;
  long m_background;
  long m_foreground;
  long double m_psnr;
  long double m_snr;
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
    << eval.snr() << ","
    << eval.psnr() << std::endl;
    //<< eval.sparsity() <<std::endl;

    return out;
}


#include "bench_evaluation.hxx"

#endif // bench_evaluation_h
