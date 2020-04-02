#ifndef _itkHessianToEigenValues_hxx
#define _itkHessianToEigenValues_hxx

#include <mutex>

namespace itk
{
    template<typename TInputImage,typename TOutputImage, typename TMaskImage>
    HessianToEigenValuesImageFilter<TInputImage,TOutputImage,TMaskImage>::HessianToEigenValuesImageFilter()
    :m_maxEigenValue(0.0f), m_minEigenValue(0.0f), m_maxEigenValueNorm(0.0f),m_maskImage(nullptr)
    {

        // max eigen value
        typename RealObjectType::Pointer outputMaxEV = 
        static_cast<RealObjectType *>(this->MakeOutput(1).GetPointer() );
        this->ProcessObject::SetNthOutput(1,outputMaxEV.GetPointer());

        // min eigen value
        typename RealObjectType::Pointer outputMinEV = 
        static_cast<RealObjectType *>(this->MakeOutput(3).GetPointer() );
        this->ProcessObject::SetNthOutput(3,outputMinEV.GetPointer());

        typename RealObjectType::Pointer outputMaxEVNorm = 
        static_cast<RealObjectType *>(this->MakeOutput(4).GetPointer());
        this->ProcessObject::SetNthOutput(4,outputMaxEVNorm.GetPointer());

        this->GetMaxEigenValueOutput()->Set( NumericTraits<RealType>::ZeroValue() );
        this->GetMinEigenValueOutput()->Set( NumericTraits<RealType>::max() );
        this->GetMaxEigenValueNormOutput()->Set( NumericTraits<RealType>::ZeroValue() );

        /* Should not be needed, see AllocateOutputs */
        typename TOutputImage::Pointer outputImage = 
        static_cast<TOutputImage *>(this->MakeOutput(2).GetPointer());
        this->ProcessObject::SetNthOutput(2,outputImage.GetPointer());
    }

    

    template<typename TInputImage,typename TOutputImage, typename TMaskImage>
    DataObject::Pointer HessianToEigenValuesImageFilter< TInputImage, TOutputImage, TMaskImage >
    ::MakeOutput(DataObjectPointerArraySizeType output)
    {
    switch ( output )
        {
        case 0: // Input Image
        return TInputImage::New().GetPointer();
        break;
        case 1: // Max eigenValue
        return RealObjectType::New().GetPointer();
        break;
        case 2: // eigenValues image
        return TOutputImage::New().GetPointer();
        break;
        case 3: // Min eigenValue
        return RealObjectType::New().GetPointer();
        break;
        case 4: // Max Norm value
        return RealObjectType::New().GetPointer();
        break;
        default:
        // might as well make an image
        return TInputImage::New().GetPointer();
        break;
        }
    }

    template<typename TInputImage,typename TOutputImage, typename TMaskImage>
    typename HessianToEigenValuesImageFilter<TInputImage,TOutputImage,TMaskImage>::RealObjectType* 
    HessianToEigenValuesImageFilter<TInputImage,TOutputImage,TMaskImage>::GetMaxEigenValueOutput()
    {
        return static_cast<RealObjectType *>( this->ProcessObject::GetOutput(1) );
    }

    template<typename TInputImage,typename TOutputImage, typename TMaskImage>
    const typename HessianToEigenValuesImageFilter<TInputImage,TOutputImage,TMaskImage>::RealObjectType* 
    HessianToEigenValuesImageFilter<TInputImage,TOutputImage,TMaskImage>::GetMaxEigenValueOutput() const
    {
        return static_cast<const RealObjectType *>( this->ProcessObject::GetOutput(1) );
    }

    template<typename TInputImage,typename TOutputImage, typename TMaskImage>
    typename HessianToEigenValuesImageFilter<TInputImage,TOutputImage,TMaskImage>::RealObjectType* 
    HessianToEigenValuesImageFilter<TInputImage,TOutputImage,TMaskImage>::GetMinEigenValueOutput()
    {
        return static_cast<RealObjectType *>( this->ProcessObject::GetOutput(3) );
    }

    template<typename TInputImage,typename TOutputImage, typename TMaskImage>
    const typename HessianToEigenValuesImageFilter<TInputImage,TOutputImage,TMaskImage>::RealObjectType* 
    HessianToEigenValuesImageFilter<TInputImage,TOutputImage,TMaskImage>::GetMinEigenValueOutput() const
    {
        return static_cast<const RealObjectType *>( this->ProcessObject::GetOutput(3) );
    }

    template<typename TInputImage,typename TOutputImage, typename TMaskImage>
    typename HessianToEigenValuesImageFilter<TInputImage,TOutputImage,TMaskImage>::RealObjectType* 
    HessianToEigenValuesImageFilter<TInputImage,TOutputImage,TMaskImage>::GetMaxEigenValueNormOutput()
    {
        return static_cast<RealObjectType *>( this->ProcessObject::GetOutput(4) );
    }

    template<typename TInputImage,typename TOutputImage, typename TMaskImage>
    const typename HessianToEigenValuesImageFilter<TInputImage,TOutputImage,TMaskImage>::RealObjectType* 
    HessianToEigenValuesImageFilter<TInputImage,TOutputImage,TMaskImage>::GetMaxEigenValueNormOutput() const
    {
        return static_cast<const RealObjectType *>( this->ProcessObject::GetOutput(4) );
    }




    template<typename TInputImage,typename TOutputImage, typename TMaskImage>
    void HessianToEigenValuesImageFilter<TInputImage,TOutputImage,TMaskImage>::GenerateInputRequestedRegion()
    {
        Superclass::GenerateInputRequestedRegion();
        if( this->GetInput() )
        {
            InputImagePointer image = const_cast<typename Superclass::InputImageType*>( this->GetInput() );
            image->SetRequestedRegionToLargestPossibleRegion();
        }
    }

    template<typename TInputImage,typename TOutputImage, typename TMaskImage>
    void HessianToEigenValuesImageFilter<TInputImage,TOutputImage,TMaskImage>::EnlargeOutputRequestedRegion(DataObject * data)
    {
        Superclass::EnlargeOutputRequestedRegion(data);
        data->SetRequestedRegionToLargestPossibleRegion();
    }

    template<typename TInputImage,typename TOutputImage, typename TMaskImage>
    void HessianToEigenValuesImageFilter<TInputImage,TOutputImage,TMaskImage>::AllocateOutputs()
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

    template<typename TInputImage, typename TOutputImage,typename TMaskImage>
    void HessianToEigenValuesImageFilter<TInputImage,TOutputImage,TMaskImage>::BeforeThreadedGenerateData()
    {
        m_maxEigenValue = NumericTraits<EigenValueType>::NonpositiveMin();
        m_maxEigenValueNorm = NumericTraits<EigenValueType>::NonpositiveMin();
        m_minEigenValue = NumericTraits<EigenValueType>::max();

    }

    template<typename TInputImage,typename TOutputImage, typename TMaskImage>
    void HessianToEigenValuesImageFilter<TInputImage,TOutputImage,TMaskImage>::AfterThreadedGenerateData()
    {
        const EigenValueType maximum = m_maxEigenValue;
        const EigenValueType minimum = m_minEigenValue;
        const EigenValueType maximumNorm = m_maxEigenValueNorm;
        //Set the outputs
        this->GetMaxEigenValueOutput()->Set(maximum);
        this->GetMinEigenValueOutput()->Set(minimum);
        this->GetMaxEigenValueNormOutput()->Set(maximumNorm);
    }

    template<typename TInputImage, typename TOutputImage, typename TMaskImage>
    void HessianToEigenValuesImageFilter<TInputImage,TOutputImage,TMaskImage>::DynamicThreadedGenerateData(const RegionType & regionForThread)
    {
        if( m_maskImage)
        {
            withMask(regionForThread);
        }
        else
        {
            noMask(regionForThread);
        }
    }

    template< typename TInputImage, typename TOutputImage, typename TMaskImage >
    void HessianToEigenValuesImageFilter< TInputImage,TOutputImage, TMaskImage >::noMask(const RegionType & regionForThread)
    {
        EigenValueType max = NumericTraits<EigenValueType>::ZeroValue();
        EigenValueType min = NumericTraits<EigenValueType>::max();
        EigenValueType norm = NumericTraits<EigenValueType>::ZeroValue();

        using CalculatorType = SymmetricEigenAnalysisFixedDimension<ImageDimension, PixelType, EigenValueArrayType>;
        CalculatorType eigenCalculator;
        eigenCalculator.SetOrderEigenMagnitudes(true);
        EigenValueArrayType eigenValues;

        // Walk the region of eigen values and get the objectness measure
        ImageScanlineConstIterator< TInputImage > it(this->GetInput(),regionForThread);
        ImageScanlineIterator< TOutputImage > itOut(this->GetOutput(),regionForThread);
        it.GoToBegin();
        itOut.GoToBegin();
        
        while( !it.IsAtEnd() )
        {
            while( !it.IsAtEndOfLine() )
            {
                eigenCalculator.ComputeEigenValues(it.Get(), eigenValues);
                // noise removal on eigenValues
                
                if( std::isinf(eigenValues[0]) || fabs(eigenValues[0]) < 1e-4){ eigenValues[0] = 0; }
                if( std::isinf(eigenValues[1]) || fabs(eigenValues[1]) < 1e-4){ eigenValues[1] = 0; }
                if( std::isinf(eigenValues[2]) || fabs(eigenValues[2]) < 1e-4){ eigenValues[2] = 0; }
                
                itOut.Set(eigenValues);

                max = std::max(max,fabs(eigenValues[2]) );
                min = std::min(min,fabs(eigenValues[0]) );
                // norm of the eigen values vector, the comparison is done on the squared norm for efficiency
                norm = std::max(norm,eigenValues[0]*eigenValues[0] + eigenValues[1]*eigenValues[1] + eigenValues[2]*eigenValues[2]);
                 
                ++it;
                ++itOut;
            } 
            it.NextLine();
            itOut.NextLine();
        }
        std::lock_guard<std::mutex> mutexHolder(m_mutex);
        m_maxEigenValue = std::max(max,m_maxEigenValue);
        m_minEigenValue = std::min(min,m_minEigenValue);
        m_maxEigenValueNorm = std::sqrt(std::max(norm,m_maxEigenValueNorm));
    }

    template< typename TInputImage, typename TOutputImage, typename TMaskImage >
    void HessianToEigenValuesImageFilter< TInputImage,TOutputImage,TMaskImage >::withMask(const RegionType & regionForThread)
    {
        EigenValueType max = NumericTraits<EigenValueType>::ZeroValue();
        EigenValueType min = NumericTraits<EigenValueType>::max();
        EigenValueType norm = NumericTraits<EigenValueType>::ZeroValue();

        using CalculatorType = SymmetricEigenAnalysisFixedDimension<ImageDimension, PixelType, EigenValueArrayType>;
        CalculatorType eigenCalculator;
        eigenCalculator.SetOrderEigenMagnitudes(true);
        EigenValueArrayType eigenValues;

        // Walk the region of eigen values and get the objectness measure
        ImageScanlineConstIterator< TInputImage > it(this->GetInput(),regionForThread);
        ImageScanlineConstIterator<TMaskImage>itMask(m_maskImage,regionForThread);

        ImageScanlineIterator< TOutputImage > itOut(this->GetOutput(),regionForThread);

        it.GoToBegin();
        itOut.GoToBegin();
        itMask.GoToBegin();

        while( !it.IsAtEnd() )
        {
            while( !it.IsAtEndOfLine() )
            {
                if( itMask.Get() == 0 )
                {
                    eigenValues[0] = 0;
                    eigenValues[1] = 0;
                    eigenValues[2] = 0;

                    itOut.Set(eigenValues);
                    
                    ++it;
                    ++itOut;
                    ++itMask;
                    continue;
                }

                eigenCalculator.ComputeEigenValues(it.Get(), eigenValues);
                // noise removal on eigenValues
                
                if( std::isinf(eigenValues[0]) || fabs(eigenValues[0]) < 1e-4){ eigenValues[0] = 0; }
                if( std::isinf(eigenValues[1]) || fabs(eigenValues[1]) < 1e-4){ eigenValues[1] = 0; }
                if( std::isinf(eigenValues[2]) || fabs(eigenValues[2]) < 1e-4){ eigenValues[2] = 0; }
                
                itOut.Set(eigenValues);

                max = std::max(max,fabs(eigenValues[2]) );
                min = std::min(min,fabs(eigenValues[0]) );
                // norm of the eigen values vector, the comparison is done on the squared norm for efficiency
                norm = std::max(norm,eigenValues[0]*eigenValues[0] + eigenValues[1]*eigenValues[1] + eigenValues[2]*eigenValues[2]);
                 
                ++it;
                ++itOut;
                ++itMask;
            } 
            it.NextLine();
            itOut.NextLine();
            itMask.NextLine();
        }
        std::lock_guard<std::mutex> mutexHolder(m_mutex);
        m_maxEigenValue = std::max(max,m_maxEigenValue);
        m_minEigenValue = std::min(min,m_minEigenValue);
        m_maxEigenValueNorm = std::sqrt(std::max(norm,m_maxEigenValueNorm));
    }

    template< typename TInputImage, typename TOutputImage, typename TMaskImage >
    void HessianToEigenValuesImageFilter< TInputImage,TOutputImage,TMaskImage >
    ::PrintSelf(std::ostream & os, Indent indent) const
    {
        Superclass::PrintSelf(os, indent);

        os << indent << "Maximum: "
        << static_cast< typename NumericTraits< PixelType >::PrintType >( this->GetMaxEigenValue() ) 
        <<"\n"
        << indent << "Min: "
        << static_cast< typename NumericTraits< PixelType >::PrintType >( this->GetMinEigenValue() ) << std::endl;

    }
}

#endif // _itkHessianToEigenvalues_hxx