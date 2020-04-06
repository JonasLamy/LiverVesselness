#ifndef itkHessianToMeasureImageFilter_h
#define itkHessianToMeasureImageFilter_h

#include <cmath>
#include <itkHessianToEigenValues.h>

namespace itk{

template< typename TInputImage, typename TOutputImage, typename TMaskImage = itk::Image<uint8_t,3> >
class ITK_TEMPLATE_EXPORT HessianToMeasureImageFilter : 
public ImageToImageFilter<TInputImage, TOutputImage>
{
    public:

    ITK_DISALLOW_COPY_AND_ASSIGN(HessianToMeasureImageFilter);

    /** standard aliases */
    using Self = HessianToMeasureImageFilter;
    using Superclass = ImageToImageFilter< TInputImage, TOutputImage >;
    using Pointer = SmartPointer< Self >;
    using ConstPointer = SmartPointer< const Self >;

    using InputImageType = typename Superclass::InputImageType;
    using OutputImageType = typename Superclass::OutputImageType;
    using MaskImageType = TMaskImage;

    using InputPixelType = typename InputImageType::PixelType;
    using OutputPixelType = typename OutputImageType::PixelType;
    using MaskPixelType = typename TMaskImage::PixelType;
    
    using OutputImageRegionType = typename OutputImageType::RegionType;
    

    /** Image dimension */
    static constexpr unsigned int ImageDimension = InputImageType ::ImageDimension;

    using EigenValueType = double;
    using EigenValueArrayType = itk::FixedArray< EigenValueType, Self::ImageDimension >;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Runtime information support. */
    itkTypeMacro(HessianToMeasureImageFilter, ImageToImageFilter);

    void SetMaskImage(typename TMaskImage::Pointer maskImg){m_maskImage = maskImg;}


    protected:

    HessianToMeasureImageFilter();
    ~HessianToMeasureImageFilter() override = default;
    
    void PrintSelf(std::ostream & os, Indent indent) const override;

    void VerifyPreconditions() ITKv5_CONST override;
    
    virtual void GenerateData() override;
    virtual void DynamicThreadedGenerateData(const OutputImageRegionType & outputRegionForThread) override;
    virtual void BeforeThreadedGenerateData() override;

    virtual void noMask();
    virtual void withMask();

    typename TMaskImage::Pointer m_maskImage;
    
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkHessianToMeasureImageFilter.hxx"
#endif

#endif // itkHessianToMeasureImageFilter_h