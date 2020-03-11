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
//#include "QuickView.h"



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
    int boxSize = std::atoi(argv[4]);

    bool skeleton = false;
    if(argc >= 5)
        skeleton = true;

    std::cout<<"-1"<<std::endl;
    // read input

    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(inputFileName);
    reader->Update();
    auto img = reader->GetOutput();

    auto maskReader = itk::ImageFileReader<ImageType>::New();
    maskReader->SetFileName(maskFileName);
    maskReader->Update();

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

    using WriterType = itk::ImageFileWriter<ImageType>;
    if(skeleton)
    {
        WriterType::Pointer thinningWriter = WriterType::New();

        thinningWriter->SetFileName("skeleton.nii");
        thinningWriter->SetInput(thinningFilter->GetOutput());
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

    auto maskFilter = itk::MaskImageFilter<ImageType,ImageType,ImageType>::New();
    maskFilter->SetInput( thinningFilter->GetOutput() );
    maskFilter->SetMaskImage( maskReader->GetOutput() );
    maskFilter->SetOutsideValue(0);
    maskFilter->Update();

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
    inverseFilter->SetLowerThreshold(255);
    inverseFilter->SetUpperThreshold(255);
    inverseFilter->SetInsideValue(0);
    inverseFilter->SetOutsideValue(255);

    inverseFilter->SetInput( maskReader->GetOutput() );
    
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
    resultImage->SetRegions( imgBifurcationNode->GetLargestPossibleRegion() );
    resultImage->Allocate();
    resultImage->FillBuffer(0);

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

        auto ellipseToImageFilter = SpacialObjectToImageFilterType::New();
        ellipseToImageFilter->SetSize( imgBifurcationNode->GetLargestPossibleRegion().GetSize() );
        ellipseToImageFilter->SetSpacing( imgBifurcationNode->GetSpacing() );

        auto ellipse = EllipseType::New();
        EllipseType::ArrayType radiusArray;
        int radius = static_cast<int>( std::sqrt( it.Value() ) ) | 1 ;
        radiusArray[0] = radius;
        radiusArray[1] = radius;
        radiusArray[2] = radius;

        ellipse->SetRadiusInObjectSpace(radiusArray);
        // move the ellipse
        auto transform = EllipseType::TransformType::New();
        transform->SetIdentity();
        EllipseType::TransformType::OutputVectorType translation;
        ImageType::IndexType index;
        index = it.GetIndex();

        std::cout<<"index:"<<index<<" radius: "<<radius<<std::endl;

        translation[0] = index[0];
        translation[1] = index[1];
        translation[2] = index[2];
        transform->Translate(translation);

        ellipse->SetObjectToParentTransform(transform);

        ellipseToImageFilter->SetInput(ellipse);
        ellipse->SetDefaultInsideValue(radius);
        ellipse->SetDefaultOutsideValue(0);
        ellipseToImageFilter->SetUseObjectValue(true);
        ellipseToImageFilter->SetOutsideValue(0);
        ellipseToImageFilter->Update();

        auto maxFilter = itk::MaximumImageFilter<ImageType,ImageType>::New();
        maxFilter->SetInput(0,resultImage);
        maxFilter->SetInput(1,ellipseToImageFilter->GetOutput());
        maxFilter->Update();
        resultImage = maxFilter->GetOutput();

        ++it;
    }

    resultImage->SetOrigin( imgBifurcationNode->GetOrigin() );
    resultImage->SetSpacing( imgBifurcationNode->GetSpacing() );

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