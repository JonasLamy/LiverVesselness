#ifndef itkHessianToMeijeringMeasureImageFilter_h
#define itkHessianToMeijeringMeasureImageFilter_h

#include <cmath>
#include "itkModifiedHessianToEigenValues.h"

namespace itk{

template< typename TInputImage, typename TOutputImage, typename TMaskImage>
class ITK_TEMPLATE_EXPORT HessianToMeijeringMeasureImageFilter : 
public ImageToImageFilter<TInputImage, TOutputImage>
{
    public:

    ITK_DISALLOW_COPY_AND_ASSIGN(HessianToMeijeringMeasureImageFilter);

    /** standard aliases */
    using Self = HessianToMeijeringMeasureImageFilter;
    using Superclass = ImageToImageFilter< TInputImage, TOutputImage >;
    using Pointer = SmartPointer< Self >;
    using ConstPointer = SmartPointer< const Self >;

    using InputImageType = typename Superclass::InputImageType;
    using OutputImageType = typename Superclass::OutputImageType;
    using InputPixelType = typename InputImageType::PixelType;
    using OutputPixelType = typename OutputImageType::PixelType;
    using OutputImageRegionType = typename OutputImageType::RegionType;

    using MaskImageType = TMaskImage;

    /** Image dimension */
    static constexpr unsigned int ImageDimension = InputImageType ::ImageDimension;

    using EigenValueType = double;
    using EigenValueArrayType = itk::FixedArray< EigenValueType, Self::ImageDimension >;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Runtime information support. */
    itkTypeMacro(HessianToMeijeringMeasureImageFilter, ImageToImageFilter);
    
    itkSetMacro(Alpha,double);
    itkGetConstMacro(Alpha,double);

    itkSetMacro(BrightObject,bool);
    itkGetConstMacro(BrightObject,bool);

    void SetMaskImage(typename MaskImageType::Pointer maskImage){m_maskImage = maskImage;}

    protected:

    HessianToMeijeringMeasureImageFilter();
    ~HessianToMeijeringMeasureImageFilter() override = default;
    
    void PrintSelf(std::ostream & os, Indent indent) const override;

    void VerifyPreconditions() ITKv5_CONST override;
    void GenerateData() override;
    void DynamicThreadedGenerateData(const OutputImageRegionType & outputRegionForThread) override;
    void BeforeThreadedGenerateData() override;

    private:

    void withMask();
    void noMask();

    struct AbsLessEqualCompare{ 
        bool operator()(EigenValueType a,EigenValueType b)
        {
            return itk::Math::abs(a)<= itk::Math::abs(b);
        }
    };

    double m_Alpha;
    bool m_BrightObject;
    typename MaskImageType::Pointer m_maskImage;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkHessianToMeijeringMeasureImageFilter.hxx"
#endif

#endif // itkHessianToMeijeringMeasureImageFilter_h