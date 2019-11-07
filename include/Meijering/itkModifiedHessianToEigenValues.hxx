#ifndef _itkModifiedHessianToEigenValues_hxx
#define _itkModifiedHessianToEigenValues_hxx

#include <mutex>

namespace itk
{
    template<typename TInputImage,typename TOutputImage>
    ModifiedHessianToEigenValuesImageFilter<TInputImage,TOutputImage>::ModifiedHessianToEigenValuesImageFilter()
    :m_minEigenValue(NumericTraits<EigenValueType>::max()),m_Alpha(-0.5)
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

        this->GetMinEigenValueOutput()->Set( NumericTraits<RealType>::ZeroValue() );

        /* Should not be needed, see AllocateOutputs */
        typename TOutputImage::Pointer outputImage = 
        static_cast<TOutputImage *>(this->MakeOutput(2).GetPointer());
        this->ProcessObject::SetNthOutput(2,outputImage.GetPointer());
    }

    

    template<typename TInputImage,typename TOutputImage>
    DataObject::Pointer
    ModifiedHessianToEigenValuesImageFilter< TInputImage, TOutputImage >
    ::MakeOutput(DataObjectPointerArraySizeType output)
    {
    switch ( output )
        {
        case 0: // Input Image
        return TInputImage::New().GetPointer();
        break;
        case 1: // Min eigenValue
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
    typename ModifiedHessianToEigenValuesImageFilter<TInputImage,TOutputImage>::RealObjectType* 
    ModifiedHessianToEigenValuesImageFilter<TInputImage,TOutputImage>::GetMinEigenValueOutput()
    {
        return static_cast<RealObjectType *>( this->ProcessObject::GetOutput(1) );
    }

    template<typename TInputImage,typename TOutputImage>
    const typename ModifiedHessianToEigenValuesImageFilter<TInputImage,TOutputImage>::RealObjectType* 
    ModifiedHessianToEigenValuesImageFilter<TInputImage,TOutputImage>::GetMinEigenValueOutput() const
    {
        return static_cast<const RealObjectType *>( this->ProcessObject::GetOutput(1) );
    }

    template<typename TInputImage,typename TOutputImage>
    void ModifiedHessianToEigenValuesImageFilter<TInputImage,TOutputImage>::GenerateInputRequestedRegion()
    {
        Superclass::GenerateInputRequestedRegion();
        if( this->GetInput() )
        {
            InputImagePointer image = const_cast<typename Superclass::InputImageType*>( this->GetInput() );
            image->SetRequestedRegionToLargestPossibleRegion();
        }
    }

    template<typename TInputImage,typename TOutputImage>
    void ModifiedHessianToEigenValuesImageFilter<TInputImage,TOutputImage>::EnlargeOutputRequestedRegion(DataObject * data)
    {
        Superclass::EnlargeOutputRequestedRegion(data);
        data->SetRequestedRegionToLargestPossibleRegion();
    }

    template<typename TInputImage,typename TOutputImage>
    void ModifiedHessianToEigenValuesImageFilter<TInputImage,TOutputImage>::AllocateOutputs()
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
    void ModifiedHessianToEigenValuesImageFilter<TInputImage,TOutputImage>::BeforeThreadedGenerateData()
    {
        // Resize he thread temporaries;
        m_minEigenValue = NumericTraits<EigenValueType>::max();

    }

    template<typename TInputImage,typename TOutputImage>
    void ModifiedHessianToEigenValuesImageFilter<TInputImage,TOutputImage>::AfterThreadedGenerateData()
    {
        const EigenValueType minimum = m_minEigenValue;
        //Set the outputs
        this->GetMinEigenValueOutput()->Set(minimum);
    }

    template<typename TInputImage, typename TOutputImage>
    void ModifiedHessianToEigenValuesImageFilter<TInputImage,TOutputImage>::DynamicThreadedGenerateData(const RegionType & regionForThread)
    {
        EigenValueType min = NumericTraits<EigenValueType>::max();

        using CalculatorType = SymmetricEigenAnalysisFixedDimension<ImageDimension, PixelType, EigenValueArrayType>;
        CalculatorType eigenCalculator;
        eigenCalculator.SetOrderEigenMagnitudes(true);
        EigenValueArrayType eigenValues;
        EigenValueArrayType modifiedEigenValues;

        // Walk the region of eigen values and get the objectness measure
        ImageScanlineConstIterator< TInputImage > it(this->GetInput(),regionForThread);
        ImageScanlineIterator< TOutputImage > itOut(this->GetOutput(),regionForThread);

        it.GoToBegin();
        itOut.GoToBegin();
        while( !it.IsAtEnd() )
        {
            while( !it.IsAtEndOfLine() )
            {
                // Eigen values from pixel's hessian
                eigenCalculator.ComputeEigenValues(it.Get(), eigenValues);
                
                // noise removal on eigenValues
                if( std::isinf(eigenValues[0]) || abs(eigenValues[0]) < 1e-4){ eigenValues[0] = 0; }
                if( std::isinf(eigenValues[1]) || abs(eigenValues[1]) < 1e-4){ eigenValues[1] = 0; }
                if( std::isinf(eigenValues[2]) || abs(eigenValues[2]) < 1e-4){ eigenValues[2] = 0; }

                // modified eigenvalues for meijerin's neuriteness
                modifiedEigenValues[0] = eigenValues[0] + m_Alpha * eigenValues[1] + m_Alpha * eigenValues[2];
                modifiedEigenValues[1] = eigenValues[1] + m_Alpha * eigenValues[0] + m_Alpha * eigenValues[2];
                modifiedEigenValues[2] = eigenValues[2] + m_Alpha * eigenValues[0] + m_Alpha * eigenValues[1];

                // sorting values lambda1 < lambda2 < lambda3
                std::sort(modifiedEigenValues.begin(), modifiedEigenValues.end(), [](EigenValueType i, EigenValueType j) { return abs(i) < abs(j); } );                 
                min = std::min( min,modifiedEigenValues[0] );
                // sorting by magnitude before giving it to mesure filter
                //std::sort(modifiedEigenValues.begin(), modifiedEigenValues.end(), [](EigenValueType i, EigenValueType j) { return abs(i) < abs(j); } );
                
                itOut.Set(modifiedEigenValues);
                
                ++it;
                ++itOut;
            } 
            it.NextLine();
            itOut.NextLine();
        }
        std::lock_guard<std::mutex> mutexHolder(m_mutex);
        m_minEigenValue = std::min(min,m_minEigenValue);
    }

    template< typename TInputImage, typename TOutputImage >
    void ModifiedHessianToEigenValuesImageFilter< TInputImage,TOutputImage >
    ::PrintSelf(std::ostream & os, Indent indent) const
    {
        Superclass::PrintSelf(os, indent);

        os << indent << "Minimum: "
        << static_cast< typename NumericTraits< PixelType >::PrintType >( this->GetMinEigenValue() ) << std::endl;

    }
}

#endif // _itkModifiedHessianToEigenValues_hxx