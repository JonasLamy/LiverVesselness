#ifndef _itkModifiedHessianToEigenValuesImageFilter_h
#define _itkModifiedHessianToEigenValuesImageFilter_h

#include "itkImageToImageFilter.h"
#include "itkSimpleDataObjectDecorator.h"

namespace itk{
    template <typename TInputImage,
    typename TOutputImage = Image<FixedArray< double, 3>,3>,
    typename TMaskImage = Image<uint8_t,3 > > 
    class ITK_TEMPLATE_EXPORT ModifiedHessianToEigenValuesImageFilter:public ImageToImageFilter<TInputImage,TOutputImage>
    {
        public:

        ITK_DISALLOW_COPY_AND_ASSIGN(ModifiedHessianToEigenValuesImageFilter);

        /** Standard self type alias **/
        using Self = ModifiedHessianToEigenValuesImageFilter;
        using Superclass = ImageToImageFilter<TInputImage,TOutputImage>;
        using Pointer = SmartPointer<Self>;
        using ConstPointer = SmartPointer<const Self>;

        itkNewMacro(Self);
        itkTypeMacro(ModifiedHessianToEigenValuesImageFilter,ImageToImageFilter);

        /* image related type alias */
        using InputImagePointer = typename TInputImage::Pointer;
        using RegionType = typename TInputImage::RegionType;
        using SizeType = typename TInputImage::SizeType;
        using IndexType = typename TInputImage::IndexType;
        using PixelType = typename TInputImage::PixelType;

        using MaskImageType = TMaskImage;

        static constexpr unsigned int ImageDimension = TInputImage::ImageDimension;


        /** Return the computed maximum EigenValue **/
        using EigenValueType = double;
        using EigenValueArrayType = itk::FixedArray< EigenValueType, Self::ImageDimension >;

        using RealType = typename NumericTraits<EigenValueType>::RealType;
        using DataObjectPointer = typename DataObject::Pointer;

        using RealObjectType = SimpleDataObjectDecorator<RealType>;
        using PixelObjectType = SimpleDataObjectDecorator<PixelType>;

        EigenValueType GetMinEigenValue() const{ return this->GetMinEigenValueOutput()->Get(); }
        RealObjectType * GetMinEigenValueOutput();
        const RealObjectType * GetMinEigenValueOutput()const;

        using DataObjectPointerArraySizeType = ProcessObject::DataObjectPointerArraySizeType;
        using Superclass::MakeOutput;
        DataObjectPointer MakeOutput(DataObjectPointerArraySizeType idx)override;

        #ifdef ITK_USE_CONCEPT_CHECKING
        // Begin concept checking
        itkConceptMacro( InputHasNumericTraitsCheck,
                   ( Concept::HasNumericTraits< RealType > ) );
        #endif
        // End concept checking

        itkSetMacro(Alpha,double);
        itkGetConstMacro(Alpha,double);

        void SetMaskImage(typename MaskImageType::Pointer maskImage){m_maskImage = maskImage;}

        protected:
        
        ModifiedHessianToEigenValuesImageFilter();
        ~ModifiedHessianToEigenValuesImageFilter() override = default;

        void PrintSelf(std::ostream & os,Indent indent) const override;
        void AllocateOutputs() override;

        void BeforeThreadedGenerateData() override;
        void AfterThreadedGenerateData() override;
        void DynamicThreadedGenerateData(const RegionType & regionForThread) override;

        void GenerateInputRequestedRegion() override;
        void EnlargeOutputRequestedRegion( DataObject *data) override;

        private:

        void noMask(const RegionType & regionForThread);
        void withMask(const RegionType & regionForThread);

        EigenValueType m_minEigenValue;
        double m_Alpha;
        std::mutex m_mutex;
        
        typename MaskImageType::Pointer m_maskImage;
    };
}

#ifndef ITK_MANUAL_INSTANCIATION
    #include "itkModifiedHessianToEigenValues.hxx"
#endif

#endif // itkModifiedHessianToEigenValues_h