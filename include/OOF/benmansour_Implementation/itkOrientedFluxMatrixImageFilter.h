//**********************************************************
//Copyright 2011 Fethallah Benmansour
//
//Licensed under the Apache License, Version 2.0 (the "License"); 
//you may not use this file except in compliance with the License. 
//You may obtain a copy of the License at
//
//http://www.apache.org/licenses/LICENSE-2.0 
//
//Unless required by applicable law or agreed to in writing, software 
//distributed under the License is distributed on an "AS IS" BASIS, 
//WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
//See the License for the specific language governing permissions and 
//limitations under the License.
//**********************************************************

#ifndef __itkOrientedFluxMatrixImageFilter_h
#define __itkOrientedFluxMatrixImageFilter_h

#include <itkImageToImageFilter.h>
#include <itkImage.h>
#include <itkSymmetricSecondRankTensor.h>
#include <itkPixelTraits.h>
#include <itkNthElementImageAdaptor.h>
#include <itkPadImageFilter.h>
#include <itkHalfHermitianToRealInverseFFTImageFilter.h>
#include <itkRealToHalfHermitianForwardFFTImageFilter.h>
#include <itkMultiplyImageFilter.h>
#include <itkImageBoundaryCondition.h>
#include <itkZeroFluxNeumannBoundaryCondition.h>
#include <itkVectorIndexSelectionCastImageFilter.h>



namespace itk
{
	
	/** \class OrientedFluxMatrixImageFilter
	 * \brief This filter takes as input an image
	 * and convolves it with the oriented flux [1] kernel.
	 * The kernels are generated in the Fourier domain.
	 * PixelType of the input image is supposed to be scalar.
	 * PixelType of the output image type is SymmetricSecondRankTensor.
	 *
	 * Instead of apply second order derivatives to the input image, 
	 * the user can decide to run this filter on image gradient.
	 * This could be useful when the gradient are estimated independently,
	 * from color images from instance.
	 *
	 * This code is inspired from itk::FFTConvolutionImageFilter
	 * The main difference is that the kernels are generated in the Fourier domain directly.
	 *
	 * \ref 	 [1]  Max W. K. Law and Albert C. S. Chung, 
	 *	“Three Dimensional Curvilinear Structure Detection using Optimally Oriented Flux”
	 *  The Tenth European Conference on Computer Vision, (ECCV’ 2008) 
	 *
	 * \author : Fethallah Benmansour
	 */
	template <typename TInputImage, 
	typename TOutputImage= Image< SymmetricSecondRankTensor< 
  typename NumericTraits< typename TInputImage::PixelType>::RealType,
    TInputImage::ImageDimension >,
	TInputImage::ImageDimension > >
	class ITK_EXPORT OrientedFluxMatrixImageFilter:
	public ImageToImageFilter<TInputImage,TOutputImage>
	{
	public:
		/** Standard class typedefs. */
		typedef OrientedFluxMatrixImageFilter															Self;
		typedef ImageToImageFilter<TInputImage,TOutputImage>							Superclass;
		typedef SmartPointer<Self>																				Pointer;
		typedef SmartPointer<const Self>																	ConstPointer;
		
		/** Pixel Type of the input image */
		typedef TInputImage																								InputImageType;
		typedef typename InputImageType::Pointer													InputImagePointer;
		typedef typename InputImageType::ConstPointer											InputImageConstPointer;
		typedef typename InputImageType::PixelType												PixelType;
		typedef typename NumericTraits<PixelType>::RealType								RealType;
		typedef typename InputImageType::SizeType													InputSizeType;
		typedef typename InputImageType::RegionType												InputRegionType;
		typedef typename InputImageType::SpacingType											SpacingType;
		typedef typename InputImageType::IndexType												IndexType;
		typedef typename InputImageType::RegionType												RegionType;
		typedef typename InputImageType::SizeType													SizeType;
		typedef typename InputImageType::PointType												PointType;
		/** Internal types used by the FFT filters. */
		typedef float																											InternalPrecision;
		typedef Image< InternalPrecision, TInputImage::ImageDimension >   InternalImageType;
		typedef typename InternalImageType::Pointer                       InternalImagePointerType;
		typedef std::complex< InternalPrecision >                         InternalComplexType;
		typedef Image< InternalComplexType, TInputImage::ImageDimension > InternalComplexImageType;
		typedef typename InternalComplexImageType::Pointer                InternalComplexImagePointerType;
		
		
		/** Typedef to describe the boundary condition. */
		typedef ImageBoundaryCondition< InternalImageType >								BoundaryConditionType;
		typedef BoundaryConditionType *																		BoundaryConditionPointerType;
		typedef ZeroFluxNeumannBoundaryCondition< InternalImageType >			DefaultBoundaryConditionType;
		
		/** FFT filters */
		typedef RealToHalfHermitianForwardFFTImageFilter
		< InternalImageType, InternalComplexImageType >										FFTFilterType;
		
		typedef HalfHermitianToRealInverseFFTImageFilter
		< InternalComplexImageType,	InternalImageType >										IFFTFilterType;
		
		/** Type of the output Image */
		typedef TOutputImage																							OutputImageType;
		typedef typename OutputImageType::Pointer													OutputImagePointer;
		typedef typename OutputImageType::PixelType												OutputPixelType;
		typedef typename PixelTraits<OutputPixelType>::ValueType					OutputComponentType;
		
		/** Image adaptor */
		typedef NthElementImageAdaptor
		<OutputImageType, OutputComponentType>														OutputImageAdaptorType;
		typedef typename OutputImageAdaptorType::Pointer									OutputImageAdaptorPointer;
		
		/**declare types for regions */
		typedef typename InputImageType::RegionType												InputImageRegionType;
    typedef typename OutputImageType::RegionType											OutputImageRegionType;
		
		/** Set/get the boundary condition. */
		itkSetMacro(BoundaryCondition, BoundaryConditionPointerType);
		itkGetConstMacro(BoundaryCondition, BoundaryConditionPointerType);

		/** Image dimension. */
		itkStaticConstMacro(ImageDimension, unsigned int,TInputImage::ImageDimension);
		
		/** Run-time type information (and related methods).   */
		itkTypeMacro( OrientedFluxMatrixImageFilter, ImageToImageFilter );
		
		/** Method for creation through the object factory. */
		itkNewMacro(Self);
		
		/** Set/Get for the smoothing parameter \Sigma0 and for the scale Radius */
		void SetSigma0( RealType sigma0 );
		RealType GetSigma0( );
		void SetRadius( RealType radius);
		RealType GetRadius( );

#ifdef ITK_USE_CONCEPT_CHECKING
		/** Begin concept checking */
		itkConceptMacro(InputHasNumericTraitsCheck,
										(Concept::HasNumericTraits<PixelType>));
		itkConceptMacro(OutputHasPixelTraitsCheck,
										(Concept::HasPixelTraits<OutputPixelType>));
		/** End concept checking */
#endif		
		
	protected:
		
		OrientedFluxMatrixImageFilter();
		virtual ~OrientedFluxMatrixImageFilter() {};
		void PrintSelf(std::ostream& os, Indent indent) const;
		
		void GenerateOrientedFluxMatrixElementKernel(InternalComplexImagePointerType &kernel,
																								 InternalComplexImagePointerType &input, 
																								 unsigned int derivA, unsigned int derivB, 
																								 float radius, float sigma0);
		
		/** Generate Data */
		void GenerateData( );
		
		/** Prepare the input image. This includes padding the image and
		 * taking the Fourier transform of the padded image. */
		void PrepareInput(const InternalImageType * input,
											InternalComplexImagePointerType & preparedInput);
		
		/** Pad the input image. */
		void PadInput(const InternalImageType * input,
									InternalImagePointerType & paddedInput);
		
		/** Take the Fourier transform of the padded input. */
		void TransformPaddedInput(const InternalImageType * paddedInput,
															InternalComplexImagePointerType & transformedInput);
		
		/** Produce output from the final Fourier domain image. */
		void ProduceOutput(InternalComplexImageType * paddedOutput, InternalImagePointerType & internalOutput);
		
		/** Crop the padded version of the output. */
		void CropOutput(InternalImageType * paddedOutput, InternalImagePointerType & croppedOutput);
		
		/** Get the lower bound for the padding of both the kernel and input
		 * images. Assuming that the regions of the kernel and input are the
		 * same, then this lower bound can be used to move the index of the
		 * padded kernel and padded input so that they are the same. This
		 * is important to avoid exceptions in filters that operate on these
		 * images. */
		InputSizeType GetPadLowerBound() const;
		
		/** Get the pad size. */
		InputSizeType GetPadSize() const;
		
		/** Get whether the X dimension has an odd size. */
		bool GetXDimensionIsOdd() const;
		
	private:
		
		OrientedFluxMatrixImageFilter(const Self&); //purposely not implemented
		void operator=(const Self&); //purposely not implemented
		
		RealType											m_Sigma0;
		RealType											m_Radius;
		
		DefaultBoundaryConditionType	m_DefaultBoundaryCondition;
		BoundaryConditionPointerType	m_BoundaryCondition;
		
		OutputImageAdaptorPointer			m_ImageAdaptor;
	};
	
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkOrientedFluxMatrixImageFilter.hxx"
#endif

#endif
