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
        OutputImageType * output = this->GetOutput();
        const InputImageType* input = this->GetInput();

    }

    template<typename TInputImage,typename TOutputImage>
    void HessianToJermanMeasureImageFilter<TInputImage, TOutputImage>::GenerateData()
    {   
        OutputImageType * output = this->GetOutput();
        const InputImageType* input = this->GetInput();

        auto ptr_filter = HessianToEigenValuesImageFilter<TInputImage>::New();
        ptr_filter->SetInput(input);
        ptr_filter->Update();
        std::cout<<"max eigen value:"<<ptr_filter->GetMaxEigenValue()<<std::endl;

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

        EigenValueType maxLambda3 = NumericTraits< EigenValueType >::ZeroValue();
        EigenValueType maxLambda2 = NumericTraits< EigenValueType >::ZeroValue();
        EigenValueType lambdaRho = NumericTraits< EigenValueType >::ZeroValue();
        EigenValueType v = NumericTraits< EigenValueType >::ZeroValue();
        OutputPixelType vesselnessMeasure = NumericTraits< OutputPixelType >::ZeroValue();

        std::cout<<"computing eigenvalues"<<std::endl;
        long nbTotalPixels = iSize[0] * iSize[1] * iSize[2] ;
        EigenValueArrayType eigenValues;
        long nbPixels = 0;
        std::vector<EigenValueArrayType> vEigenValues;
        while( !it.IsAtEnd() )
            // Compute eigen values
        {   
            /*
            for(int i=0;i<4;i++)
            {
                if(it.Get()[i] > 2e-4 )
                    std::cout<<it.Get()[i]<<" ";
                else
                    std::cout<<0<<" ";
            }
            */
            //std::cout<<std::endl;
            // sort eigenValues by magnitude but retain their sign.
            // The eigenvalues are to be sorted |e1|<=|e2|<=...<=|eN|

            eigenCalculator.SetOrderEigenMagnitudes(true);
            eigenCalculator.ComputeEigenValues(it.Get(), eigenValues);
            // noise removal on eigenValues
            if( std::isinf(eigenValues[0]) || abs(eigenValues[0]) < 1e-4){ eigenValues[0] = 0; }
            if( std::isinf(eigenValues[1]) || abs(eigenValues[1]) < 1e-4){ eigenValues[1] = 0; }
            if( std::isinf(eigenValues[2]) || abs(eigenValues[2]) < 1e-4){ eigenValues[2] = 0; }
                
            
            // only second and third eigen values are useful
            if( m_BrightObject)
            {
                eigenValues[1] *= -1;
                eigenValues[2] *= -1;
            }
            // keeping max lambda3 value accross the whole image
            if( maxLambda3 < eigenValues[2] )
            {
                maxLambda3 = eigenValues[2];
            };
            // keeping max Lambda2 for debug
            if( maxLambda2 < eigenValues[1] )
            {
                maxLambda2 = eigenValues[1];
            };

            // simple process notification
            if(nbPixels % 1000000 == 0)
                std::cout<<nbPixels<<" out of "<<nbTotalPixels<<std::endl;
            vEigenValues.push_back(eigenValues);
            nbPixels++;

            ++it;
        }

        lambdaRho = maxLambda3 * m_Tau;

        // additionnal infos
        std::cout<<"max lambda 3:"<< maxLambda3<<std::endl;
        std::cout<<"max lambda 2:"<< maxLambda2<<std::endl;
        std::cout<<"tau :"<<m_Tau<<std::endl;
        std::cout<<"lambda rho:"<< lambdaRho<<std::endl;
        std::cout<<"EigenValues vector size:"<<vEigenValues.size()<<std::endl;
        std::cout<<"sample"<<vEigenValues[0]<<std::endl;

        for(int i=0; i<vEigenValues.size();i++)
        {
            eigenValues = vEigenValues[i];

            if( eigenValues[1] <= 0 || lambdaRho <= 0)
            {
                oit.Set( NumericTraits< OutputPixelType >::ZeroValue() );
                ++oit;
                continue;
            }
            if( eigenValues[1] >= lambdaRho /2.0 && lambdaRho > 0)
            {
                oit.Set(NumericTraits< OutputPixelType >::OneValue() );
                ++oit;
                continue;
            }
            
            // Jerman's ratio
            // lambda2^2 * ( lambdaP - lambda2 ) * [3/(lambdaP + lambda2)]^3
            vesselnessMeasure = eigenValues[1] * eigenValues[1] * (lambdaRho - eigenValues[1]); // lambda2^2 * ( lambdaP - lambda2 )
            vesselnessMeasure *= 27 / ( (eigenValues[1] + lambdaRho) * (eigenValues[1] + lambdaRho) * (eigenValues[1] + lambdaRho) ); // [3/(lambdaP + lambda2)]^3

            oit.Set( vesselnessMeasure);
            ++oit;
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