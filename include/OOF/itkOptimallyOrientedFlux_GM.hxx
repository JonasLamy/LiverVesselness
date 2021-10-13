
template<typename TInputImage, typename TOutputImage>
itk::OptimallyOrientedFlux_GM<TInputImage,TOutputImage>::OptimallyOrientedFlux_GM()
:m_sigma(1.0),m_normalizationType(1),m_responseType(1)
{
}
/*
template<typename TInputImage, typename TOutputImage>
itk::OptimallyOrientedFlux_GM<TInputImage,TOutputImage>::~OptimallyOrientedFlux_GM()
{


}
*/
template<typename TInputImage, typename TOutputImage>
void itk::OptimallyOrientedFlux_GM<TInputImage,TOutputImage>::PrintSelf(std::ostream & os,Indent indent) const
{

}
template <typename TInputImage, typename TOutputImage>
void itk::OptimallyOrientedFlux_GM<TInputImage,TOutputImage>::SetRadii(const std::vector<double> &v){
  m_Radii = v;
}


template <typename TInputImage, typename TOutputImage>
itk::Image<double,3>::Pointer itk::OptimallyOrientedFlux_GM<TInputImage,TOutputImage>::ifftShiftedCoordMatrixX(typename TInputImage::SizeType dimension,typename TInputImage::SpacingType spacing, typename TInputImage::PointType origin)
{
    CoordImageType::SizeType p;
    p[0] = std::floor(dimension[0]/2.0);
    p[1] = std::floor(dimension[1]/2.0);
    p[2] = std::floor(dimension[2]/2.0);

    CoordImageType::SizeType size;
    size[0] = dimension[0];
    size[1] = dimension[1];
    size[2] = dimension[2];

    CoordImageType::IndexType index = {0,0,0};

    CoordImageType::RegionType region;
    region.SetSize(size);
    region.SetIndex(index);

    auto X = CoordImageType::New();
    X->SetOrigin(origin);
    X->SetRegions(region);
    X->SetSpacing(spacing);
    X->Allocate();
    X->FillBuffer(0.0);

    CoordImageType::IndexType pixelIndexX;

    double value = 0;

    // do it for X
    for(int row=0; row<size[0];row++)
        for(int depth=0;depth<size[2];depth++)
        {
            for(int col=0;col<p[1];col++)
            {
                pixelIndexX[0] = row;
                pixelIndexX[1] = col;
                pixelIndexX[2] = depth;
                //computing value for the slice;
                value = col / static_cast<double>(size[1]) / static_cast<double>(spacing[1]);
                X->SetPixel(pixelIndexX, value );
            }
            for(int col=p[1];col<size[1];col++)
            {
                pixelIndexX[0] = row;
                pixelIndexX[1] = col;
                pixelIndexX[2] = depth;
                //computing value for the slice;
                value = ( col - static_cast<int>( p[1] )*2 ) / static_cast<double>(size[1]) / static_cast<double>(spacing[1]);
                X->SetPixel(pixelIndexX, value );
            }
        }
    return X;
}


template <typename TInputImage, typename TOutputImage>
itk::Image<double,3>::Pointer itk::OptimallyOrientedFlux_GM<TInputImage,TOutputImage>::ifftShiftedCoordMatrixY(typename TInputImage::SizeType dimension,typename TInputImage::SpacingType spacing,typename TInputImage::PointType origin)
{    
    CoordImageType::SizeType p;
    p[0] = std::floor(dimension[0]/2.0);
    p[1] = std::floor(dimension[1]/2.0);
    p[2] = std::floor(dimension[2]/2.0);

    CoordImageType::SizeType size;
    size[0] = dimension[0];
    size[1] = dimension[1];
    size[2] = dimension[2];

    CoordImageType::IndexType index = {0,0,0};

    CoordImageType::RegionType region;
    region.SetSize(size);
    region.SetIndex(index);

    auto Y = CoordImageType::New();
    Y->SetOrigin(origin);
    Y->SetRegions(region);
    Y->SetSpacing(spacing);
    Y->Allocate();
    Y->FillBuffer(0.0);

    CoordImageType::IndexType pixelIndexY;

    double value = 0;
        // do it for Y
    for(int col=0; col<size[1];col++)
    {
        for(int depth=0;depth<size[2];depth++)
        {
            for(int row=0;row<p[0];row++)
            {
                pixelIndexY[0] = row;
                pixelIndexY[1] = col;
                pixelIndexY[2] = depth;
                //computing value for the slice;
                //varying from 0 to p[0]
                value = (row ) / static_cast<double>(size[0]) / static_cast<double>(spacing[0]);
                Y->SetPixel(pixelIndexY, value );
            }
            
            for(int row=p[0];row<size[0];row++)
            {
                pixelIndexY[0] = row;
                pixelIndexY[1] = col;
                pixelIndexY[2] = depth;
                //computing value for the slice;
                //varying from -p to -1 
                value = ( row - static_cast<int>( p[0] )*2 )  / static_cast<double>(size[0]) / static_cast<double>(spacing[0]);
                Y->SetPixel(pixelIndexY, value );
            }
            
        }
    }
    return Y;
}

template <typename TInputImage, typename TOutputImage>
itk::Image<double,3>::Pointer itk::OptimallyOrientedFlux_GM<TInputImage,TOutputImage>::ifftShiftedCoordMatrixZ(typename TInputImage::SizeType dimension,typename TInputImage::SpacingType spacing, typename TInputImage::PointType origin)
{
    CoordImageType::SizeType p;
    p[0] = std::floor(dimension[0]/2.0);
    p[1] = std::floor(dimension[1]/2.0);
    p[2] = std::floor(dimension[2]/2.0);

    CoordImageType::SizeType size;
    size[0] = dimension[0];
    size[1] = dimension[1];
    size[2] = dimension[2];

    CoordImageType::IndexType index = {0,0,0};

    CoordImageType::RegionType region;
    region.SetSize(size);
    region.SetIndex(index);

    auto Z = CoordImageType::New();
    Z->SetOrigin(origin);
    Z->SetRegions(region);
    Z->SetSpacing(spacing);
    Z->Allocate();
    Z->FillBuffer(0.0);

    CoordImageType::IndexType pixelIndexZ;

    double value = 0;

    // TODO take spacing into account

    // do it for Z
    for(int row=0; row<size[0];row++)
        for(int col=0;col<size[1];col++)
        {
            for(int depth=0;depth<p[2];depth++)
            {
                pixelIndexZ[0] = row;
                pixelIndexZ[1] = col;
                pixelIndexZ[2] = depth;
                //computing value for the slice;
                value = depth / static_cast<double>(size[2]) / static_cast<double>(spacing[2]);
                Z->SetPixel(pixelIndexZ, value );
            }
            for(int depth=p[2];depth<size[2];depth++)
            {
                pixelIndexZ[0] = row;
                pixelIndexZ[1] = col;
                pixelIndexZ[2] = depth;
                //computing value for the slice;
                value = ( depth - static_cast<int>( p[2]*2 ) ) / static_cast<double>(size[2]) / static_cast<double>(spacing[2]);
                Z->SetPixel(pixelIndexZ, value );
            }
        }
    return Z;
}

template<typename TInputImage, typename TOutputImage>
itk::Image<double,3>::Pointer itk::OptimallyOrientedFlux_GM<TInputImage,TOutputImage>::ifftshiftedcoordinate(typename TInputImage::SizeType dimension,int index,typename TInputImage::SpacingType spacing, typename TInputImage::PointType origin)
{
    CoordImageType::Pointer img;

    switch(index)
    {
        case 0:
        img = ifftShiftedCoordMatrixY(dimension,spacing,origin);
        break;
        case 1: // Z is second so that itk/matlab correspondance is the same
        img = ifftShiftedCoordMatrixX(dimension,spacing,origin);
        break;
        case 2:
        img = ifftShiftedCoordMatrixZ(dimension,spacing,origin);
        break;
        default:
            throw "dimensions error";
    }
    

    return img;
}

template <typename TInputImage, typename TOutputImage>
std::vector<itk::Image<double,3>::Pointer> itk::OptimallyOrientedFlux_GM<TInputImage,TOutputImage>::ifftShiftedCoordMatrix(typename TInputImage::SizeType dimension,typename TInputImage::SpacingType spacing,typename TInputImage::PointType origin)
{

    CoordImageType::Pointer X = ifftShiftedCoordMatrixX(dimension,spacing,origin);
    CoordImageType::Pointer Y = ifftShiftedCoordMatrixY(dimension,spacing,origin);
    CoordImageType::Pointer Z = ifftShiftedCoordMatrixZ(dimension,spacing,origin);
    
    std::vector<CoordImageType::Pointer> vect;
    vect.push_back(X);
    vect.push_back(Y);
    vect.push_back(Z);

    return vect;
}


template<typename TInputImage, typename TOutputImage>
void itk::OptimallyOrientedFlux_GM<TInputImage,TOutputImage>::GenerateData() 
{
    typename ImageType::Pointer input = ImageType::New();
    input->Graft(const_cast<ImageType *>(this->GetInput()));

   using FFTType = typename itk::FFTWForwardFFTImageFilter<TInputImage>;
    auto FFTfilter = FFTType::New();
    FFTfilter->SetInput(input);

    using FFTOutputImageType = typename FFTType::OutputImageType;
    using FFTOutputPixelType = typename FFTType::OutputPixelType;
    using FloatImageType = itk::Image<double,3>;

    // extracting fft coordinates
    auto vector = ifftShiftedCoordMatrix(input->GetLargestPossibleRegion().GetSize(),input->GetSpacing(),input->GetOrigin() );

    auto radius = CoordImageType::New();
    radius->SetOrigin( vector[0]->GetOrigin() );
    radius->SetRegions(vector[0]->GetLargestPossibleRegion() );
    radius->SetSpacing(vector[0]->GetSpacing());
    radius->Allocate();

    SizeType sizeInput = input->GetLargestPossibleRegion().GetSize();
    CoordImageType::IndexType index;
    double value;
    for(int i=0; i<sizeInput[0];i++)
        for(int j=0; j<sizeInput[1];j++)
            for(int k=0; k<sizeInput[2]; k++)
            {
                index[0] = i;
                index[1] = j;
                index[2] = k;

                double x_carre = vector[0]->GetPixel(index) * vector[0]->GetPixel(index); 
                double y_carre = vector[1]->GetPixel(index) * vector[1]->GetPixel(index); 
                double z_carre = vector[2]->GetPixel(index) * vector[2]->GetPixel(index); 
                value = sqrt( x_carre + y_carre + z_carre ) + 1e-12;
                radius->SetPixel(index,value);
            }   
    // erasing vector of X,Y,Z for low memory consuptiom
    vector.clear();

    using BesselBufferImageType = itk::Image<FFTOutputPixelType,3>;
    using ComplexMultiplyImageFilterType = itk::MultiplyImageFilter<BesselBufferImageType,BesselBufferImageType>;

    auto outputImage = FloatImageType::New();
    outputImage->SetOrigin(input->GetOrigin() );
    outputImage->SetSpacing( input->GetSpacing() );
    outputImage->SetRegions(input->GetLargestPossibleRegion() );
    outputImage->Allocate();
    outputImage->FillBuffer(0);

    // creating radius image 
    double normalization = 0;
    for(auto &rad : m_Radii)
    {
        std::cout<<"radius:"<<rad<<std::endl;

        /* TODO: find out why 1e-12^(3/2) needs to be turned into 1e12^(3/2) to match Matlab implementation */ 
        normalization = 4.0/3.0*M_PI*(rad*rad*rad)/boost::math::cyl_bessel_j<double,double>(1.5,2*M_PI*rad*1e-12)/std::pow(1e12,1.5)
        / (rad*rad) * ( std::pow(rad / sqrt( 2*rad*m_sigma - m_sigma*m_sigma ),m_normalizationType ) );

        auto BesselJBuffer = BesselBufferImageType::New();
        BesselJBuffer->SetOrigin(input->GetOrigin() );
        BesselJBuffer->SetRegions( input->GetLargestPossibleRegion() );
        BesselJBuffer->SetSpacing( input->GetSpacing() );
        BesselJBuffer->Allocate();

        itk::ImageRegionIterator<BesselBufferImageType> itBesselJBuffer(BesselJBuffer,BesselJBuffer->GetLargestPossibleRegion());
        itk::ImageRegionIterator<CoordImageType> itRadius(radius,radius->GetLargestPossibleRegion());

        itBesselJBuffer.GoToBegin();
        itRadius.GoToBegin();

        double besselValue = 0.0;
        while( !itBesselJBuffer.IsAtEnd() )
        {
            besselValue =  normalization * exp( (-m_sigma*m_sigma)*2*M_PI*M_PI *(itRadius.Get()*itRadius.Get()) )/ std::pow(itRadius.Get(),1.5);
            itBesselJBuffer.Value() = std::complex<double>( sin(2*M_PI*rad*itRadius.Get() ) / ( 2*M_PI*rad*itRadius.Get() ) - cos(2*M_PI * rad* itRadius.Get() ) )
                                        * std::complex<double>( besselValue * std::sqrt(1.0/M_PI/M_PI/rad/itRadius.Get() ) );
            ++itBesselJBuffer;
            ++itRadius;
        }

        auto multiplyImageFilter = ComplexMultiplyImageFilterType::New();
        multiplyImageFilter->SetInput1(BesselJBuffer);
        multiplyImageFilter->SetInput2(FFTfilter->GetOutput());
        multiplyImageFilter->Update();

        BesselJBuffer = multiplyImageFilter->GetOutput();
        

        std::vector<FloatImageType::Pointer> outputFeatures;
        FloatImageType::IndexType index;
        index[0] = 10;
        index[1] = 10;
        index[2] = 10;

        for(int i=0;i<3;i++)
            for(int j=i;j<3;j++)
            {
                // computing all 6 ifft for each radius
                using InverseFFTFilter = itk::InverseFFTImageFilter<FFTOutputImageType,FloatImageType>;

                auto multiplyShiftedCoordinates = itk::MultiplyImageFilter<CoordImageType,CoordImageType>::New();
                auto multiplyWithBesselBuffer = itk::MultiplyImageFilter<CoordImageType,BesselBufferImageType,BesselBufferImageType>::New();
                auto inverseFilter = InverseFFTFilter::New();

                multiplyShiftedCoordinates->SetInput1( ifftshiftedcoordinate( sizeInput,i,BesselJBuffer->GetSpacing(), BesselJBuffer->GetOrigin() ) );
                multiplyShiftedCoordinates->SetInput2( ifftshiftedcoordinate( sizeInput,j,BesselJBuffer->GetSpacing(), BesselJBuffer->GetOrigin() ) );

                multiplyWithBesselBuffer->SetInput1( multiplyShiftedCoordinates->GetOutput() );
                multiplyWithBesselBuffer->SetInput2( BesselJBuffer );
                
                inverseFilter->SetInput( multiplyWithBesselBuffer->GetOutput() );  
                inverseFilter->Update();

                auto feature = inverseFilter->GetOutput();
                // triple check at this points that values are the same as matlab (rounding error apearing at e10-8)
                auto iv = inverseFilter->GetOutput();
                
                /*
                for(int depth=0;depth<5;depth++)
                {   
                    for(int row=0; row<5;row++) 
                    {
                        for(int col=0;col<5;col++)
                        {
                            index[0] = row;
                            index[1] = col;
                            index[2] = depth;
                            std::cout<<iv->GetPixel(index)<<" ";
                        }
                        std::cout<<std::endl;
                    }
                    std::cout<<std::endl;
                }
                
                std::cout<<std::endl;
                */
                outputFeatures.push_back(feature);
            }
        std::cout<<"computed ifft for all 6 bandwidth"<<std::endl;

        FloatImageType::RegionType r;
        FloatImageType::RegionType::SizeType si;
        si[0] = 5;
        si[1] = 5;
        si[2] = 5;
        r.SetSize(si);
        index[0] = 0;
        index[1] = 0;
        index[2] = 0;
        r.SetIndex(index);
        
        using HessianPixelType = itk::SymmetricSecondRankTensor< double, 3 >;
        HessianPixelType hessianMat;

        using EigenValueArrayType = itk::FixedArray<double,3>;


        using CalculatorType =  SymmetricEigenAnalysisFixedDimension<3, HessianPixelType, EigenValueArrayType>;
        CalculatorType eigenCalculator;
        eigenCalculator.SetOrderEigenMagnitudes(true);
        EigenValueArrayType eigenValues;
        int count =0;

        std::cout<<"computing vesselness"<<std::endl;
        // looping
        for(int depth=0;depth<sizeInput[2];depth++)
        {   
            for(int row=0; row<sizeInput[0];row++) 
            {
                for(int col=0;col<sizeInput[1];col++)
                {
                    index[0] = row;
                    index[1] = col;
                    index[2] = depth;
                    //std::cout<<index<<std::endl;
    
                    // computing eigen values and metrics
                    hessianMat[0] = outputFeatures[0]->GetPixel(index); // feature_11
                    hessianMat[1] = outputFeatures[1]->GetPixel(index); // feature_12
                    hessianMat[2] = outputFeatures[2]->GetPixel(index); // feature_13

                    hessianMat[3] = outputFeatures[3]->GetPixel(index); // feature_22
                    hessianMat[4] = outputFeatures[4]->GetPixel(index); // feature_23
                    hessianMat[5] = outputFeatures[5]->GetPixel(index); // feature_33

                    eigenCalculator.ComputeEigenValues( hessianMat,eigenValues );

                    typename TOutputImage::PixelType vesselness;

                    // TODO : find out why there is an inversion of eigen values sign. 
                    // This doesn't change the quality of the enhancement, but we have to watch for eigen values order...
                    if( eigenValues[1] >= 0 && eigenValues[2] >= 0) 
                    {
                        vesselness = std::sqrt( std::abs( eigenValues[1] * eigenValues[2] ) );
                    }else
                    {
                        vesselness = 0;
                    }
                     
                    if( vesselness > outputImage->GetPixel(index) ) // use maximum response of vesselness
                        outputImage->SetPixel(index, vesselness ); 
                }
            }
        }
        std::cout<<"done"<<std::endl;
    }
    this->GraftOutput(outputImage);
}
