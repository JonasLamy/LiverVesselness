#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkBinaryThinningImageFilter3D.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkConvolutionImageFilter.h"
#include "itkFlatStructuringElement.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkMaskImageFilter.h"
#include "itkSignedMaurerDistanceMapImageFilter.h"

#include "itkEllipseSpatialObject.h"
#include "itkSpatialObjectToImageFilter.h"
#include "itkImageRegionConstIterator.h"
#include "itkMaximumImageFilter.h"

#include "utils.h"



#include <string>

using PixelType = unsigned char;
using ImageType = itk::Image<PixelType,3>;
using FloatImageType = itk::Image<float,3>;

void CreateKernel(ImageType::Pointer kernel, unsigned int width)
{
  ImageType::IndexType start;
  start.Fill(0);

  ImageType::SizeType size;
  size.Fill(width);

  ImageType::RegionType region;
  region.SetSize(size);
  region.SetIndex(start);

  kernel->SetRegions(region);
  kernel->Allocate();

  itk::ImageRegionIterator<ImageType> imageIterator(kernel, region);
  imageIterator.GoToBegin();
  while (!imageIterator.IsAtEnd())
  {
    // imageIterator.Set(255);
    imageIterator.Set(1);

    ++imageIterator;
  }
}

int main(int argc,char** argv)
 {

    std::string inputFileName(argv[1]);
    std::string maskFileName(argv[2]);
    std::string gtFileName(argv[3]);
    
    bool isDicom = false;
    

    bool skeleton = false;
    if(argc >= 5)
        skeleton = true;

    std::cout<<"-1"<<std::endl;
    // read input

    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(inputFileName);
    reader->Update();

    auto img = vUtils::readImage<ImageType>(inputFileName,isDicom);
    auto mask = vUtils::readImage<ImageType>(maskFileName,isDicom);

    std::cout<<"0"<<std::endl;

    auto rescaleFilter = itk::RescaleIntensityImageFilter<ImageType,ImageType>::New();
    rescaleFilter->SetOutputMaximum(1);
    rescaleFilter->SetOutputMinimum(0);
    rescaleFilter->SetInput(img);
    rescaleFilter->Update();

    std::cout<<"1"<<std::endl;

    auto thinningFilter = itk::BinaryThinningImageFilter3D<ImageType,ImageType>::New();
    thinningFilter->SetInput(rescaleFilter->GetOutput());
    thinningFilter->Update();

    

    auto maskFilter = itk::MaskImageFilter<ImageType,ImageType,ImageType>::New();
    maskFilter->SetInput( thinningFilter->GetOutput() );
    maskFilter->SetMaskImage( mask );
    maskFilter->SetOutsideValue(0);
    maskFilter->SetMaskingValue(0);
    maskFilter->Update();

    using WriterType = itk::ImageFileWriter<ImageType>;
    if(skeleton)
    {
        WriterType::Pointer thinningWriter = WriterType::New();

        thinningWriter->SetFileName("skeleton.nii");
        thinningWriter->SetInput(maskFilter->GetOutput());
        try
        {
            thinningWriter->Update();
        }
        catch (itk::ExceptionObject & excp)
        {
            std::cerr << excp << std::endl;
            return EXIT_FAILURE;
        }
    }

    std::cout<<"2"<<std::endl;


    // creating structuring element

    auto kernel = ImageType::New();
    CreateKernel(kernel,5);

    auto convFilter = itk::ConvolutionImageFilter<ImageType,ImageType>::New();
    convFilter->SetInput(maskFilter->GetOutput());
    convFilter->SetKernelImage(kernel);
    convFilter->Update();

    auto maskFilter2 = itk::MaskImageFilter<ImageType,ImageType,ImageType>::New();
    maskFilter2->SetInput(convFilter->GetOutput());
    maskFilter2->SetMaskImage(thinningFilter->GetOutput());
    

    std::cout<<"3"<<std::endl;

    auto thresholdFilter = itk::BinaryThresholdImageFilter<ImageType,ImageType>::New();
    thresholdFilter->SetLowerThreshold(7);
    thresholdFilter->SetUpperThreshold(255);
    thresholdFilter->SetInsideValue(255);
    thresholdFilter->SetOutsideValue(0);
    
    thresholdFilter->SetInput( maskFilter2->GetOutput() );
    thresholdFilter->Update();
    
    std::cout<<"4"<<std::endl;

    // create the gt

    // 1) inverse image
    // 2) distance transform
    // 3) mask filter with bifurcations
    // 4) draw balls from filter

    auto inverseFilter = itk::BinaryThresholdImageFilter<ImageType,ImageType>::New();
    inverseFilter->SetLowerThreshold(254);
    inverseFilter->SetUpperThreshold(254);
    inverseFilter->SetInsideValue(0);
    inverseFilter->SetOutsideValue(254);

    inverseFilter->SetInput( mask );
    
    auto distanceTransform = itk::SignedMaurerDistanceMapImageFilter<ImageType,FloatImageType>::New();
    distanceTransform->SquaredDistanceOff();
    //distanceTransform->SquaredDistanceOn();
    distanceTransform->SetInput( inverseFilter->GetOutput() );

    auto maskDistanceFilter = itk::MaskImageFilter<FloatImageType,ImageType>::New();
    maskDistanceFilter->SetInput( distanceTransform->GetOutput() );
    maskDistanceFilter->SetMaskImage( thresholdFilter->GetOutput() );
    maskDistanceFilter->Update();

    auto imgBifurcationNode = maskDistanceFilter->GetOutput();


    // creating image
    auto resultImage = ImageType::New();
    resultImage->SetRegions( img->GetLargestPossibleRegion() );
    
    resultImage->SetSpacing(img->GetSpacing() );
    resultImage->Allocate();
    resultImage->FillBuffer(0);
    std::cout<<"result image spacing \n"<<resultImage->GetSpacing() <<std::endl;
    // creating ellipses

    using EllipseType = itk::EllipseSpatialObject<3>;
    using SpacialObjectToImageFilterType = itk::SpatialObjectToImageFilter<EllipseType,ImageType>;
    int radius;

    itk::ImageRegionConstIterator<FloatImageType> it( imgBifurcationNode, imgBifurcationNode->GetLargestPossibleRegion() );
    it.GoToBegin();
    while( !it.IsAtEnd() )
    {
        if(it.Value() == 0)
        {
            ++it;
            continue;
        }

        auto ellipse = EllipseType::New();
        EllipseType::ArrayType radiusArray;
        int radius = static_cast<int>( std::sqrt( it.Value() ) ) | 1 ;
        radiusArray[0] = radius;
        radiusArray[1] = radius;
        radiusArray[2] = radius;
        
        std::cout<<radius<<std::endl;

        ImageType::RegionType::SizeType imSize;

        imSize[0] = radius*2 +1 ;
        imSize[1] = radius*2 +1 ;
        imSize[2] = radius*2 +1 ;

        ImageType::SpacingType spacing;
        spacing[0] = 1;
        spacing[1] = 1;
        spacing[2] = 1;

        ImageType::PointType origin;
        origin[0] = 0;//12.5;
        origin[1] = 0;//12.5;
        origin[2] = 0;//12.5;

        auto ellipseToImageFilter = SpacialObjectToImageFilterType::New();
        ellipseToImageFilter->SetSize( imSize );
        ellipseToImageFilter->SetSpacing( spacing );
        ellipseToImageFilter->SetOrigin( origin );

        ellipse->SetRadiusInObjectSpace(radiusArray);

        std::cout<<"ellipse filter spacing \n"<<imgBifurcationNode->GetSpacing() <<std::endl;

        ellipse->SetRadiusInObjectSpace(radiusArray);
        
        ImageType::IndexType bifurcationIndex;
        bifurcationIndex = it.GetIndex();

        std::cout<<"index:"<<bifurcationIndex<<" radius: "<<radius<<std::endl;

        ellipseToImageFilter->SetInput(ellipse);
        ellipse->SetDefaultInsideValue(255);
        ellipse->SetDefaultOutsideValue(0);
        ellipseToImageFilter->SetUseObjectValue(true);
        ellipseToImageFilter->SetOutsideValue(0);
        ellipseToImageFilter->Update();

        // iterate on ball and translate to real image
        auto ball = ellipseToImageFilter->GetOutput();
        itk::ImageRegionConstIterator<ImageType> itBall(ball,ball->GetLargestPossibleRegion());
        while( !itBall.IsAtEnd() )
        {
            // get iterator index
            auto ballIndex = itBall.GetIndex();
            // translate indexes
            ballIndex[0] += bifurcationIndex[0] - radius / 2;
            ballIndex[1] += bifurcationIndex[1] - radius / 2;
            ballIndex[2] += bifurcationIndex[2] - radius / 2;
            //write value
            resultImage->SetPixel(ballIndex, itBall.Value() );  
            
            ++itBall;
        }

        ++it;
    }

    resultImage->SetOrigin( img->GetOrigin() );


    std::cout<<"5"<<std::endl;
    using OutputWriterType = itk::ImageFileWriter<ImageType>;
    auto writer = OutputWriterType::New();

    writer->SetFileName(gtFileName);
    writer->SetInput( resultImage );
    try
    {
        writer->Update();
    }
    catch (itk::ExceptionObject & excp)
    {
        std::cerr << excp << std::endl;
        return EXIT_FAILURE;
    }

    return 0;
}