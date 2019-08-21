#ifndef itkHessianToJermanMeasureImageFilter_hxx
#define itkHessianToJermanMeasureImageFilter_hxx

#include "itkHessianToJermanMeasureImageFilter.h"
#include "itkStatisticsImageFilter.h"

namespace itk{
    template< typename TInputImage,typename TOutputImage>
    HessianToJermanMeasureImageFilter<TInputImage, TOutputImage>::HessianToJermanMeasureImageFilter()
    {
        //this->DynamicMultiThreadingOn();
    }

    template< typename TInputImage,typename TOutputImage>
    void HessianToJermanMeasureImageFilter<TInputImage,TOutputImage>::VerifyPreconditions() ITKv5_CONST
    {
        Superclass::VerifyPreconditions();
        if ( ImageDimension != 3 )
        {
        itkExceptionMacro("Image Dimension must be 3");
        }
    }

    template<typename TInputImage, typename TOutputImage>
    void HessianToJermanMeasureImageFilter<TInputImage,TOutputImage>::BeforeThreadedGenerateData()
    {
    }

    template<typename TInputImage,typename TOutputImage>
    void HessianToJermanMeasureImageFilter<TInputImage,TOutputImage>::DynamicThreadedGenerateData(const OutputImageRegionType & outputRegionForThread)
    {
    }

    template<typename TInputImage,typename TOutputImage>
    void HessianToJermanMeasureImageFilter<TInputImage, TOutputImage>::GenerateData()
    {   
        OutputImageType * output = this->GetOutput();
        const InputImageType* input = this->GetInput();

        //input->SetRequestedRegionToLargestPossibleRegion();

        OutputImageRegionType outputRegion;
        outputRegion.SetSize( input->GetLargestPossibleRegion().GetSize() );
        output->SetBufferedRegion(outputRegion);
        output->Allocate();
        
        typename InputImageType::SizeType iSize = input->GetLargestPossibleRegion().GetSize();
        // Calculator for computing of eigen values
        using CalculatorType = SymmetricEigenAnalysisFixedDimension<ImageDimension, InputPixelType, EigenValueArrayType>;
        CalculatorType eigenCalculator;
        // Walk the region of eigen values and get the objectness measure
        ImageRegionConstIterator< InputImageType > it(input, input->GetLargestPossibleRegion());
        ImageRegionIterator< OutputImageType >     oit(output, output->GetLargestPossibleRegion());

        oit.GoToBegin();
        it.GoToBegin();

        EigenValueType maxLambda3 = 0;
        EigenValueType lambdaRho = 0;
        OutputPixelType vesselnessMeasure = 0.0;

        EigenValueArrayType sortedEigenValues;
        std::cout<<"computing eigenvalues"<<std::endl;
        long nbPixels = 0;
        long nbTotalPixels = iSize[0] * iSize[1] * iSize[2] ;
        while( !it.IsAtEnd() )
        {
            // Compute eigen values
            EigenValueArrayType eigenValues;
            eigenCalculator.ComputeEigenValues(it.Get(), eigenValues);
            // sort eigenValues by magnitude but retain their sign.
            // The eigenvalues are to be sorted |e1|<=|e2|<=...<=|eN|
            sortedEigenValues = eigenValues;
            std::sort( sortedEigenValues.Begin(), sortedEigenValues.End(), AbsLessEqualCompare() );

            // only second and third eigen values are useful
            if( m_BrightObject)
            {
                sortedEigenValues[1] *= -1;
                sortedEigenValues[2] *= -1;
            }

            // keeping max lambda3 value accross the whole image
            if( maxLambda3 < sortedEigenValues[2] )
            {
                maxLambda3 = sortedEigenValues[2];
            };

            nbPixels++;
            if(nbPixels % 500000 == 0)
                std::cout<<"----------"<<std::endl
                    <<"nbPixels :"<<nbPixels<<" out of "<<nbTotalPixels<<std::endl
                    <<it.Get()<<std::endl
                    <<eigenValues<<std::endl
                    <<sortedEigenValues<<std::endl;

            ++it;
        }

        lambdaRho = maxLambda3 * m_Tau;
        std::cout<<"max lambda:"<< maxLambda3<<std::endl;
        std::cout<<"lambda rho:"<< lambdaRho<<std::endl;

        it.GoToBegin();
        nbPixels = 0;
        long modulo = 500000;
        while(!it.IsAtEnd())
        {
            // Compute Jerman's volume ratio here....
            // Set pixel value in oit...
            if( nbPixels % modulo == 0)
                std::cout<< sortedEigenValues<<std::endl;

            if( sortedEigenValues[1] <= 0 || lambdaRho <= 0)
            {

                oit.Set( NumericTraits< OutputPixelType >::ZeroValue() );
                ++it;
                ++oit;
                nbPixels++;
                if(nbPixels % modulo == 0)
                std::cout<<"vesselnessValue zero :"<<vesselnessMeasure<<std::endl;
                continue;
            }
            if( sortedEigenValues[1] >= lambdaRho /2 && lambdaRho/2 > 0)
            {
                oit.Set(NumericTraits< OutputPixelType >::OneValue() );
                ++it;
                ++oit;
                nbPixels++;
                if(nbPixels % modulo == 0)
                std::cout<<"vesselnessValue one :"<<vesselnessMeasure<<std::endl;
                continue;
            }
            
            // Jerman's ratio
            vesselnessMeasure = sortedEigenValues[1] * sortedEigenValues[1] * (lambdaRho - sortedEigenValues[1]);
            vesselnessMeasure *= std::pow( 3/(sortedEigenValues[1] + lambdaRho), 3);  

            oit.Set( vesselnessMeasure);
            ++it;
            ++oit;

            nbPixels++;
            //if(nbPixels % 500000 == 0)
            std::cout<<"vesselnessValue lambda:"<<vesselnessMeasure<<std::endl;
        }

        auto stats = StatisticsImageFilter<TOutputImage>::New();
        stats->SetInput(output);
        stats->Update();

        std::cout<<"min"
        <<stats->GetMinimum()<<std::endl
        <<"mean:"<<stats->GetMean()<<std::endl
        <<"max:"<<stats->GetMaximum()<<std::endl;
    }

    template<typename TInputImage,typename TOutputImage>
    void HessianToJermanMeasureImageFilter<TInputImage, TOutputImage>
    ::PrintSelf(std::ostream & os, Indent indent) const
    {
        Superclass::PrintSelf(os,indent);
        os << indent << "Tau " << m_Tau << std::endl; 
    }

}

#endif // itkHessianToJermanMeasureImageFilter_hxx