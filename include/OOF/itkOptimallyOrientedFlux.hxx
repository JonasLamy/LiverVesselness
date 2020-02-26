template<typename TInputImage, typename TOutputImage>
itk::OptimallyOrientedFlux<TInputImage,TOutputImage>::OptimallyOrientedFlux()
:m_sigma(1.0),m_normalizationType(1),m_responseType(1)
{
    m_radii.push_back(3);
}
/*
template<typename TInputImage, typename TOutputImage>
itk::OptimallyOrientedFlux<TInputImage,TOutputImage>::~OptimallyOrientedFlux()
{


}
*/
template<typename TInputImage, typename TOutputImage>
void itk::OptimallyOrientedFlux<TInputImage,TOutputImage>::PrintSelf(std::ostream & os,Indent indent) const
{

}

template <typename TInputImage, typename TOutputImage>
std::vector<itk::Image<float,3>::Pointer> itk::OptimallyOrientedFlux<TInputImage,TOutputImage>::ifftShiftedCoordMatrix(typename TInputImage::SizeType dimension,typename TInputImage::SpacingType spacing)
{
    int dim = 3;
    
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
    auto Y = CoordImageType::New();
    auto Z = CoordImageType::New();

    X->SetRegions(region);
    Y->SetRegions(region);
    Z->SetRegions(region);

    X->Allocate();
    X->FillBuffer(0.0);
    Y->Allocate();
    Y->FillBuffer(0.0);
    Z->Allocate();
    Z->FillBuffer(0.0);

    CoordImageType::IndexType pixelIndexX;
    CoordImageType::IndexType pixelIndexY;
    CoordImageType::IndexType pixelIndexZ;

    std::vector<int> a;
    float value = 0;

    // TODO take spacing into account

    std::cout<<"computing coord image"<<std::endl;
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
                value = depth / static_cast<float>(size[2]) / static_cast<float>(spacing[2]);
                Z->SetPixel(pixelIndexZ, value );
            }
            for(int depth=p[2];depth<size[2];depth++)
            {
                pixelIndexZ[0] = row;
                pixelIndexZ[1] = col;
                pixelIndexZ[2] = depth;
                //computing value for the slice;
                value = ( depth - static_cast<int>( p[2]*2 ) ) / static_cast<float>(size[2]) / static_cast<float>(spacing[2]);
                Z->SetPixel(pixelIndexZ, value );
            }
        }

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
                value = col / static_cast<float>(size[1]) / static_cast<float>(spacing[1]);
                X->SetPixel(pixelIndexX, value );
            }
            for(int col=p[1];col<size[1];col++)
            {
                pixelIndexX[0] = row;
                pixelIndexX[1] = col;
                pixelIndexX[2] = depth;
                //computing value for the slice;
                value = ( col - static_cast<int>( p[1] )*2 ) / static_cast<float>(size[1]) / static_cast<float>(spacing[1]);
                X->SetPixel(pixelIndexX, value );
            }
        }
    
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
                value = (row ) / static_cast<float>(size[0]) / static_cast<float>(spacing[0]);
                Y->SetPixel(pixelIndexY, value );
            }
            
            for(int row=p[0];row<size[0];row++)
            {
                pixelIndexY[0] = row;
                pixelIndexY[1] = col;
                pixelIndexY[2] = depth;
                //computing value for the slice;
                //varying from -p to -1 
                value = ( row - static_cast<int>( p[0] )*2 )  / static_cast<float>(size[0]) / static_cast<float>(spacing[0]);
                Y->SetPixel(pixelIndexY, value );
            }
            
        }
    }

    std::vector<CoordImageType::Pointer> vect;
    vect.push_back(X);
    vect.push_back(Y);
    vect.push_back(Z);

    return vect;
}


template<typename TInputImage, typename TOutputImage>
void itk::OptimallyOrientedFlux<TInputImage,TOutputImage>::GenerateData() 
{
    typename ImageType::Pointer input = ImageType::New();
    input->Graft(const_cast<ImageType *>(this->GetInput()));

    /*
    Make magic happens here
    */ 

   // making fft
   using FFTType = itk::FFTWForwardFFTImageFilter<TInputImage>;
    auto FFTfilter = FFTType::New();
    FFTfilter->SetInput(input);

    using FFTOutputImageType = typename FFTType::OutputImageType;
    using FFTOutputPixelType = typename FFTType::OutputPixelType;
    using FloatImageType = itk::Image<float,3>;
    
     // Extract the real part
    using RealFilterType = itk::ComplexToRealImageFilter<FFTOutputImageType, FloatImageType>;
    typename RealFilterType::Pointer realFilter = RealFilterType::New();
    realFilter->SetInput(FFTfilter->GetOutput());
    realFilter->Update();

    // Extract the imaginary part
    using ImaginaryFilterType = itk::ComplexToImaginaryImageFilter<FFTOutputImageType, FloatImageType>;
    typename ImaginaryFilterType::Pointer imaginaryFilter = ImaginaryFilterType::New();
    imaginaryFilter->SetInput(FFTfilter->GetOutput());
    imaginaryFilter->Update();


    // extracting fft coordinates
    auto vector = ifftShiftedCoordMatrix(input->GetLargestPossibleRegion().GetSize(),input->GetSpacing());
    std::cout<<"finishing shift"<<std::endl;

    auto radius = CoordImageType::New();
    radius->SetRegions(vector[0]->GetLargestPossibleRegion() );
    radius->SetSpacing(vector[0]->GetSpacing());
    radius->Allocate();

    SizeType size = input->GetLargestPossibleRegion().GetSize();
    CoordImageType::IndexType index;
    float value;
    for(int i=0; i<size[0];i++)
        for(int j=0; j<size[1];j++)
            for(int k=0; k<size[2]; k++)
            {
                index[0] = i;
                index[1] = j;
                index[2] = k;

                float x_carre = vector[0]->GetPixel(index) * vector[0]->GetPixel(index); 
                float y_carre = vector[1]->GetPixel(index) * vector[1]->GetPixel(index); 
                float z_carre = vector[2]->GetPixel(index) * vector[2]->GetPixel(index); 
                value = sqrt( x_carre + y_carre + z_carre ) + 1e-12;
                radius->SetPixel(index,value);
            }   
    // erasing vector of X,Y,Z for low memory consuptiom
    //vector.clear();

    using BesselBufferImageType = itk::Image<FFTOutputPixelType,3>;
    using ComplexMultiplyImageFilterType = itk::MultiplyImageFilter<BesselBufferImageType,BesselBufferImageType>;
    // creating radius image 
    float normalization = 0;
    for(auto &rad : m_radii)
    {
        /* TODO: find out why 1e-12^(3/2) needs to be turned into 1e12^(3/2) to match Matlab implementation */ 
        normalization = 4.0/3.0*M_PI*(rad*rad*rad)/boost::math::cyl_bessel_j<float,float>(1.5,2*M_PI*rad*1e-12)/std::pow(1e12,1.5)
        / (rad*rad) * ( std::pow(rad / sqrt( 2*rad*m_sigma - m_sigma*m_sigma ),m_normalizationType ) );
        std::cout<<normalization<<std::endl;

        auto BesselJBuffer = BesselBufferImageType::New();
        BesselJBuffer->SetRegions( input->GetLargestPossibleRegion() );
        BesselJBuffer->SetSpacing( input->GetSpacing() );
        BesselJBuffer->Allocate();

        itk::ImageRegionIterator<BesselBufferImageType> itBesselJBuffer(BesselJBuffer,BesselJBuffer->GetLargestPossibleRegion());
        itk::ImageRegionIterator<CoordImageType> itRadius(radius,radius->GetLargestPossibleRegion());

        itBesselJBuffer.GoToBegin();
        itRadius.GoToBegin();
        while( !itBesselJBuffer.IsAtEnd() )
        {
            // radius^2 ./ radius^1.5 <-> (radius * radius) / (radius* sqrt(radius)) <-> sqrt(radius) 
            itBesselJBuffer.Value() =  normalization * exp( (-m_sigma*m_sigma)*2*M_PI*M_PI* sqrt(itRadius.Get()) );
            //std::cout<<itBesselJBuffer.Get()<<std::endl;
            itBesselJBuffer.Value() = std::complex<float>( sin(2*M_PI*rad*itRadius.Get()) / (2*M_PI*rad*itRadius.Get()) - cos(2*M_PI*rad*itRadius.Get()) ) 
                                    * itBesselJBuffer.Get()* std::complex<float>( (1/M_PI/M_PI/rad/itRadius.Get() ) );
            ++itBesselJBuffer;
            ++itRadius;
        }

        auto multiplyImageFilter = ComplexMultiplyImageFilterType::New();
        multiplyImageFilter->SetInput1(BesselJBuffer);
        multiplyImageFilter->SetInput2(FFTfilter->GetOutput());
        multiplyImageFilter->Update();
        
        for(int row=0; row<1;row++)
        {   
            for(int col=0;col<size[1];col++)
            {
                for(int depth=0;depth<size[2];depth++)
                {
                    index[0] = row;
                    index[1] = col;
                    index[2] = depth;
                    std::cout<<radius->GetPixel(index)<<" ";
                }
            
                std::cout<<std::endl;
            }
            std::cout<<std::endl;
        }
        std::cout<<"\n";
    }

    this->GraftOutput(realFilter->GetOutput());
}