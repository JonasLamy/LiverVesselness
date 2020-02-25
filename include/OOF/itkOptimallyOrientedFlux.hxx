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
void itk::OptimallyOrientedFlux<TInputImage,TOutputImage>::ifftShiftedCoordMatrix(typename TInputImage::SizeType dimension)
{
    int dim = 3;

    using CoordImageType = itk::Image<int,3>;
    
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
    X->FillBuffer(0);
    Y->Allocate();
    Y->FillBuffer(0);
    Z->Allocate();
    Z->FillBuffer(0);

    CoordImageType::IndexType pixelIndexX;
    CoordImageType::IndexType pixelIndexY;
    CoordImageType::IndexType pixelIndexZ;

    std::vector<int> a;
    int v = 0;

    std::cout<<"computing coord image"<<std::endl;
    // do it for X
    for(int row=0; row<size[0];row++)
        for(int col=0;col<size[1];col++)
        {
            int  g = 0;
            for(int depth=0;depth<p[2];depth++)
            {
                pixelIndexX[0] = row;
                pixelIndexX[1] = col;
                pixelIndexX[2] = depth;
                //computing value for the slice;
                X->SetPixel(pixelIndexX, depth);
                std::cout<<pixelIndexX<<std::endl;
                g++;
            }
            for(int depth=p[2];depth<size[2];depth++)
            {
                pixelIndexX[0] = row;
                pixelIndexX[1] = col;
                pixelIndexX[2] = depth;
                //computing value for the slice;
                X->SetPixel(pixelIndexX, depth-p[size[2]]);
                g++;
                std::cout<<pixelIndexX<<std::endl;
            }
        }

    for(int row=0; row<size[0];row++)
    {   
        for(int col=0;col<size[1];col++)
        {
            for(int depth=0;depth<size[2];depth++)
            {
                pixelIndexX[0] = row;
                pixelIndexX[1] = col;
                pixelIndexX[2] = depth;
                std::cout<<X->GetPixel(pixelIndexX)<<" ";
            }
            std::cout<<std::endl;
        }
        std::cout<<std::endl;
    }

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
    ifftShiftedCoordMatrix(input->GetLargestPossibleRegion().GetSize());

    this->GraftOutput(realFilter->GetOutput());
}