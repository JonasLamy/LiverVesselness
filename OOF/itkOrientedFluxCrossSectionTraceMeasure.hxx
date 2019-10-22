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


#ifndef __itkOrientedFluxCrossSectionTraceMeasureFilter_txx
#define __itkOrientedFluxCrossSectionTraceMeasureFilter_txx

#include "itkOrientedFluxCrossSectionTraceMeasure.h"

namespace itk
{
	/**
	 * Constructor
	 */
	template <typename TInputImage, typename TOutputImage >
	OrientedFluxCrossSectionTraceMeasureFilter<TInputImage,TOutputImage>
	::OrientedFluxCrossSectionTraceMeasureFilter()
	{
		this->DynamicMultiThreadingOff();
		m_IsBright = true;
	}
	
	/**
	 * Set Bright Object
	 */
	template <typename TInputImage, typename TOutputImage>
	void
	OrientedFluxCrossSectionTraceMeasureFilter<TInputImage,TOutputImage>
	::SetBrightObject( bool brightObj )
	{
		m_IsBright = brightObj;
		
		this->Modified();
	}
	
	/**
	 * Set Bright Object
	 */
	template <typename TInputImage, typename TOutputImage>
	bool
	OrientedFluxCrossSectionTraceMeasureFilter<TInputImage,TOutputImage>
	::GetBrightObject( ) const
	{
		return m_IsBright;
	}
	
	//
	//
	//
	template <typename TInputImage, typename TOutputImage>
	void
	OrientedFluxCrossSectionTraceMeasureFilter<TInputImage,TOutputImage>
	::GenerateInputRequestedRegion()
	{
		// call the superclass' implementation of this method. this should
		// copy the output requested region to the input requested region
		Superclass::GenerateInputRequestedRegion();
		
		// This filter needs all of the input
		typename OrientedFluxCrossSectionTraceMeasureFilter<TInputImage,TOutputImage>::InputImagePointer image = const_cast<InputImageType *>( this->GetInput() );
		if (image)
    {
			image->SetRequestedRegion( this->GetInput()->GetLargestPossibleRegion() );
    }
	}
	
	
	//
	//
	//
	template <typename TInputImage, typename TOutputImage>
	void
	OrientedFluxCrossSectionTraceMeasureFilter<TInputImage,TOutputImage>
	::EnlargeOutputRequestedRegion(DataObject *output)
	{
		TOutputImage *out = dynamic_cast<TOutputImage*>(output);
		
		if (out)
    {
			out->SetRequestedRegion( out->GetLargestPossibleRegion() );
    }
	}
	
	
	template <typename TInputImage, typename TOutputImage>
	void
	OrientedFluxCrossSectionTraceMeasureFilter<TInputImage,TOutputImage>
	::BeforeThreadedGenerateData( )
	{
		//Get Input and Output
		InputImageConstPointer input  = this->GetInput();
		
		if ( input.IsNull() )
		{
			itkExceptionMacro( "Input image must be provided" );
		}
		
		/**Eigen Analisys Filter */
		m_eigenAnalysisFilter = EigenAnalysisImageFilterType::New();
		//m_eigenAnalysisFilter->OrderEigenValuesBy(	EigenAnalysisImageFilterType::EigenValueOrderType::OrderByMagnitude );
		//m_eigenAnalysisFilter->SetDimension( ImageDimension );
		m_eigenAnalysisFilter->SetInput( input );
		m_eigenAnalysisFilter->Update();
		m_maxImageEigenValue = m_eigenAnalysisFilter->GetMaxEigenValue();
		
		OutputImagePointer     output = this->GetOutput();
		output->CopyInformation(m_eigenAnalysisFilter->GetOutput());
		output->SetBufferedRegion(m_eigenAnalysisFilter->GetOutput()->GetBufferedRegion());
		output->Allocate();
	}
	
	/**
	 * 
	 */
	template <typename TInputImage, typename TOutputImage >
	void OrientedFluxCrossSectionTraceMeasureFilter<TInputImage,TOutputImage >
	::ThreadedGenerateData( const OutputImageRegionType& outputRegionForThread,
												 ThreadIdType threadId)
	{
		
		// support progress methods/callbacks
		ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());
		
		
		ImageRegionIterator<OutputImageType>				 outputIt;
		typedef ImageRegionConstIterator< EigenValueImageType> EigenValueIteratorType;
		EigenValueIteratorType eigenIt;
		
		outputIt = ImageRegionIterator<OutputImageType>(this->GetOutput(), outputRegionForThread );
		eigenIt = EigenValueIteratorType( m_eigenAnalysisFilter->GetOutput(),
																		 outputRegionForThread );
		outputIt.GoToBegin();
		eigenIt.GoToBegin();
		while ( !outputIt.IsAtEnd() ) 
		{
			RealType value = 0.0;
			
			if (m_IsBright) 
			{
				for (unsigned int i = 0; i < ImageDimension-1; i++) 
				{
					value -= eigenIt.Get()[i];
				}
				if( value < 0) // masking black tubular structures
				{
					value = 0.0f;
				}
			}
			else
			{
				for (unsigned int i = 1; i < ImageDimension; i++) 
				{
					value += eigenIt.Get()[i];
				}

				if(value > 0) // masking white tubular structures
				{
					value = 0.0f;
				}
			}

			if( abs(value) < 1e-4 ) // cleaning noisy data noisy data ( the same is done by jerman)
			{
				outputIt.Set( 0.0f );
			}
			else
			{
				outputIt.Set( value / (2 * abs(m_maxImageEigenValue) ) );
			}
			
			
			++outputIt;
			++eigenIt;
			progress.CompletedPixel();
		}
		
		
	}
	
	
	template <typename TInputImage, typename TOutputImage>
	void
	OrientedFluxCrossSectionTraceMeasureFilter<TInputImage,TOutputImage>
	::PrintSelf(std::ostream& os, Indent indent) const
	{
		Superclass::PrintSelf(os,indent);
		os << indent << "is Bright: " << std::endl
		<< this->m_IsBright << std::endl;
	}
	
	
} // end namespace itk

#endif
