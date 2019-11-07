#ifndef itkHessianToMeijeringMeasureImageFilter_hxx
#define itkHessianToMeijeringMeasureImageFilter_hxx

#include "itkHessianToMeijeringMeasureImageFilter.h"
#include "itkStatisticsImageFilter.h"

namespace itk{
    template< typename TInputImage,typename TOutputImage>
    HessianToMeijeringMeasureImageFilter<TInputImage, TOutputImage>::HessianToMeijeringMeasureImageFilter()
    {
        //this->DynamicMultiThreadingOn();
    }

    template< typename TInputImage,typename TOutputImage>
    void HessianToMeijeringMeasureImageFilter<TInputImage,TOutputImage>::VerifyPreconditions() ITKv5_CONST
    {
        Superclass::VerifyPreconditions();
        if ( ImageDimension != 3 )
        {
        itkExceptionMacro("Image Dimension must be 3");
        }
    }

    template<typename TInputImage, typename TOutputImage>
    void HessianToMeijeringMeasureImageFilter<TInputImage,TOutputImage>::BeforeThreadedGenerateData()
    {
    }

    template<typename TInputImage,typename TOutputImage>
    void HessianToMeijeringMeasureImageFilter<TInputImage,TOutputImage>::DynamicThreadedGenerateData(const OutputImageRegionType & outputRegionForThread)
    {
        OutputImageType * output = this->GetOutput();
        const InputImageType* input = this->GetInput();

    }

    template<typename TInputImage,typename TOutputImage>
    void HessianToMeijeringMeasureImageFilter<TInputImage, TOutputImage>::GenerateData()
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
        ptr_filter->SetInput(input);
        ptr_filter->SetAlpha(m_Alpha);
        ptr_filter->Update();

        auto eigenValuesImage = ptr_filter->GetOutput();
        std::cout<<"min eigen value:"<<ptr_filter->GetMinEigenValue()<<std::endl;

        EigenValueType minLambda = ptr_filter->GetMinEigenValue();
        OutputPixelType neuritenessMesure = NumericTraits< OutputPixelType >::ZeroValue();

        std::cout<<"computing eigenvalues"<<std::endl;
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
                neuritenessMesure = lambda1 /  minLambda;
            
            oit.Set( neuritenessMesure);   

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
    void HessianToMeijeringMeasureImageFilter<TInputImage, TOutputImage>
    ::PrintSelf(std::ostream & os, Indent indent) const
    {
        Superclass::PrintSelf(os,indent);
        os << indent << "Alpha " << m_Alpha << std::endl; 
    }

}

#endif // itkHessianToMeijeringMeasureImageFilter_hxx