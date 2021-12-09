#ifndef itkHessianToFrangiMeasureImageFilter_h
#define itkHessianToFrangiMeasureImageFilter_h

#include <cmath>
#include "itkHessianToMeasureImageFilter.h"

namespace itk{

template< typename TInputImage, typename TOutputImage, typename TMaskImage = itk::Image<uint8_t,3> >
class ITK_TEMPLATE_EXPORT HessianToFrangiMeasureImageFilter : 
public HessianToMeasureImageFilter<TInputImage, TOutputImage,TMaskImage>
{
    public:

    ITK_DISALLOW_COPY_AND_ASSIGN(HessianToFrangiMeasureImageFilter);

    /** standard aliases */
    using Self = HessianToFrangiMeasureImageFilter;
    using Superclass = HessianToMeasureImageFilter<TInputImage, TOutputImage,TMaskImage>;
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
    static constexpr unsigned int ImageDimension = InputImageType::ImageDimension;

    using EigenValueType = double;
    using EigenValueArrayType = itk::FixedArray< EigenValueType, Self::ImageDimension >;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Runtime information support. */
    itkTypeMacro(HessianToFrangiMeasureImageFilter, HessianToMeasureImageFilter);
    
    itkSetMacro(Alpha,double);
    itkGetConstMacro(Alpha,double);

    itkSetMacro(Beta,double);
    itkGetConstMacro(Beta,double);

    itkSetMacro(BrightObject,bool);
    itkGetConstMacro(BrightObject,bool);

    protected:

    HessianToFrangiMeasureImageFilter();
    ~HessianToFrangiMeasureImageFilter() override = default;
    
    void PrintSelf(std::ostream & os, Indent indent) const override;

    virtual void GenerateData() override;
    virtual void DynamicThreadedGenerateData(const OutputImageRegionType & outputRegionForThread) override;
    virtual void BeforeThreadedGenerateData() override;

    private:

    virtual void noMask() override;
    virtual void withMask() override;

    struct AbsLessEqualCompare{ 
        bool operator()(EigenValueType a,EigenValueType b)
        {
            return itk::Math::abs(a)<= itk::Math::abs(b);
        }
    };

    double m_Alpha{0.5};
    double m_Beta{0.5};
    bool m_BrightObject{true};
    
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkHessianToFrangiMeasureImageFilter.hxx"
#endif

#endif // itkHessianToFrangiMeasureImageFilter_h