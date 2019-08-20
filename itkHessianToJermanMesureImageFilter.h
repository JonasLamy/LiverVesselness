#ifndef itkHessianToJermanMesureImageFilter_h
#define itkHessianToJermanMesureImageFilter_h

namespace itk{

template< typename TInputImage, typename TOuputImage>
class ITK_TEMPLATE_EXPORT HessianToJermanMesureImageFilter : 
public ImageToImageFilter<TInputImage, TOutputImage>
{
    public:

    ITK_DISALLOW_COPY_AND_ASSIGN(HessianToJermanMesureImageFilter);

    /** standard aliases */
    using Self = HessianToObjectnessMeasureImageFilter;
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
    itkTypeMacro(HessianToJermanMesureImageFilter, ImageToImageFilter);
    
    itkSetMacro(Tau,double);
    itkGetConstMacro(Tau,double);

    protected:

    HessianToJermanMesureImageFilter();
    ~HessianToJermanMesureImageFilter() override = default;
    
    void PrintSelf(std::ostream & os, Indent indent) const override;

    void VerifyPreconditions() ITKv5_CONST override;
    void DynamicThreadedGenerateData(const OutputImageRegionType & outputRegionForThread) override;

    private:

    struct AbsLessEqualCompare{ 
        bool operator()(EigenValueType a,EigenValueType b)
        {
            return itk::Math::abs(a)<= itk::Math::abs(b);
        }
    }

    double m_tau;

};

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkHessianToJermanMesureImageFilter.hxx"
#endif // itkHessianToJermanMesureImageFilter_h