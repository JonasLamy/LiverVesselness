#ifndef itkHessianToRuiZhangMeasureImageFilter_hxx
#define itkHessianToRuiZhangMeasureImageFilter_hxx

#include "itkHessianToRuiZhangMeasureImageFilter.h"
#include "itkStatisticsImageFilter.h"

namespace itk{
    template< typename TInputImage,typename TOutputImage, typename TMaskImage>
    HessianToRuiZhangMeasureImageFilter<TInputImage, TOutputImage, TMaskImage>::HessianToRuiZhangMeasureImageFilter()
    {
        //this->DynamicMultiThreadingOn();
    }

    template< typename TInputImage, typename TOutputImage, typename TMaskImage>
    void HessianToRuiZhangMeasureImageFilter<TInputImage,TOutputImage,TMaskImage>::BeforeThreadedGenerateData()
    {
    }

    template< typename TInputImage, typename TOutputImage, typename TMaskImage>
    void HessianToRuiZhangMeasureImageFilter<TInputImage,TOutputImage,TMaskImage>::DynamicThreadedGenerateData(const OutputImageRegionType & outputRegionForThread)
    {
        OutputImageType * output = this->GetOutput();
        const InputImageType* input = this->GetInput();

    }

    template< typename TInputImage, typename TOutputImage, typename TMaskImage>
    void HessianToRuiZhangMeasureImageFilter<TInputImage, TOutputImage, TMaskImage>::GenerateData()
    {   
        if(this->m_maskImage)
            withMask();
        else
            noMask();
    }

    template< typename TInputImage, typename TOutputImage, typename TMaskImage>
    void HessianToRuiZhangMeasureImageFilter<TInputImage, TOutputImage, TMaskImage>
    ::noMask()
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
        //std::cout<<"max eigen value:"<<ptr_filter->GetMaxEigenValue()<<std::endl;

        EigenValueType maxLambda3 = ptr_filter->GetMaxEigenValue();
        EigenValueType lambdaRho = maxLambda3 * m_Tau;
        OutputPixelType vesselnessMeasure = NumericTraits< OutputPixelType >::ZeroValue();

        std::cout<<"computing eigenvalues"<<std::endl;
        EigenValueType lambda1;
        EigenValueType lambda2;
        EigenValueType lambda3;
        
        // additionnal infos
        //std::cout<<"max lambda 3:"<< maxLambda3<<std::endl;
        //std::cout<<"tau :"<<m_Tau<<std::endl;
        //std::cout<<"lambda rho:"<< lambdaRho<<std::endl;

        // Walk the region of eigen values and get the objectness measure
        ImageRegionConstIterator< Image<EigenValueArrayType,3> > itEV(eigenValuesImage, eigenValuesImage->GetLargestPossibleRegion());
        ImageRegionIterator< OutputImageType >     oit(output, output->GetLargestPossibleRegion());
        oit.GoToBegin();
        itEV.GoToBegin();
        while( !itEV.IsAtEnd() )
        {   
            lambda1 = itEV.Value()[0];
            lambda2 = itEV.Value()[1];
            lambda3 = itEV.Value()[2];

            if( m_BrightObject) // if brightObject, then eigen values are negatives
            {
                lambda2 *= -1.0f;
                lambda3 *= -1.0f;
            }

            if( lambda3 > maxLambda3 * m_Tau)
                lambdaRho = lambda3;
            else
            {
                if( lambda3 > 0 )
                {
                    lambdaRho = maxLambda3 * m_Tau;
                }
                else
                {
                    lambdaRho = 0;
                }
            } 

            if( lambda2 <= 0.0f || lambdaRho <= 0.0f) // not interested in dark objects
            {
                oit.Set( NumericTraits< OutputPixelType >::ZeroValue() );
                ++oit;
                ++itEV;
                continue;
            }
            if( lambda2 >= lambdaRho /2.0f && lambdaRho > 0.0f) 
            {
                //exp(-(lambda2/lambdaRho)*( 1-exp() ) )
                oit.Set(NumericTraits< OutputPixelType >::OneValue() );
                ++oit;
                ++itEV;
                continue;
            }
            
            // RuiZhang's ratio
            // lambda2^2 * ( lambdaP - lambda2 ) * [3/(lambdaP + lambda2)]^3 * (1-exp(-R^2 / 2y)) with y=lambdaRho/3
            vesselnessMeasure = lambda2* lambda2 * (lambdaRho - lambda2); // lambda2^2 * ( lambdaP - lambda2 )
            vesselnessMeasure *= ( 1 - exp( -(lambda1 * lambda1 + lambda2 * lambda2 + lambdaRho * lambdaRho ) / ( 2 * lambdaRho / 3) ) ); // (1-exp(-R^2 / 2y)) see paper for R and y
            vesselnessMeasure *= 27.0f / ( (lambda2 + lambdaRho) * (lambda2 + lambdaRho) * (lambda2 + lambdaRho) ); // [3/(lambdaP + lambda2)]^3

            oit.Set( vesselnessMeasure);   
            

            ++oit;
            ++itEV;
        }

        std::cout<<"vesselness stats"<<std::endl;

        auto stats = StatisticsImageFilter<TOutputImage>::New();
        stats->SetInput(output);
        stats->Update();

        std::cout<<"min"
        <<stats->GetMinimum()<<std::endl
        <<"mean:"<<stats->GetMean()<<std::endl
        <<"max:"<<stats->GetMaximum()<<std::endl;
    }

    template< typename TInputImage, typename TOutputImage, typename TMaskImage>
    void HessianToRuiZhangMeasureImageFilter<TInputImage, TOutputImage, TMaskImage>
    ::withMask()
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
        ptr_filter->SetMaskImage(this->m_maskImage);
        ptr_filter->SetInput(input);
        ptr_filter->Update();

        auto eigenValuesImage = ptr_filter->GetOutput();
        //std::cout<<"max eigen value:"<<ptr_filter->GetMaxEigenValue()<<std::endl;

        EigenValueType maxLambda3 = ptr_filter->GetMaxEigenValue();
        EigenValueType lambdaRho = maxLambda3 * m_Tau;
        OutputPixelType vesselnessMeasure = NumericTraits< OutputPixelType >::ZeroValue();

        std::cout<<"computing eigenvalues"<<std::endl;
        EigenValueType lambda1;
        EigenValueType lambda2;
        EigenValueType lambda3;
        
        // additionnal infos
        //std::cout<<"max lambda 3:"<< maxLambda3<<std::endl;
        //std::cout<<"tau :"<<m_Tau<<std::endl;
        //std::cout<<"lambda rho:"<< lambdaRho<<std::endl;

        // Walk the region of eigen values and get the objectness measure
        ImageRegionConstIterator< Image<EigenValueArrayType,3> > itEV(eigenValuesImage, eigenValuesImage->GetLargestPossibleRegion());
        ImageRegionConstIterator< MaskImageType > itMask(this->m_maskImage, this->m_maskImage->GetLargestPossibleRegion() );
        ImageRegionIterator< OutputImageType >     oit(output, output->GetLargestPossibleRegion());

        oit.GoToBegin();
        itEV.GoToBegin();
        itMask.GoToBegin();

        while( !itEV.IsAtEnd() )
        {   
            if( itMask.Get() == 0)
            {
                oit.Set(0);
                ++oit;
                ++itEV;
                ++itMask;
                continue;
            }


            lambda1 = itEV.Value()[0];
            lambda2 = itEV.Value()[1];
            lambda3 = itEV.Value()[2];

            if( m_BrightObject) // if brightObject, then eigen values are negatives
            {
                lambda2 *= -1.0f;
                lambda3 *= -1.0f;
            }

            if( lambda3 > maxLambda3 * m_Tau)
                lambdaRho = lambda3;
            else
            {
                if( lambda3 > 0 )
                {
                    lambdaRho = maxLambda3 * m_Tau;
                }
                else
                {
                    lambdaRho = 0;
                }
            } 

            if( lambda2 <= 0.0f || lambdaRho <= 0.0f) // not interested in dark objects
            {
                oit.Set( NumericTraits< OutputPixelType >::ZeroValue() );
                ++oit;
                ++itEV;
                ++itMask;
                continue;
            }
            if( lambda2 >= lambdaRho /2.0f && lambdaRho > 0.0f) 
            {
                //exp(-(lambda2/lambdaRho)*( 1-exp() ) )
                oit.Set(NumericTraits< OutputPixelType >::OneValue() );
                ++oit;
                ++itEV;
                ++itMask;
                continue;
            }
            
            // RuiZhang's ratio
            // lambda2^2 * ( lambdaP - lambda2 ) * [3/(lambdaP + lambda2)]^3 * (1-exp(-R^2 / 2y)) with y=lambdaRho/3
            vesselnessMeasure = lambda2* lambda2 * (lambdaRho - lambda2); // lambda2^2 * ( lambdaP - lambda2 )
            vesselnessMeasure *= ( 1 - exp( -(lambda1 * lambda1 + lambda2 * lambda2 + lambdaRho * lambdaRho ) / ( 2 * lambdaRho / 3) ) ); // (1-exp(-R^2 / 2y)) see paper for R and y
            vesselnessMeasure *= 27.0f / ( (lambda2 + lambdaRho) * (lambda2 + lambdaRho) * (lambda2 + lambdaRho) ); // [3/(lambdaP + lambda2)]^3

            oit.Set( vesselnessMeasure);   
            

            ++oit;
            ++itEV;
            ++itMask;
        }

        std::cout<<"vesselness stats"<<std::endl;

        auto stats = StatisticsImageFilter<TOutputImage>::New();
        stats->SetInput(output);
        stats->Update();

        std::cout<<"min"
        <<stats->GetMinimum()<<std::endl
        <<"mean:"<<stats->GetMean()<<std::endl
        <<"max:"<<stats->GetMaximum()<<std::endl;
    }

    template< typename TInputImage, typename TOutputImage, typename TMaskImage>
    void HessianToRuiZhangMeasureImageFilter<TInputImage, TOutputImage, TMaskImage>
    ::PrintSelf(std::ostream & os, Indent indent) const
    {
        Superclass::PrintSelf(os,indent);
        os << indent << "Tau " << m_Tau << std::endl; 
    }

}

#endif // itkHessianToRuiZhangMeasureImageFilter_hxx