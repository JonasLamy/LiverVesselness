#ifndef itkHessianToMeasureImageFilter_hxx
#define itkHessianToMeasureImageFilter_hxx

#include "itkHessianToMeasureImageFilter.h"
#include "itkStatisticsImageFilter.h"

namespace itk{
    template< typename TInputImage,typename TOutputImage, typename TMaskImage>
    HessianToMeasureImageFilter<TInputImage, TOutputImage,TMaskImage>::HessianToMeasureImageFilter()
    :m_maskImage(nullptr)
    {
        //this->DynamicMultiThreadingOn();
        std::cout<<"Using Hessian Measure Image Filter"<<std::endl;
    }

    template< typename TInputImage,typename TOutputImage, typename TMaskImage>
    void HessianToMeasureImageFilter<TInputImage, TOutputImage,TMaskImage>::VerifyPreconditions() ITKv5_CONST
    {
        Superclass::VerifyPreconditions();
        if ( ImageDimension != 3 )
        {
        itkExceptionMacro("Image Dimension must be 3");
        }
    }

    template<typename TInputImage, typename TOutputImage, typename TMaskImage>
    void HessianToMeasureImageFilter<TInputImage, TOutputImage,TMaskImage>::BeforeThreadedGenerateData()
    {
    }

    template<typename TInputImage,typename TOutputImage, typename TMaskImage>
    void HessianToMeasureImageFilter<TInputImage, TOutputImage,TMaskImage>::DynamicThreadedGenerateData(const OutputImageRegionType & outputRegionForThread)
    {
        OutputImageType * output = this->GetOutput();
        const InputImageType* input = this->GetInput();

    }

    template<typename TInputImage,typename TOutputImage, typename TMaskImage>
    void HessianToMeasureImageFilter<TInputImage, TOutputImage,TMaskImage>::GenerateData()
    {   
        if( m_maskImage)
        {
            withMask();
        }
        else
        {
            noMask();
        }
    }

    template<typename TInputImage,typename TOutputImage, typename TMaskImage>
    void HessianToMeasureImageFilter<TInputImage, TOutputImage,TMaskImage>::noMask()
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
        ptr_filter->SetMaskImage(m_maskImage);
        ptr_filter->SetInput(input);
        ptr_filter->Update();

        auto eigenValuesImage = ptr_filter->GetOutput();
        std::cout<<"max eigen value:"<<ptr_filter->GetMaxEigenValue()<<std::endl;

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

            
            oit.Set( lambda1 + lambda2 + lambda3 ); // default value
            
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
    void HessianToMeasureImageFilter<TInputImage, TOutputImage,TMaskImage>::withMask()
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
        ptr_filter->SetMaskImage(m_maskImage);
        ptr_filter->SetInput(input);
        ptr_filter->Update();

        auto eigenValuesImage = ptr_filter->GetOutput();
        std::cout<<"max eigen value:"<<ptr_filter->GetMaxEigenValue()<<std::endl;

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
        ImageRegionConstIterator<TMaskImage> itMask(m_maskImage, m_maskImage->GetLargestPossibleRegion() );
        ImageRegionIterator< OutputImageType >     oit(output, output->GetLargestPossibleRegion());
        
        
        oit.GoToBegin();
        itEV.GoToBegin();
        itMask.GoToBegin();
        while( !itEV.IsAtEnd() )
        {   
            if(itMask.Get() == 0 )
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
            
            oit.Set( lambda1 + lambda2 + lambda3 ); // default value
            
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
    void HessianToMeasureImageFilter<TInputImage, TOutputImage,TMaskImage>
    ::PrintSelf(std::ostream & os, Indent indent) const
    {
        Superclass::PrintSelf(os,indent);
        os << indent << "Base Measure Image filter" << std::endl; 
    }

}

#endif // itkHessianToMeasureImageFilter_hxx