#ifndef itkHessianToMeijeringMeasureImageFilter_hxx
#define itkHessianToMeijeringMeasureImageFilter_hxx

#include "itkHessianToMeijeringMeasureImageFilter.h"
#include "itkStatisticsImageFilter.h"

namespace itk{
    template< typename TInputImage,typename TOutputImage,typename TMaskImage>
    HessianToMeijeringMeasureImageFilter<TInputImage, TOutputImage, TMaskImage>::HessianToMeijeringMeasureImageFilter()
    :m_BrightObject(true),m_Alpha(-0.33)
    {
        //this->DynamicMultiThreadingOn();
    }

    template< typename TInputImage,typename TOutputImage,typename TMaskImage>
    void HessianToMeijeringMeasureImageFilter<TInputImage,TOutputImage, TMaskImage>::VerifyPreconditions() ITKv5_CONST
    {
        Superclass::VerifyPreconditions();
        if ( ImageDimension != 3 )
        {
        itkExceptionMacro("Image Dimension must be 3");
        }
    }

    template<typename TInputImage, typename TOutputImage,typename TMaskImage>
    void HessianToMeijeringMeasureImageFilter<TInputImage,TOutputImage, TMaskImage>::BeforeThreadedGenerateData()
    {
    }

    template<typename TInputImage,typename TOutputImage,typename TMaskImage>
    void HessianToMeijeringMeasureImageFilter<TInputImage,TOutputImage, TMaskImage>::DynamicThreadedGenerateData(const OutputImageRegionType & outputRegionForThread)
    {
        OutputImageType * output = this->GetOutput();
        const InputImageType* input = this->GetInput();

    }

    template<typename TInputImage,typename TOutputImage,typename TMaskImage>
    void HessianToMeijeringMeasureImageFilter<TInputImage, TOutputImage, TMaskImage>::GenerateData()
    {   
        if(this->m_maskImage)
            withMask();
        else
            noMask();
    }

    template<typename TInputImage,typename TOutputImage,typename TMaskImage>
    void HessianToMeijeringMeasureImageFilter<TInputImage, TOutputImage, TMaskImage>
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
        auto ptr_filter = ModifiedHessianToEigenValuesImageFilter<TInputImage>::New();
        ptr_filter->SetMaskImage(m_maskImage);
        ptr_filter->SetInput(input);
        ptr_filter->SetAlpha(m_Alpha);
        ptr_filter->Update();

        auto eigenValuesImage = ptr_filter->GetOutput();

        EigenValueType minLambda = ptr_filter->GetMinEigenValue();
        OutputPixelType neuritenessMesure = NumericTraits< OutputPixelType >::ZeroValue();

        EigenValueType lambda1;
        EigenValueType lambda2;
        EigenValueType lambda3;
        
        // Walk the region of eigen values and get the objectness measure
        ImageRegionConstIterator< Image<EigenValueArrayType,3> > itEV(eigenValuesImage, eigenValuesImage->GetLargestPossibleRegion());
        ImageRegionConstIterator< MaskImageType > itMask(m_maskImage, m_maskImage->GetLargestPossibleRegion());

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

            // Meijering's ratio
            if( lambda3 >= 0 )
                neuritenessMesure = 0;
            else
                neuritenessMesure = lambda3 /  minLambda;
            
            oit.Set( neuritenessMesure);   

            ++oit;
            ++itEV;
            ++itMask;
        }
    }
    
    template<typename TInputImage,typename TOutputImage,typename TMaskImage>
    void HessianToMeijeringMeasureImageFilter<TInputImage, TOutputImage, TMaskImage>
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
        auto ptr_filter = ModifiedHessianToEigenValuesImageFilter<TInputImage>::New();
        ptr_filter->SetMaskImage(m_maskImage);
        ptr_filter->SetInput(input);
        ptr_filter->SetAlpha(m_Alpha);
        ptr_filter->Update();

        auto eigenValuesImage = ptr_filter->GetOutput();

        EigenValueType minLambda = ptr_filter->GetMinEigenValue();
        OutputPixelType neuritenessMesure = NumericTraits< OutputPixelType >::ZeroValue();

        EigenValueType lambda1;
        EigenValueType lambda2;
        EigenValueType lambda3;
        
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

            // Meijering's ratio
            if( lambda3 > 0 )
                neuritenessMesure = 0;
            else
                neuritenessMesure = lambda3 /  minLambda;
            
            oit.Set( neuritenessMesure);   

            ++oit;
            ++itEV;
        }


        auto stats = StatisticsImageFilter<TOutputImage>::New();
        stats->SetInput(output);
        stats->Update();

    }

    template<typename TInputImage,typename TOutputImage,typename TMaskImage>
    void HessianToMeijeringMeasureImageFilter<TInputImage, TOutputImage, TMaskImage>
    ::PrintSelf(std::ostream & os, Indent indent) const
    {
        Superclass::PrintSelf(os,indent);
        os << indent << "Alpha " << m_Alpha << std::endl; 
    }

}

#endif // itkHessianToMeijeringMeasureImageFilter_hxx