#ifndef itkHessianToFrangiMeasureImageFilter_hxx
#define itkHessianToFrangiMeasureImageFilter_hxx

#include "itkHessianToFrangiMeasureImageFilter.h"
#include "itkStatisticsImageFilter.h"

namespace itk{
    template< typename TInputImage,typename TOutputImage, typename TMaskImage>
    HessianToFrangiMeasureImageFilter<TInputImage, TOutputImage,TMaskImage>::HessianToFrangiMeasureImageFilter()
    {
        //this->DynamicMultiThreadingOn();
        std::cout<<"Using Hessian Frangi Image Filter"<<std::endl;
    }

    template<typename TInputImage,typename TOutputImage, typename TMaskImage>
    void HessianToFrangiMeasureImageFilter<TInputImage, TOutputImage,TMaskImage>::DynamicThreadedGenerateData(const OutputImageRegionType & outputRegionForThread)
    {
        OutputImageType * output = this->GetOutput();
        const InputImageType* input = this->GetInput();

    }

    template<typename TInputImage,typename TOutputImage, typename TMaskImage>
    void HessianToFrangiMeasureImageFilter<TInputImage, TOutputImage,TMaskImage>::GenerateData()
    {   
        if(this->m_maskImage)
            withMask();
        else
            noMask();
    }

    template<typename TInputImage,typename TOutputImage, typename TMaskImage>
    void HessianToFrangiMeasureImageFilter<TInputImage, TOutputImage,TMaskImage>::BeforeThreadedGenerateData()
    {   
        if(this->m_maskImage)
            withMask();
        else
            noMask();
    }
    
    template<typename TInputImage,typename TOutputImage, typename TMaskImage>
    void HessianToFrangiMeasureImageFilter<TInputImage, TOutputImage,TMaskImage>
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

        OutputPixelType vesselnessMeasure = NumericTraits< OutputPixelType >::ZeroValue();

        std::cout<<"computing eigenvalues"<<std::endl;
        EigenValueType lambda1;
        EigenValueType lambda2;
        EigenValueType lambda3;

        float Ra = 0;
        float Rb = 0;
        float S = 0;
        float C = ptr_filter->GetMaxEigenValueNorm() / 2.0f;

        std::cout<<"highest norm withMask: "<< ptr_filter->GetMaxEigenValueNorm() << std::endl;
        std::cout<<"C withMask: "<< C << std::endl;
        std::cout<<"gamma withMask: "<< 2*C*C << std::endl;
        
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

                        

            if( lambda2 > 0 or lambda3 > 0) // not interested in dark objects
            {
                oit.Set( NumericTraits< OutputPixelType >::ZeroValue() );
                ++oit;
                ++itEV;
                ++itMask;
                continue;
            }
            
            Ra = std::abs(lambda2) / std::abs(lambda3);
            Rb = std::abs(lambda1) / std::sqrt( std::abs(lambda2 * lambda3) );
            S = sqrt( lambda1 * lambda1 + lambda2 * lambda2 + lambda3 * lambda3 );
            
            // Frangi's ratio
            vesselnessMeasure =  ( 1 - std::exp( -(Ra*Ra)/(2*m_Alpha*m_Alpha) ) );
            vesselnessMeasure *= std::exp( -(Rb*Rb)/(2*m_Beta*m_Beta) );
            vesselnessMeasure *= ( 1 - std::exp( -(S*S)/(2*C*C) ) );    

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

    template<typename TInputImage,typename TOutputImage, typename TMaskImage>
    void HessianToFrangiMeasureImageFilter<TInputImage, TOutputImage,TMaskImage>
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
        OutputPixelType vesselnessMeasure = NumericTraits< OutputPixelType >::ZeroValue();

        std::cout<<"computing eigenvalues"<<std::endl;
        EigenValueType lambda1;
        EigenValueType lambda2;
        EigenValueType lambda3;

        float Ra = 0;
        float Rb = 0;
        float S = 0;
        float C = ptr_filter->GetMaxEigenValueNorm() / 2.0f;

        std::cout<<"highest norm no Mask: "<< C << std::endl;
        
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

            if( lambda2 > 0 or lambda3 > 0) // not interested in dark objects
            {
                oit.Set( NumericTraits< OutputPixelType >::ZeroValue() );
                ++oit;
                ++itEV;
                continue;
            }
            
            Ra = std::abs(lambda2) / std::abs(lambda3);
            Rb = std::abs(lambda1) / std::sqrt( std::abs(lambda2 * lambda3) );
            S = sqrt( lambda1 * lambda1 + lambda2 * lambda2 + lambda3 * lambda3 );
            
            // Frangi's ratio
            vesselnessMeasure =  1 - std::exp( -(Ra*Ra)/(2*m_Alpha*m_Alpha) );
            vesselnessMeasure *= std::exp( -(Rb*Rb)/(2*m_Beta*m_Beta) );
            vesselnessMeasure *= 1-std::exp( -(S*S)/(2*C*C) ); 

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

    template<typename TInputImage,typename TOutputImage, typename TMaskImage>
    void HessianToFrangiMeasureImageFilter<TInputImage, TOutputImage,TMaskImage>
    ::PrintSelf(std::ostream & os, Indent indent) const
    {
        Superclass::PrintSelf(os,indent);
        os << indent << "Alpha: " << m_Alpha << " Beta: "<< m_Beta << std::endl; 
    }

}

#endif // itkHessianToFrangiMeasureImageFilter_hxx