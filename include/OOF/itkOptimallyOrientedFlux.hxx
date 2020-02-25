template<typename TInputImage, typename TOutputImage>
itk::OptimallyOrientedFlux<TInputImage,TOutputImage>::OptimallyOrientedFlux()
{


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
                value = ( depth - static_cast<int>( p[size[2]] ) ) / static_cast<float>(size[2]) / static_cast<float>(spacing[2]);
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
                value = depth / static_cast<float>(size[1]) / static_cast<float>(spacing[1]);
                X->SetPixel(pixelIndexX, value );
            }
            for(int col=p[1];col<size[1];col++)
            {
                pixelIndexX[0] = row;
                pixelIndexX[1] = col;
                pixelIndexX[2] = depth;
                //computing value for the slice;
                value = ( depth - static_cast<int>( p[size[1]] ) ) / static_cast<float>(size[1]) / static_cast<float>(spacing[1]);
                X->SetPixel(pixelIndexX, value );
            }
        }
    
    // do it for Y
    for(int col=0; col<size[1];col++)
        for(int depth=0;depth<size[2];depth++)
        {
            for(int row=0;row<p[0];row++)
            {
                pixelIndexY[0] = row;
                pixelIndexY[1] = col;
                pixelIndexY[2] = depth;
                //computing value for the slice;
                value = depth / static_cast<float>(size[0]) / static_cast<float>(spacing[0]);
                Y->SetPixel(pixelIndexY, value );
            }
            for(int row=p[0];row<size[0];row++)
            {
                pixelIndexY[0] = row;
                pixelIndexY[1] = col;
                pixelIndexY[2] = depth;
                //computing value for the slice;
                value = ( depth - static_cast<int>( p[size[0]] ) ) / static_cast<float>(size[0]) / static_cast<float>(spacing[0]);
                Y->SetPixel(pixelIndexY, value );
            }
        }

    for(int row=0; row<size[0];row++)
    {   
        for(int col=0;col<size[1];col++)
        {
            std::cout<<"X"<<std::endl;
            for(int depth=0;depth<size[2];depth++)
            {
                pixelIndexX[0] = row;
                pixelIndexX[1] = col;
                pixelIndexX[2] = depth;
                std::cout<<X->GetPixel(pixelIndexX)<<" ";
            }
            std::cout<<std::endl;
            
            std::cout<<"Y"<<std::endl;
            for(int depth=0;depth<size[2];depth++)
            {
                pixelIndexY[0] = row;
                pixelIndexY[1] = col;
                pixelIndexY[2] = depth;
                std::cout<<Y->GetPixel(pixelIndexY)<<" ";
            }
            std::cout<<std::endl;

            std::cout<<"Z"<<std::endl;
            for(int depth=0;depth<size[2];depth++)
            {
                pixelIndexZ[0] = row;
                pixelIndexZ[1] = col;
                pixelIndexZ[2] = depth;
                std::cout<<Z->GetPixel(pixelIndexZ)<<" ";
            }
            std::cout<<std::endl;
        }
        std::cout<<std::endl;
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
    typedef itk::ImageFileWriter<OptimallyOrientedFlux::CoordImageType> ScoreImageWriter;
    int i=0;
    for(auto &v : vector)
    {
        auto writer = ScoreImageWriter::New();
        writer->SetInput( v );
        std::string s = std::to_string(i) + std::string(".nii");
        std::cout<<"saving "<<s<<std::endl;
        writer->SetFileName( s );
        writer->Update();
        i++;
    }


    this->GraftOutput(realFilter->GetOutput());
}