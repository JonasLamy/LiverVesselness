#include "bench_evaluation.h"

// Change QuickView.h for itkViewImage.h
#include "itkViewImage.h"

template <typename TImageType, typename TGroundTruthImageType, typename TMaskImageType>
Eval<TImageType, TGroundTruthImageType,TMaskImageType>::Eval(const typename TImageType::Pointer img, const typename TGroundTruthImageType::Pointer gt, const typename TMaskImageType::Pointer mask)
	: m_truePositive(0), m_trueNegative(0), m_falsePositive(0), m_falseNegative(0)
{
	countMatchesBinary(img, gt, mask);
}

template <typename TImageType, typename TGroundTruthImageType, typename TMaskImageType>
void Eval<TImageType, TGroundTruthImageType,TMaskImageType>::countMatchesBinary(const typename TImageType::Pointer segmentation, const typename TGroundTruthImageType::Pointer gt,const typename TMaskImageType::Pointer mask)
{

	typename itk::ImageRegionConstIterator<TImageType> itImg(segmentation, segmentation->GetLargestPossibleRegion());
	typename itk::ImageRegionConstIterator<TGroundTruthImageType> itGT(gt, gt->GetLargestPossibleRegion());
	typename itk::ImageRegionConstIterator<TMaskImageType> itMask(mask, mask->GetLargestPossibleRegion());

	itImg.GoToBegin();
	itGT.GoToBegin();
	itMask.GoToBegin();

	// debug purpose to insure that both images are in the same coordinate system....
	typename TImageType::Pointer p = TImageType::New();
	p->SetRegions(segmentation->GetLargestPossibleRegion());
	p->Allocate();
	p->FillBuffer(0);

	typename itk::ImageRegionIterator<TImageType> itP(p, p->GetLargestPossibleRegion());
	auto writer = itk::ImageFileWriter<TImageType>::New();

	while (!itImg.IsAtEnd())
	{
		if (itMask.Get() > 0)
		{
			if (itImg.Get() > 0)
			{
				if (itGT.Get() > 0)
				{
					itP.Set(255); // both values
					m_truePositive++;
				}
				else
				{
					itP.Set(100); // only source image
					m_falsePositive++;
				}
			}
			else
			{
				if (itGT.Get() > 0)
				{
					itP.Set(50); // only gt
					m_falseNegative++;
				}
				else
				{
					itP.Set(0); // nothing
					m_trueNegative++;
				}
			}
		}

		++itImg;
		++itGT;
		++itP;
		++itMask;
	}

	writer->SetFileName("verif.nii");
	writer->SetInput(p);
	writer->Update();
}


template <typename TImageType, typename TGroundTruthImageType, typename TMaskImageType>
double Eval<TImageType, TGroundTruthImageType,TMaskImageType>::precision()
{
	return m_truePositive / (long double)(m_truePositive + m_falsePositive);
}

template <typename TImageType, typename TGroundTruthImageType, typename TMaskImageType>
double Eval<TImageType, TGroundTruthImageType,TMaskImageType>::sensitivity()
{
	return m_truePositive / (long double)(m_truePositive + m_falseNegative);
}

template <typename TImageType, typename TGroundTruthImageType, typename TMaskImageType>
double Eval<TImageType, TGroundTruthImageType,TMaskImageType>::specificity()
{
	return m_trueNegative / (long double)(m_trueNegative + m_falsePositive);
}

template <typename TImageType, typename TGroundTruthImageType, typename TMaskImageType>
double Eval<TImageType, TGroundTruthImageType,TMaskImageType>::accuracy()
{
	return (m_truePositive + m_trueNegative) / (long double)(m_truePositive + m_trueNegative + m_falsePositive + m_falseNegative);
}

template <typename TImageType, typename TGroundTruthImageType, typename TMaskImageType>
long double Eval<TImageType, TGroundTruthImageType,TMaskImageType>::matthewsCorrelation()
{
	long double a = (m_truePositive + m_falsePositive);
	a *= (m_truePositive + m_falseNegative);
	a *= (m_trueNegative + m_falsePositive);
	a *= (m_trueNegative + m_falseNegative);

	long double b = (m_truePositive * m_trueNegative) - (m_falsePositive * m_truePositive);

	return b / std::sqrt(a);
}

template <typename TImageType, typename TGroundTruthImageType, typename TMaskImageType>
double Eval<TImageType, TGroundTruthImageType,TMaskImageType>::dice()
{
	return 2 * m_truePositive / (double)(m_falsePositive + m_falseNegative + 2 * m_truePositive);
}

template <typename TImageType, typename TGroundTruthImageType, typename TMaskImageType>
void Eval<TImageType,TGroundTruthImageType,TMaskImageType>::roc(VoxelsMap &vMap)
{
	std::cout<<vMap["TP"].size()<<std::endl;
	
	for(int i=0;i<vMap["TP"].size();i++)
	{
		std::cout<< vMap["TP"][i] << "-" << vMap["FP"][i] <<std::endl;
	}
}




template <typename TImageType, typename TGroundTruthImageType, typename TMaskImageType>
void Eval<TImageType, TGroundTruthImageType,TMaskImageType>::print()
{
	std::cout << "stats\n"
			  << "TP (1,1) :" << m_truePositive << std::endl
			  << "TN (0,0):" << m_trueNegative << std::endl
			  << "FP (1,0):" << m_falsePositive << std::endl
			  << "FN (0,1):" << m_falseNegative << std::endl
			  << std::endl
			  << "Sensitivity:" << sensitivity() << std::endl
			  << "Specificity:" << specificity() << std::endl
			  << "Precision:" << precision() << std::endl
			  << "Accuracy:" << accuracy() << std::endl
			  << "Matthews correlation:" << matthewsCorrelation() << std::endl
			  << "Dice:" << dice() << std::endl;
}
