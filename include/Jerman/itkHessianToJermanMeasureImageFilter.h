#ifndef itkHessianToJermanMeasureImageFilter_h
#define itkHessianToJermanMeasureImageFilter_h

#include <cmath>
#include "itkHessianToMeasureImageFilter.h"

namespace itk{

template< typename TInputImage, typename TOutputImage, typename TMaskImage = itk::Image<uint8_t,3> >
class ITK_TEMPLATE_EXPORT HessianToJermanMeasureImageFilter : 
public HessianToMeasureImageFilter<TInputImage, TOutputImage,TMaskImage>
{
    public:

    ITK_DISALLOW_COPY_AND_ASSIGN(HessianToJermanMeasureImageFilter);

    /** standard aliases */
    using Self = HessianToJermanMeasureImageFilter;
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
    itkTypeMacro(HessianToJermanMeasureImageFilter, HessianToMeasureImageFilter);
    
    itkSetMacro(Tau,double);
    itkGetConstMacro(Tau,double);

    itkSetMacro(BrightObject,bool);
    itkGetConstMacro(BrightObject,bool);

    protected:

    HessianToJermanMeasureImageFilter();
    ~HessianToJermanMeasureImageFilter() override = default;
    
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

    double m_Tau{0.75};
    bool m_BrightObject{true};
    
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkHessianToJermanMeasureImageFilter.hxx"
#endif

#endif // itkHessianToJermanMeasureImageFilter_h