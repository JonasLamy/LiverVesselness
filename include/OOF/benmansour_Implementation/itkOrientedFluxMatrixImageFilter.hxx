//**********************************************************
//Copyright 2012 Fethallah Benmansour
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


#ifndef __itkOrientedFluxMatrixImageFilter_hxx
#define __itkOrientedFluxMatrixImageFilter_hxx

#include "itkOrientedFluxMatrixImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkExtractImageFilter.h"
#include "itkMath.h"
#include "vnl/vnl_bessel.h"

namespace itk
{
	/**
	 * Constructor
	 */
	template <typename TInputImage, typename TOutputImage >
	OrientedFluxMatrixImageFilter<TInputImage, TOutputImage >
	::OrientedFluxMatrixImageFilter()
	{
		m_Sigma0								= 1.0;
		m_Radius								= 1.0;
		m_BoundaryCondition			= &m_DefaultBoundaryCondition;
		m_ImageAdaptor					= OutputImageAdaptorType::New();
	}
	
	/**
	 * Set Sigma0
	 */
	template <typename TInputImage, typename TOutputImage >
	void
	OrientedFluxMatrixImageFilter<TInputImage, TOutputImage >
	::SetSigma0( RealType sigma0 )
	{
		if(m_Sigma0 != sigma0)
		{
			m_Sigma0 = sigma0;
			this->Modified();
		}
	}
	
	/**
	 * Get Sigma0
	 */
	template <typename TInputImage, typename TOutputImage >
	typename OrientedFluxMatrixImageFilter<TInputImage, TOutputImage >::RealType
	OrientedFluxMatrixImageFilter<TInputImage, TOutputImage >
	::GetSigma0( )
	{
		return	m_Sigma0;
	}
	
	/**
	 * Set Radius
	 */
	template <typename TInputImage, typename TOutputImage >
	void
	OrientedFluxMatrixImageFilter<TInputImage, TOutputImage >
	::SetRadius( RealType radius )
	{
		if(m_Radius != radius)
		{
			m_Radius = radius;
			this->Modified();
		}
	}

	/**
	 * Get Radius
	 */
	template <typename TInputImage, typename TOutputImage >
	typename OrientedFluxMatrixImageFilter<TInputImage, TOutputImage >::RealType
	OrientedFluxMatrixImageFilter<TInputImage, TOutputImage >
	::GetRadius( )
	{
		return m_Radius;
	}
		
	/***************************************************************************************
	 *  For 2 given directions, Generates the oriented flux matix kernel in the fourier
	 *  domain as Given by Eq.8 in:
	 *  Max W. K. Law and Albert C. S. Chung, 
	 *	“Three Dimensional Curvilinear Structure Detection using Optimally Oriented Flux”
	 *  The Tenth European Conference on Computer Vision, (ECCV’ 2008)
	 *
	 * \author Fethallah Benmansour
	 ***************************************************************************************/
	template <typename TInputImage, typename TOutputImage >
	void
	OrientedFluxMatrixImageFilter<TInputImage, TOutputImage >
	::GenerateOrientedFluxMatrixElementKernel(InternalComplexImagePointerType &kernel,
																					InternalComplexImagePointerType &input, 
																					unsigned int derivA, unsigned int derivB, 
																					float radius, float sigma0)
	{
		// i and j should both be smaller than the dimension
		if (derivA >= ImageDimension || derivB >= ImageDimension)
		{
			itkGenericExceptionMacro("Derivatives along the dimensions, these indices should be less than the dimension and positive");
		}
		RegionType region = input->GetLargestPossibleRegion();
		SpacingType imageSpacing = input->GetSpacing();
		SizeType    size    = region.GetSize();
		IndexType  start    = region.GetIndex();
		
		SpacingType freqSpacing;
		PointType  origin;
		for(unsigned int i = 0; i < ImageDimension; i++)
		{
			origin[i] = -0.5/ imageSpacing[i];
			if(i != 0)
			{
				freqSpacing[i] = (1.0 / static_cast<double>(size[i]-1)) / imageSpacing[i];
			}
			else
			{				
				freqSpacing[i] = (0.5 / static_cast<double>(size[i]-1)) / imageSpacing[i];
			}
		}
		kernel = InternalComplexImageType::New();
		kernel->CopyInformation( input );
		kernel->SetSpacing( freqSpacing );
		kernel->SetOrigin(origin);
		kernel->SetBufferedRegion( input->GetBufferedRegion() );
		kernel->Allocate();
		// The multiply image filter checks that the 2 images occupy exactly the same space
		// therefore, the physical space of the fourtier transform of the input image needs to be modified
		input->SetSpacing( freqSpacing );
		input->SetOrigin(origin);		
		// 2 corners per dimension except for the X dimension.
		unsigned int nbCorners = pow(float(2),float( ImageDimension-1));
		typedef std::vector<IndexType> listOfIndexCorners;
		typedef std::vector<PointType> listOfPointCorners;
		listOfIndexCorners listOfIdxC;
		listOfPointCorners listOfPtsC;
		listOfIdxC.clear();
		listOfPtsC.clear();
		// Get the corners (2D) *
		//*--------------/
		//---------------/
		//---------------/
		//---------------/
		//---------------/
		//---------------/
		//*--------------/
		for(unsigned int i = 0; i < nbCorners; i++)
		{
			IndexType corner;
			unsigned int idx = 0;
			corner[idx] = start[idx]; 
			idx++;
			unsigned int k = i;
			for(unsigned int j = 0; j < ImageDimension-1; j++)
			{
				if( k%2 )
				{
					corner[idx] = start[idx];
				}
				else
				{
					corner[idx] = start[idx]+size[idx]-1;
				}
				idx++;
				k /= 2;
			}
			listOfIdxC.push_back(corner);
		}
		// Get the physical location of these corners
		PointType pt;
		for(unsigned int i = 0; i < nbCorners; i++)
		{
			kernel->TransformIndexToPhysicalPoint(listOfIdxC[i], pt);
			listOfPtsC.push_back(pt);
		}
		// computation of the kernel 
		double eps = itk::NumericTraits<float>::epsilon();	
		double Ui = 0.0, Uj = 0.0;
		double normU;
		
		typedef itk::ImageRegionIterator< InternalComplexImageType > ImageIterator;
		ImageIterator  it( kernel, kernel->GetLargestPossibleRegion() );
		it.GoToBegin();
		//Compute the kernel here
		while(!it.IsAtEnd())
		{
			IndexType index  = it.GetIndex();
			PointType point;
			kernel->TransformIndexToPhysicalPoint(index, point);
			//compute norm of U
			normU = 1e9;
			for(unsigned int i = 0; i < nbCorners; i++)
			{
				double distToCorner = point.EuclideanDistanceTo(listOfPtsC[i]);
				if (normU > distToCorner)
				{
					normU = distToCorner;
					Ui = point[derivA] - (listOfPtsC[i])[derivA];
					Uj = point[derivB] - (listOfPtsC[i])[derivB];
				}
			}
			
			InternalComplexType value(0.0, 0.0);
			if(normU >= eps)
			{
				double phase = 2.0 * vnl_math::pi * radius * normU;
				double piNormUSigma	= (vnl_math::pi * normU * sigma0);
				InternalComplexType derivativesTerm(0.0, 0.0);
				// second order derivatives.
				derivativesTerm = InternalComplexType(-exp( -2.0* piNormUSigma * piNormUSigma )* Ui * Uj, 0.0);

				InternalComplexType kernelTerm(0.0, 0.0);
				if(ImageDimension%2 == 0)
				{// scale normalized response
					unsigned int p = ImageDimension / 2; 
					kernelTerm  = InternalComplexType(vnl_bessel(p, phase) / (std::pow(double(normU), double(p)) * std::pow(double(radius), double(p-1))), 0.0);
				}
				else if(ImageDimension == 3)
				{// scale normalized response
					kernelTerm  = InternalComplexType(( sin(phase)/phase - cos(phase) ) / (radius * normU*normU), 0.0);
				}
				else
				{
					itkGenericExceptionMacro("Oriented Flux filter in the Fourier imlemented only for dimensions 2 and 3 and 4, and all even dimensions");
				}
				value = kernelTerm * derivativesTerm;
			}
			it.Set(value);
			++it;
		}		
	}
	
	/**
	 * Generate data:
	 * Compute the convolution component per component in the Fourier domain 
	 *
	 * \author: F. Benmansour
	 */
	template <typename TInputImage, typename TOutputImage >
	void
	OrientedFluxMatrixImageFilter<TInputImage, TOutputImage >
	::GenerateData( )
	{
		InputImageConstPointer inputImage  = this->GetInput();
		OutputImagePointer		 output = this->GetOutput();		
		if( inputImage.IsNull() )
		{
			itkExceptionMacro("Input image must be provided");
		}
		typedef CastImageFilter< InputImageType, InternalImageType > InputCastFilterType;
		typename InputCastFilterType::Pointer inputCaster = InputCastFilterType::New();
		inputCaster->SetNumberOfThreads( this->GetNumberOfThreads() );
		inputCaster->SetInput( inputImage );
		inputCaster->ReleaseDataFlagOn();
		inputCaster->Update();
		
		typename InternalImageType::Pointer localInput = InternalImageType::New();
		localInput->Graft( inputCaster->GetOutput() );
		localInput->Update();
		
		InternalComplexImagePointerType inputFourierTransform = nullptr;
		InternalComplexImagePointerType kernel = nullptr;
		
		PrepareInput( localInput, inputFourierTransform );
		
		//The original spacing is needed for generating properly the kernels
		SpacingType originalSpacing = inputImage->GetSpacing();
		// Prepare Image adaptor
		m_ImageAdaptor->SetImage( this->GetOutput() );
		m_ImageAdaptor->SetLargestPossibleRegion( this->GetInput()->GetLargestPossibleRegion() );
		m_ImageAdaptor->SetBufferedRegion( this->GetInput()->GetBufferedRegion() );
		m_ImageAdaptor->SetRequestedRegion( this->GetInput()->GetRequestedRegion() );
		m_ImageAdaptor->Allocate();
		unsigned int element = 0;
		for(unsigned int i = 0; i < ImageDimension; i++)
		{
			for(unsigned int j = i; j < ImageDimension; j++)
			{
				InternalComplexImagePointerType kernel = nullptr;
				// the spacing needs to be reset to the spacing of the input image for 2 reasons.
				// First, it's needed to generate properly the kernel(s).
				// Second, the spacing of inputFourierTransform is modified in GenerateOrientedFluxMatrixElementKernel.
				// this is done because the multiply image filter require that the 2 input images occupy 
				// the exact same physical domain.
				inputFourierTransform->SetSpacing( originalSpacing );
				GenerateOrientedFluxMatrixElementKernel( kernel, inputFourierTransform, i, j, this->GetRadius(), this->GetSigma0() );
				typedef itk::MultiplyImageFilter< InternalComplexImageType,
				InternalComplexImageType,
				InternalComplexImageType > MultType;
				typename MultType::Pointer multiplyFilter = MultType::New();
				multiplyFilter->SetInput1( inputFourierTransform );
				multiplyFilter->SetInput2( kernel );
				multiplyFilter->SetNumberOfThreads( this->GetNumberOfThreads() );
				multiplyFilter->SetReleaseDataFlag( true );
				multiplyFilter->SetInPlace( false );
				multiplyFilter->Update();
				// Free up the memory for the prepared kernel
				kernel = NULL;
				InternalImagePointerType croppedOutput = nullptr;
				this->ProduceOutput( multiplyFilter->GetOutput(), croppedOutput );
				ImageRegionIteratorWithIndex< InternalImageType > it(croppedOutput, croppedOutput->GetRequestedRegion());
				m_ImageAdaptor->SelectNthElement( element++ );
				ImageRegionIteratorWithIndex< OutputImageAdaptorType > ot( m_ImageAdaptor, m_ImageAdaptor->GetRequestedRegion());
				it.GoToBegin();
				ot.GoToBegin();
				while( !it.IsAtEnd() )
				{
					ot.Set( it.Get() );
					++it;
					++ot;
				}
			}
		}
	}
	
	template <typename TInputImage, typename TOutputImage >
	void
	OrientedFluxMatrixImageFilter<TInputImage, TOutputImage >
	::PrepareInput(const InternalImageType * input,
								 InternalComplexImagePointerType & preparedInput)
	{
		InternalImagePointerType paddedInput;
		this->PadInput( input, paddedInput );
		this->TransformPaddedInput( paddedInput, preparedInput );
	}
	
	template <typename TInputImage, typename TOutputImage >
	void
	OrientedFluxMatrixImageFilter<TInputImage, TOutputImage >
	::TransformPaddedInput(const InternalImageType * paddedInput,
												 InternalComplexImagePointerType & transformedInput)
	{
		// Take the Fourier transform of the padded image.
		typename FFTFilterType::Pointer imageFFTFilter = FFTFilterType::New();
		imageFFTFilter->SetNumberOfThreads( this->GetNumberOfThreads() );
		imageFFTFilter->SetInput( paddedInput );
		imageFFTFilter->Update();
		
		transformedInput = imageFFTFilter->GetOutput();
	}
	
	template <typename TInputImage, typename TOutputImage >
	void
	OrientedFluxMatrixImageFilter<TInputImage, TOutputImage >
	::PadInput(const InternalImageType * input,
						 InternalImagePointerType & paddedInput)
	{
		// Pad the image
		InputSizeType padSize = this->GetPadSize();
		InputRegionType inputRegion = input->GetLargestPossibleRegion();
		InputSizeType inputSize = inputRegion.GetSize();
		
		typedef PadImageFilter< InternalImageType, InternalImageType > InputPadFilterType;
		typename InputPadFilterType::Pointer inputPadder = InputPadFilterType::New();
		inputPadder->SetBoundaryCondition( this->GetBoundaryCondition() );
		
		InputSizeType inputLowerBound = this->GetPadLowerBound();
		inputPadder->SetPadLowerBound( inputLowerBound );
		
		InputSizeType inputUpperBound;
		for (unsigned int i = 0; i < ImageDimension; ++i)
    {
			inputUpperBound[i] = (padSize[i] - inputSize[i]) / 2;
			if ( ( padSize[i] - inputSize[i] ) % 2 == 1 )
      {
				inputUpperBound[i]++;
      }
    }
		inputPadder->SetPadUpperBound( inputUpperBound );
		inputPadder->SetNumberOfThreads( this->GetNumberOfThreads() );
		inputPadder->SetInput( input );
		inputPadder->ReleaseDataFlagOn();
		inputPadder->Update();
		
		paddedInput = inputPadder->GetOutput();
	}
	
	template <typename TInputImage, typename TOutputImage >
	typename OrientedFluxMatrixImageFilter<TInputImage, TOutputImage >::InputSizeType
	OrientedFluxMatrixImageFilter<TInputImage, TOutputImage >
	::GetPadLowerBound() const
	{
		typename InputImageType::ConstPointer inputImage = this->GetInput();
		InputSizeType inputSize = inputImage->GetLargestPossibleRegion().GetSize();
		InputSizeType padSize = this->GetPadSize();
		
		InputSizeType inputLowerBound;
		for (unsigned int i = 0; i < ImageDimension; ++i)
    {
			inputLowerBound[i] = (padSize[i] - inputSize[i]) / 2;
    }
		return inputLowerBound;
	}
	
	template <typename TInputImage, typename TOutputImage >
	typename OrientedFluxMatrixImageFilter<TInputImage, TOutputImage >::InputSizeType
	OrientedFluxMatrixImageFilter<TInputImage, TOutputImage >
	::GetPadSize() const
	{
		typename InputImageType::ConstPointer inputImage = this->GetInput();
		InputSizeType inputSize = inputImage->GetLargestPossibleRegion().GetSize();
		//RealType radius   = this->GetRadius();
		double minSpacing = inputImage->GetSpacing().GetVnlVector().min_value();
		unsigned int halfWindowSize = itk::Math::Round<unsigned int>(m_Radius/minSpacing) + 1;
		InputSizeType kernelSize;
		kernelSize.Fill( 2*halfWindowSize + 1 );
		InputSizeType padSize;
		for (unsigned int i = 0; i < ImageDimension; ++i)
    {
			padSize[i] = inputSize[i] + kernelSize[i];
			// Use the valid sizes for VNL because they are fast sizes for
			// both VNL and FFTW.
			while ( !VnlFFTCommon::IsDimensionSizeLegal( padSize[i] ) )
      {
				padSize[i]++;
      }
    }
		return padSize;
	}
	
	template <typename TInputImage, typename TOutputImage >
	bool
	OrientedFluxMatrixImageFilter<TInputImage, TOutputImage >
	::GetXDimensionIsOdd() const
	{
		InputSizeType padSize = this->GetPadSize();
		return (padSize[0] % 2 != 0);
	}
	
	template <typename TInputImage, typename TOutputImage >
	void
	OrientedFluxMatrixImageFilter<TInputImage, TOutputImage >
	::ProduceOutput(InternalComplexImageType * paddedOutput, InternalImagePointerType & internalOutput)
	{
		typename IFFTFilterType::Pointer ifftFilter = IFFTFilterType::New();
		ifftFilter->SetActualXDimensionIsOdd( this->GetXDimensionIsOdd() );
		ifftFilter->SetNumberOfThreads( this->GetNumberOfThreads() );
		ifftFilter->SetInput( paddedOutput );
		ifftFilter->ReleaseDataFlagOn();
		
		this->CropOutput( ifftFilter->GetOutput(), internalOutput );
	}
	
	template <typename TInputImage, typename TOutputImage >
	void
	OrientedFluxMatrixImageFilter<TInputImage, TOutputImage >
	::CropOutput(InternalImageType * paddedOutput, InternalImagePointerType & croppedOutput)
	{
		//Initialize the cropped image
		croppedOutput = InternalImageType::New();
		croppedOutput->SetRequestedRegion( this->GetInput()->GetRequestedRegion() );
		croppedOutput->SetLargestPossibleRegion( this->GetInput()->GetLargestPossibleRegion() );
		croppedOutput->SetBufferedRegion( this->GetInput()->GetLargestPossibleRegion() );
		croppedOutput->CopyInformation( this->GetInput() );
		croppedOutput->Allocate();
		// Now crop the output to the desired size.
		typedef ExtractImageFilter< InternalImageType, InternalImageType > ExtractFilterType;
		typename ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
		extractFilter->SetDirectionCollapseToSubmatrix();
		extractFilter->InPlaceOn();
		extractFilter->GraftOutput( croppedOutput );
		// crop to the same region as the original image
		InputRegionType sameRegion = croppedOutput->GetLargestPossibleRegion();
		extractFilter->SetExtractionRegion( sameRegion );
		// Graft the minipipeline output to this filter.
		extractFilter->SetNumberOfThreads( this->GetNumberOfThreads() );
		extractFilter->SetInput( paddedOutput );
		extractFilter->GetOutput()->SetRequestedRegion( croppedOutput->GetRequestedRegion() );
		extractFilter->Update();
	}
	
	template <typename TInputImage, typename TOutputImage >
	void
	OrientedFluxMatrixImageFilter<TInputImage, TOutputImage >
	::PrintSelf(std::ostream& os, Indent indent) const
	{
		Superclass::PrintSelf(os,indent);
		os << indent << "Sigma0 (for smoothing): " << std::endl << this->m_Sigma0 << std::endl;
		os << indent << "Radius: " << std::endl << this->m_Radius << std::endl;
		os << indent << "ImageAdaptor: " << std::endl << this->m_ImageAdaptor << std::endl;
	}
} // end namespace itk

#endif
