#ifndef _itkHessianToEigenValuesImageFilter_h
#define _itkHessianToEigenValuesImageFilter_h

#include "itkImageToImageFilter.h"
#include "itkSimpleDataObjectDecorator.h"
#include "itkSymmetricEigenAnalysis.h"
#include "itkImageScanlineIterator.h"

namespace itk{
    template <typename TInputImage,
    typename TOutputImage = Image<FixedArray< double, 3>,3>, 
    typename TMaskImage = Image<uint8_t,3> >
    class ITK_TEMPLATE_EXPORT HessianToEigenValuesImageFilter:public ImageToImageFilter<TInputImage,TOutputImage>
    {
        public:

        ITK_DISALLOW_COPY_AND_ASSIGN(HessianToEigenValuesImageFilter);

        /** Standard self type alias **/
        using Self = HessianToEigenValuesImageFilter;
        using Superclass = ImageToImageFilter<TInputImage,TOutputImage>;
        using Pointer = SmartPointer<Self>;
        using ConstPointer = SmartPointer<const Self>;

        itkNewMacro(Self);
        itkTypeMacro(HessianToEigenValuesImageFilter,ImageToImageFilter);

        /* image related type alias */
        using InputImagePointer = typename TInputImage::Pointer;
        using RegionType = typename TInputImage::RegionType;
        using SizeType = typename TInputImage::SizeType;
        using IndexType = typename TInputImage::IndexType;
        using PixelType = typename TInputImage::PixelType;

        static constexpr unsigned int ImageDimension = TInputImage::ImageDimension;


        /** Return the computed maximum EigenValue **/
        using EigenValueType = double;
        using EigenValueArrayType = itk::FixedArray< EigenValueType, Self::ImageDimension >;

        using OutputVectorType = itk::Vector<float,3>;

        using RealType = typename NumericTraits<EigenValueType>::RealType;
        using DataObjectPointer = typename DataObject::Pointer;

        using RealObjectType = SimpleDataObjectDecorator<RealType>;
        using PixelObjectType = SimpleDataObjectDecorator<PixelType>;

        EigenValueType GetMaxEigenValue() const{ return this->GetMaxEigenValueOutput()->Get(); }
        RealObjectType * GetMaxEigenValueOutput();
        const RealObjectType * GetMaxEigenValueOutput()const;

        EigenValueType GetMinEigenValue() const{ return this->GetMinEigenValueOutput()->Get();}
        RealObjectType * GetMinEigenValueOutput();
        const RealObjectType * GetMinEigenValueOutput()const;

        EigenValueType GetMaxEigenValueNorm() const{return this->GetMaxEigenValueNormOutput()->Get();}
        RealObjectType * GetMaxEigenValueNormOutput();
        const RealObjectType * GetMaxEigenValueNormOutput()const;

        using DataObjectPointerArraySizeType = ProcessObject::DataObjectPointerArraySizeType;
        using Superclass::MakeOutput;
        DataObjectPointer MakeOutput(DataObjectPointerArraySizeType idx)override;

        void SetMaskImage(typename TMaskImage::Pointer imgMask){m_maskImage = imgMask;}

        #ifdef ITK_USE_CONCEPT_CHECKING
        // Begin concept checking
        itkConceptMacro( InputHasNumericTraitsCheck,
                   ( Concept::HasNumericTraits< RealType > ) );
        #endif
        // End concept checking

        protected:
        HessianToEigenValuesImageFilter();
        ~HessianToEigenValuesImageFilter() override = default;

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

        typename TMaskImage::Pointer m_maskImage;
        EigenValueType m_maxEigenValue;
        EigenValueType m_minEigenValue;
        EigenValueType m_maxEigenValueNorm;
        std::mutex m_mutex;
        
    };
}

#ifndef ITK_MANUAL_INSTANCIATION
    #include "itkHessianToEigenValues.hxx"
#endif

#endif // itkHessianToEigenValues_h
