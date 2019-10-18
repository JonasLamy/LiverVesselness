#ifndef itkHessianToZhangMeasureImageFilter_h
#define itkHessianToZhangMeasureImageFilter_h

#include <cmath>
#include "itkHessianToEigenValues.h"

namespace itk{

template< typename TInputImage, typename TOutputImage>
class ITK_TEMPLATE_EXPORT HessianToZhangMeasureImageFilter : 
public ImageToImageFilter<TInputImage, TOutputImage>
{
    public:

    ITK_DISALLOW_COPY_AND_ASSIGN(HessianToZhangMeasureImageFilter);

    /** standard aliases */
    using Self = HessianToZhangMeasureImageFilter;
    using Superclass = ImageToImageFilter< TInputImage, TOutputImage >;
    using Pointer = SmartPointer< Self >;
    using ConstPointer = SmartPointer< const Self >;

    using InputImageType = typename Superclass::InputImageType;
    using OutputImageType = typename Superclass::OutputImageType;
    using InputPixelType = typename InputImageType::PixelType;
    using OutputPixelType = typename OutputImageType::PixelType;
    using OutputImageRegionType = typename OutputImageType::RegionType;

    /** Image dimension */
    static constexpr unsigned int ImageDimension = InputImageType ::ImageDimension;

    using EigenValueType = double;
    using EigenValueArrayType = itk::FixedArray< EigenValueType, Self::ImageDimension >;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Runtime information support. */
    itkTypeMacro(HessianToZhangMeasureImageFilter, ImageToImageFilter);
    
    itkSetMacro(Tau,double);
    itkGetConstMacro(Tau,double);

    itkSetMacro(Scale,double);
    itkGetConstMacro(Scale,double);

    itkSetMacro(Alpha,double);
    itkGetConstMacro(Alpha,double);

    itkSetMacro(BrightObject,bool);
    itkGetConstMacro(BrightObject,bool);

    protected:

    HessianToZhangMeasureImageFilter();
    ~HessianToZhangMeasureImageFilter() override = default;
    
    void PrintSelf(std::ostream & os, Indent indent) const override;

    void VerifyPreconditions() ITKv5_CONST override;
    void GenerateData() override;
    void DynamicThreadedGenerateData(const OutputImageRegionType & outputRegionForThread) override;
    void BeforeThreadedGenerateData() override;

    private:

    struct AbsLessEqualCompare{ 
        bool operator()(EigenValueType a,EigenValueType b)
        {
            return itk::Math::abs(a)<= itk::Math::abs(b);
        }
    };

    double m_Tau{0.75};
    double m_Scale;
    double m_Alpha;
    bool m_BrightObject{true};
    

};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkHessianToZhangMeasureImageFilter.hxx"
#endif

#endif // itkHessianToZhangMeasureImageFilter_h