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
        // Grafting and allocating data
        OutputImageType * output = this->GetOutput();
        const InputImageType* input = this->GetInput();

        OutputImageRegionType outputRegion;
        outputRegion.SetSize( input->GetLargestPossibleRegion().GetSize() );
        output->SetBufferedRegion(outputRegion);
        output->Allocate();

        // computing eigenValues
        auto ptr_filter = HessianToEigenValuesImageFilter<TInputImage>::New();
        ptr_filter->SetInput(input);
        ptr_filter->Update();

        auto eigenValuesImage = ptr_filter->GetOutput();
        std::cout<<"max eigen value:"<<ptr_filter->GetMaxEigenValue()<<std::endl;

        EigenValueType maxLambda3 = ptr_filter->GetMaxEigenValue();
        EigenValueType lambdaRho = maxLambda3 * m_Tau;
        EigenValueType v = NumericTraits< EigenValueType >::ZeroValue();
        OutputPixelType vesselnessMeasure = NumericTraits< OutputPixelType >::ZeroValue();

        std::cout<<"computing eigenvalues"<<std::endl;
        EigenValueArrayType eigenValues;
        long nbPixels = 0;
        std::vector<EigenValueArrayType> vEigenValues;

        // additionnal infos
        std::cout<<"max lambda 3:"<< maxLambda3<<std::endl;
        std::cout<<"tau :"<<m_Tau<<std::endl;
        std::cout<<"lambda rho:"<< lambdaRho<<std::endl;

        // Walk the region of eigen values and get the objectness measure
        ImageRegionConstIterator< Image<EigenValueArrayType,3> > itEV(eigenValuesImage, eigenValuesImage->GetLargestPossibleRegion());
        ImageRegionIterator< OutputImageType >     oit(output, output->GetLargestPossibleRegion());
        oit.GoToBegin();
        itEV.GoToBegin();
        while( !itEV.IsAtEnd() )
        {   
            eigenValues = itEV.Value();

            if( m_BrightObject)
            {
                eigenValues[1] *= -1.0f;
                eigenValues[2] *= -1.0f;
            }

            if( eigenValues[1] <= 0.0f || lambdaRho <= 0.0f)
            {
                oit.Set( NumericTraits< OutputPixelType >::ZeroValue() );
                ++oit;
                ++itEV;
                continue;
            }
            if( eigenValues[1] >= lambdaRho /2.0f && lambdaRho > 0.0f)
            {
                oit.Set(NumericTraits< OutputPixelType >::OneValue() );
                ++oit;
                ++itEV;
                continue;
            }
            
            // Jerman's ratio
            // lambda2^2 * ( lambdaP - lambda2 ) * [3/(lambdaP + lambda2)]^3
            vesselnessMeasure = eigenValues[1] * eigenValues[1] * (lambdaRho - eigenValues[1]); // lambda2^2 * ( lambdaP - lambda2 )
            vesselnessMeasure *= 27.0f / ( (eigenValues[1] + lambdaRho) * (eigenValues[1] + lambdaRho) * (eigenValues[1] + lambdaRho) ); // [3/(lambdaP + lambda2)]^3

            oit.Set( vesselnessMeasure);

            ++oit;
            ++itEV;
        }

        std::cout<<"before stats"<<std::endl;

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