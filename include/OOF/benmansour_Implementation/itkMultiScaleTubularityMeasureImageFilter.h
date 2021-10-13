//**********************************************************
//Copyright 2011 Engin Turetken & Fethallah Benmansour
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

#ifndef __itkMultiScaleTubularityMeasureImageFilter_h
#define __itkMultiScaleTubularityMeasureImageFilter_h

#include <itkImageToImageFilter.h>
#include <itkImage.h>
#include <itkImageBase.h>
#include <itkOrientedFluxMatrixImageFilter.h>
#include <itkTimeProbe.h>

namespace itk
{
	/**\class MultiScaleTubularityMeasureImageFilter
	 * \brief A filter to enhance tubular or ridge-like structures based on 
	 * multi-scale oriented flux eigensystem analysis.
	 *
	 *
	 * This filter evaluates an oriented flux-based [1] tubularity measure 
	 * at different scale levels (i.e., tube radius) for the structures. The tubularity measure 
	 * is computed by another filter, which is passed to this filter as a template argument.
	 * The measure is computed from the oriented flux matrix image at each scale 
	 * level and hence, for an N-D input image, the filter generates an (N+1)-D measure 
	 * image. The last dimension denotes the scale axis. In addition, by storing the maximum measure 
	 * response along the scale dimension, the filter can also output an N-D measure image as well
	 * as a scale image. Finally, the filter can also output the N-D and (N+1)-D oriented flux 
	 * matrix images, which are used to compute the measure images.
	 *
	 *
	 * Minimum and maximum scale values can be set using SetMinSigma and SetMaxSigma
	 * methods respectively. The number of scale levels is set using 
	 * SetNumberOfSigmaSteps method. Linearly  distributed scale levels are 
	 * computed within the bound set by the minimum and maximum values. 
	 *  
	 *
	 * \authors: Engin Turetken & Fethallah Benmansour
	 *
	 * \sa HessianToObjectnessMeasureImageFilter 
	 * \sa Hessian3DToVesselnessMeasureImageFilter 
	 * \sa HessianSmoothed3DToVesselnessMeasureImageFilter 
	 * \sa HessianRecursiveGaussianImageFilter 
	 * \sa SymmetricEigenAnalysisImageFilter
	 * \sa SymmetricSecondRankTensor
	 * 
	 * \ingroup IntensityImageFilters TensorObjects
	 *
	 * \ref 	 [1]  Max W. K. Law and Albert C. S. Chung, 
	 *	“Three Dimensional Curvilinear Structure Detection using Optimally Oriented Flux”
	 *  The Tenth European Conference on Computer Vision, (ECCV’ 2008)	 
	 *
	 */
	
	template <typename TInputImage,
	typename TOrientedFluxImage, 
	typename TScaleImage,
	typename TOrientedFluxToMeasureFilter,
	typename TOutputNDImage = itk::Image< typename NumericTraits<typename TInputImage::PixelType >::ScalarRealType, TInputImage::ImageDimension> >

	class ITK_EXPORT MultiScaleTubularityMeasureImageFilter 
	: public ImageToImageFilter< TInputImage, TOutputNDImage > 
	{
	public:
		/** Standard class typedefs. */
		typedef MultiScaleTubularityMeasureImageFilter														Self;
		typedef ImageToImageFilter<TInputImage, TOutputNDImage>										Superclass;
		typedef SmartPointer<Self>																								Pointer;
		typedef SmartPointer<const Self>																					ConstPointer;
		
		typedef typename NumericTraits
		<typename TInputImage::PixelType>::ScalarRealType													RealType;
		typedef TInputImage																												InputImageType;
		typedef TOutputNDImage																										OutputNDImageType;
		typedef TOrientedFluxImage																								OrientedFluxImageType;
		typedef OrientedFluxMatrixImageFilter
		<InputImageType, OrientedFluxImageType>																		OrientedFluxFilterType;
		typedef TScaleImage																												ScaleImageType;
		typedef TOrientedFluxToMeasureFilter																			OrientedFluxToMeasureFilterType;
		
		typedef Image<typename OrientedFluxImageType::PixelType, 
		OrientedFluxImageType::ImageDimension + 1>			NPlus1DOrientedFluxImageType;
		typedef	Image<typename OutputNDImageType::PixelType, 
		OutputNDImageType::ImageDimension + 1>					OutputNPlus1DImageType;
		
		typedef typename TInputImage::PixelType																		InputPixelType;
		typedef typename TInputImage::RegionType																	InputRegionType;
		typedef typename TOutputNDImage::PixelType																OutputNDPixelType;
		typedef typename TOutputNDImage::RegionType																OutputNDRegionType;
		typedef typename OutputNPlus1DImageType::RegionType												OutputNPlus1DRegionType;
		
		typedef ImageToImageFilterDetail::ImageRegionCopier<OutputNPlus1DImageType::ImageDimension,
		InputImageType::ImageDimension>																						InputToOutputRegionCopierType;
		
		/** Image dimension. */
		itkStaticConstMacro(ImageDimension, unsigned int, InputImageType::ImageDimension);
		
		/** Types for Scale image */
		typedef typename ScaleImageType::PixelType																ScalePixelType;
				
		/** Update image buffer that holds the best objectness response. This is not redundant from
		 the output image because the latter may not be of float type, which is required for the comparisons 
		 between responses at different scales. */ 
		typedef Image< double, itkGetStaticConstMacro(ImageDimension) >	UpdateBufferType;
		typedef typename UpdateBufferType::ValueType BufferValueType;
    
		typedef typename Superclass::DataObjectPointer	DataObjectPointer;
		
		typedef OrientedFluxMatrixImageFilter
		< InputImageType, OrientedFluxImageType > FFTOrientedFluxType;
		typedef typename OutputNDImageType::Pointer	OutputNDImagePointer;
		
		/** Method for creation through the object factory. */
		itkNewMacro(Self);
		
		/** Runtime information support. */
		itkTypeMacro(MultiScaleTubularityMeasureImageFilter, ImageToImageFilter);
		
		/** 
		 * Set the minimum radius value (measured in world coordinates) 
		 * for tubular structures.
		 */
		itkSetMacro(SigmaMinimum, double);
		
		/** 
		 * Get the minimum radius value (measured in world coordinates) 
		 * for tubular structures.
		 */
		itkGetConstMacro(SigmaMinimum, double);
		
		/** 
		 * Set the maximum radius value (measured in world coordinates) 
		 * for tubular structures.
		 */
		itkSetMacro(SigmaMaximum, double);
		
		/** 
		 * Get the maximum radius value (measured in world coordinates) 
		 * for tubular structures.
		 */
		itkGetConstMacro(SigmaMaximum, double);
		
		/** 
		 * Set the number of radius levels that will be used in the 
		 * scale-space tubularity analysis. The size of the last dimension of (N+1)-D 
		 * output images is equal to the number of radius levels.
		 */
		virtual void SetNumberOfSigmaSteps(unsigned int );
		
		/** 
		 * Get the number of radius levels that will be used in the 
		 * scale-space tubularity analysis. The size of the last dimension of (N+1)-D 
		 * output images is equal to the number of radius levels.
		 */
		itkGetConstMacro(NumberOfSigmaSteps, unsigned int);
		
		/** 
		 * Set the fixed sigma value of the oriented flux image.
		 * This is the sigma of the Gaussian function used to slightly 
		 * smooth the input image prior to computing the oriented flux matrices.
		 */
		itkSetMacro(FixedSigmaForOrientedFluxImage, double);
		
		/** 
		 * Get the fixed sigma value of the oriented flux image.
		 * This is the sigma of the Gaussian function used to slightly 
		 * smooth the input image prior to computing the oriented flux matrices.
		 */
		itkGetConstMacro(FixedSigmaForOrientedFluxImage, double);
		
		/** 
		 * Get the N-D image containing the oriented flux matrices computed at the best
		 * response scale.
		 */
		virtual OrientedFluxImageType* GetOrientedFluxOutput();
		
		/** 
		 * Get the (N+1)-D image containing the  oriented flux matrices computed at all 
		 * the scales.
		 */
		virtual NPlus1DOrientedFluxImageType* GetNPlus1DOrientedFluxOutput();
		
		/** 
		 * Get the N-D scale image which is computed by storing the radius value associated 
		 * with the highest tubularity measure.
		 */
		virtual ScaleImageType* GetScaleOutput();
		
		/** 
		 * Get the (N+1)-D image containing the oriented flux based tubularity measure
		 * responses at all scales. To get the N-D tubularity measure image, call 
		 * the GetOutput() method instead.
		 */
		virtual OutputNPlus1DImageType* GetNPlus1DImageOutput();
		
		virtual void EnlargeOutputRequestedRegion (DataObject *);
		
		/** 
		 * Enable or disable generation of the scale image.
		 * By default, it is disabled (false).
		 */
		itkSetMacro(GenerateScaleOutput,bool);
		
		/** 
		 * Enable or disable generation of the scale image.
		 * By default, it is disabled (false).
		 */		
		itkBooleanMacro(GenerateScaleOutput);
		
		/** 
		 * Get the boolean flag for generation of the scale image.
		 */
		itkGetConstMacro(GenerateScaleOutput,bool);
		
		
		
		/** 
		 * Enable or disable generation of the N-D oriented flux image.
		 * By default, it is disabled (false).
		 */
		itkSetMacro(GenerateOrientedFluxOutput,bool);
		
		/** 
		 * Enable or disable generation of the N-D oriented flux image.
		 * By default, it is disabled (false).
		 */
		itkBooleanMacro(GenerateOrientedFluxOutput);
		
		/** 
		 * Get the boolean flag for generation of the N-D oriented flux image.
		 */
		itkGetConstMacro(GenerateOrientedFluxOutput,bool);
		
		
		
		/** 
		 * Enable or disable generation of the (N+1)-D oriented flux image.
		 * By default, it is disabled (false).
		 */
		itkSetMacro(GenerateNPlus1DOrientedFluxOutput,bool);
		
		/** 
		 * Enable or disable generation of the (N+1)-D oriented flux image.
		 * By default, it is disabled (false).
		 */
		itkBooleanMacro(GenerateNPlus1DOrientedFluxOutput);

		/** 
		 * Get the boolean flag for generation of the (N+1)-D oriented flux image.
		 */
		itkGetConstMacro(GenerateNPlus1DOrientedFluxOutput,bool);
		
		
		
		/** 
		 * Enable or disable generation of the (N+1)-D tubularity measure image.
		 * By default, it is disabled (false). The only image that is produced 
		 * by default is the N-D tubularity measure image.
		 */
		itkSetMacro(GenerateNPlus1DOrientedFluxMeasureOutput,bool);
		
		/** 
		 * Enable or disable generation of the (N+1)-D tubularity measure image.
		 * By default, it is disabled (false). The only image that is produced 
		 * by default is the N-D tubularity measure image.
		 */
		itkBooleanMacro(GenerateNPlus1DOrientedFluxMeasureOutput);
		
		/** 
		 * Get the boolean flag for generation of the (N+1)-D tubularity measure image.
		 */
		itkGetConstMacro(GenerateNPlus1DOrientedFluxMeasureOutput,bool);

		
		
		
		/** 
		 * Set the brightness flag that determines whether the image intensities on 
		 * tubular structures are higher (or lower) than those on the background.
		 * Default value is true (i.e., bright structures).
		 */
		itkSetMacro(BrightObject,bool);
		
		/** 
		 * Set the brightness flag that determines whether the image intensities on 
		 * tubular structures are higher (or lower) than those on the background.
		 * Default value is true (i.e., bright structures).
		 */
		itkBooleanMacro(BrightObject);
		
		/** 
		 * Get the brightness flag that determines whether the image intensities on 
		 * tubular structures are higher (or lower) than those on the background.
		 */		
		itkGetConstMacro(BrightObject,bool);
		
		
		
		/** This is overloaded to create the Scale and oriented flux output images */
		virtual DataObjectPointer MakeOutput(DataObject::DataObjectPointerArraySizeType idx);		
		
#ifdef ITK_USE_CONCEPT_CHECKING
		/** Begin concept checking */
		itkConceptMacro(SameDimensionCheck,
										(Concept::SameDimension< TInputImage::ImageDimension, 
										 TOrientedFluxImage::ImageDimension>));
		itkConceptMacro(SameDimensionCheck2,
										(Concept::SameDimension<
										 TInputImage::ImageDimension, 
										 TScaleImage::ImageDimension>));
		itkConceptMacro(SameDimensionCheck3,
										(Concept::SameDimension<
										 TInputImage::ImageDimension, 
										 TOutputNDImage::ImageDimension>));
		itkConceptMacro(InputHasNumericTraitsCheck,
										(Concept::HasNumericTraits<InputPixelType>));
		itkConceptMacro(OrientedFluxMatrixHasPixelTraitsCheck,
										(Concept::HasPixelTraits<typename TOrientedFluxImage::PixelType>));
		itkConceptMacro(ScaleHasValueTypeCheck,
										(Concept::HasNumericTraits<typename TScaleImage::PixelType>));
		itkConceptMacro(OutputHasNumericTraitsCheck,
										(Concept::HasNumericTraits<typename TOutputNDImage::PixelType>));
		itkConceptMacro(OrientedFluxMatrixHasBracketOperatorCheck,
										(Concept::BracketOperator<typename TOrientedFluxImage::PixelType, 
										 unsigned long, typename TOrientedFluxImage::PixelType::ComponentType>));
		/** End concept checking */
#endif
		
	protected:
		MultiScaleTubularityMeasureImageFilter();
		virtual ~MultiScaleTubularityMeasureImageFilter() {};
		void PrintSelf(std::ostream& os, Indent indent) const;
		
		
		virtual void CallCopyInputRegionToOutputRegion(OutputNPlus1DRegionType &destRegion,
																									 const InputRegionType &srcRegion);
		virtual void GenerateOutputInformation();
		
		virtual void AllocateOutputs(); 
		
		/** Generate Data */
		virtual void GenerateData( void );
		
	private:
		void UpdateMaximumResponse(double sigma, unsigned int scaleLevel);
		double ComputeSigmaValue(int scaleLevel);
		
		void AllocateUpdateBuffer();
		
		MultiScaleTubularityMeasureImageFilter(const Self&); //purposely not implemented
		void operator=(const Self&); //purposely not implemented
		
		double																						m_SigmaMinimum;
		double																						m_SigmaMaximum;
		unsigned int																			m_NumberOfSigmaSteps;
		std::vector< RealType >														m_Sigmas;
		
		double																						m_FixedSigmaForOrientedFluxImage;
		
		std::vector<typename OrientedFluxToMeasureFilterType::Pointer>		
																											m_OrientedFluxToMeasureFilterList;
		typename UpdateBufferType::Pointer	m_UpdateBuffer;
		
		bool																							m_GenerateScaleOutput;
		bool																							m_GenerateOrientedFluxOutput;
		bool																							m_GenerateNPlus1DOrientedFluxOutput;	
		bool																							m_GenerateNPlus1DOrientedFluxMeasureOutput;
		
		bool																							m_BrightObject;

	};
	
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMultiScaleTubularityMeasureImageFilter.hxx"
#endif

#endif
