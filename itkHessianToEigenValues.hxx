#ifndef _itkHessianToEigenValues_hxx
#define _itkHessianToEigenValues_hxx

#include <mutex>

namespace itk
{
    template<typename TInputImage,typename TOutputImage>
    HessianToEigenValuesImageFilter<TInputImage,TOutputImage>::HessianToEigenValuesImageFilter()
    :m_maxEigenValue(0.0f)
    {
        /*
      // first output is a copy of the image, DataObject created by superlass
      // allocate the data objects for the outputs which are just decorators around pixels type
      for( int i=1; i<3; i++)
      {
          typename PixelObjectType::Pointer output = static_cast<PixelObjectType *>(this->MakeOutput(i).GetPointer() );
          this->ProcessObject::SetNthInput(i,output.GetPointer() );
      } 
      // allocate the data objects for the outputs which are
      // just decorators around real types
      for( int i=3; i<8;i++)
      {
          typename RealObjectType::Pointer output = static_cast<RealObject *>(this->makeOutput(i).GetPointer() );
          this->ProcessObject::SetNthOutput(i,output.GetPointer());
      }  */

        typename RealObjectType::Pointer outputEV = 
        static_cast<RealObjectType *>(this->MakeOutput(1).GetPointer() );
        this->ProcessObject::SetNthOutput(1,outputEV.GetPointer());

        this->GetMaxEigenValueOutput()->Set( NumericTraits<RealType>::ZeroValue() );

        /* Should not be needed, see AllocateOutputs */
        typename TOutputImage::Pointer outputImage = 
        static_cast<TOutputImage *>(this->MakeOutput(2).GetPointer());
        this->ProcessObject::SetNthOutput(2,outputImage.GetPointer());
    }

    

    template<typename TInputImage,typename TOutputImage>
    DataObject::Pointer
    HessianToEigenValuesImageFilter< TInputImage, TOutputImage >
    ::MakeOutput(DataObjectPointerArraySizeType output)
    {
    switch ( output )
        {
        case 0: // Input Image
        return TInputImage::New().GetPointer();
        break;
        case 1: // Max eigenValue
        return RealObjectType::New().GetPointer();
        case 2: // eigenValues image
        return TOutputImage::New().GetPointer();
        default:
        // might as well make an image
        return TInputImage::New().GetPointer();
        break;
        }
    }

    template<typename TInputImage,typename TOutputImage>
    typename HessianToEigenValuesImageFilter<TInputImage,TOutputImage>::RealObjectType* 
    HessianToEigenValuesImageFilter<TInputImage,TOutputImage>::GetMaxEigenValueOutput()
    {
        return static_cast<RealObjectType *>( this->ProcessObject::GetOutput(1) );
    }

    template<typename TInputImage,typename TOutputImage>
    const typename HessianToEigenValuesImageFilter<TInputImage,TOutputImage>::RealObjectType* 
    HessianToEigenValuesImageFilter<TInputImage,TOutputImage>::GetMaxEigenValueOutput() const
    {
        return static_cast<const RealObjectType *>( this->ProcessObject::GetOutput(1) );
    }

    template<typename TInputImage,typename TOutputImage>
    void HessianToEigenValuesImageFilter<TInputImage,TOutputImage>::GenerateInputRequestedRegion()
    {
        Superclass::GenerateInputRequestedRegion();
        if( this->GetInput() )
        {
            InputImagePointer image = const_cast<typename Superclass::InputImageType*>( this->GetInput() );
            image->SetRequestedRegionToLargestPossibleRegion();
        }
    }

    template<typename TInputImage,typename TOutputImage>
    void HessianToEigenValuesImageFilter<TInputImage,TOutputImage>::EnlargeOutputRequestedRegion(DataObject * data)
    {
        Superclass::EnlargeOutputRequestedRegion(data);
        data->SetRequestedRegionToLargestPossibleRegion();
    }

    template<typename TInputImage,typename TOutputImage>
    void HessianToEigenValuesImageFilter<TInputImage,TOutputImage>::AllocateOutputs()
    {
        // pass the input through as the output
        //InputImagePointer image = const_cast<TInputImage *>( this->GetInput() );
        
        // filter's output is now the eigen values image
        typename TOutputImage::Pointer out = TOutputImage::New();
        // make output based on input infos
        typename TOutputImage::RegionType region = this->GetInput()->GetLargestPossibleRegion();
        typename TOutputImage::SpacingType spacing = this->GetInput()->GetSpacing();
        out->SetSpacing(spacing);
        out->SetRegions(region);
        out->Allocate();
        this->GraftOutput(out);

        
        // nothing to allocate for the remaining outputs for now...
    }

    template<typename TInputImage, typename TOutputImage>
    void HessianToEigenValuesImageFilter<TInputImage,TOutputImage>::BeforeThreadedGenerateData()
    {
        // Resize he thread temporaries;
        m_maxEigenValue = NumericTraits<EigenValueType>::NonpositiveMin();

    }

    template<typename TInputImage,typename TOutputImage>
    void HessianToEigenValuesImageFilter<TInputImage,TOutputImage>::AfterThreadedGenerateData()
    {
        const EigenValueType maximum = m_maxEigenValue;
        //Set the outputs
        this->GetMaxEigenValueOutput()->Set(maximum);
    }

    template<typename TInputImage, typename TOutputImage>
    void HessianToEigenValuesImageFilter<TInputImage,TOutputImage>::DynamicThreadedGenerateData(const RegionType & regionForThread)
    {
        EigenValueType max = NumericTraits<EigenValueType>::NonpositiveMin();

        using CalculatorType = SymmetricEigenAnalysisFixedDimension<ImageDimension, PixelType, EigenValueArrayType>;
        CalculatorType eigenCalculator;
        eigenCalculator.SetOrderEigenMagnitudes(true);
        EigenValueArrayType eigenValues;

        // Walk the region of eigen values and get the objectness measure
        ImageScanlineConstIterator< TInputImage > it(this->GetInput(),regionForThread);
        ImageScanlineIterator< TOutputImage > itOut(this->GetOutput(),regionForThread);
        std::cout<<"region :"<<regionForThread<<std::endl;
        it.GoToBegin();
        itOut.GoToBegin();
        while( !it.IsAtEnd() )
        {
            while( !it.IsAtEndOfLine() )
            {
                eigenCalculator.ComputeEigenValues(it.Get(), eigenValues);
                // noise removal on eigenValues
                if( std::isinf(eigenValues[0]) || abs(eigenValues[0]) < 1e-4){ eigenValues[0] = 0; }
                if( std::isinf(eigenValues[1]) || abs(eigenValues[1]) < 1e-4){ eigenValues[1] = 0; }
                if( std::isinf(eigenValues[2]) || abs(eigenValues[2]) < 1e-4){ eigenValues[2] = 0; }

                itOut.Set(eigenValues);

                max = std::max(max,eigenValues[2]);
                ++it;
                ++itOut;
            } 
            it.NextLine();
            itOut.NextLine();
        }
        std::lock_guard<std::mutex> mutexHolder(m_mutex);
        m_maxEigenValue = std::max(max,m_maxEigenValue);
    }

    template< typename TInputImage, typename TOutputImage >
    void HessianToEigenValuesImageFilter< TInputImage,TOutputImage >
    ::PrintSelf(std::ostream & os, Indent indent) const
    {
        Superclass::PrintSelf(os, indent);

        os << indent << "Maximum: "
        << static_cast< typename NumericTraits< PixelType >::PrintType >( this->GetMaxEigenValue() ) << std::endl;

    }
}

#endif // _itkHessianToEigenvalues_hxx