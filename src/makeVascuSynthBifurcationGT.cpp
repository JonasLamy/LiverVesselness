#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkBinaryThresholdImageFilter.h"
#include "itkBinaryThinningImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkConvolutionImageFilter.h"
#include "itkSignedMaurerDistanceMapImageFilter.h"
#include "itkMaximumImageFilter.h"

#include "itkEllipseSpatialObject.h"
#include "itkSpatialObjectToImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkImageDuplicator.h"
#include "itkConstantPadImageFilter.h"


//#include "QuickView.h"

#include <string>

using PixelType = unsigned char;
using GTPixelType = unsigned char;
using ImageType = itk::Image<PixelType,3>;
using GTImageType = itk::Image<PixelType,3>;
using DistanceImageType = itk::Image<float,3>;

int main(int argc,char** argv)
{
    std::string inputFileName(argv[1]);
    std::string bifurcationFileName(argv[2]);
    std::string gtFileName(argv[3]);

    // read input

    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(inputFileName);
    reader->Update();
    auto img = reader->GetOutput();

    // making empty image
    auto maskImg = ImageType::New();
    maskImg->SetRegions(img->GetLargestPossibleRegion() );
    maskImg->SetOrigin( img->GetOrigin() );
    maskImg->SetSpacing( img->GetSpacing() );
    maskImg->Allocate();
    maskImg->FillBuffer(0);


    // reading distance transform image

    auto inverseFilter = itk::BinaryThresholdImageFilter<ImageType,ImageType>::New();
    inverseFilter->SetLowerThreshold(255);
    inverseFilter->SetUpperThreshold(255);
    inverseFilter->SetInsideValue(0);
    inverseFilter->SetOutsideValue(255);

    inverseFilter->SetInput( img );

    auto distanceTransform = itk::SignedMaurerDistanceMapImageFilter<ImageType,DistanceImageType>::New();
    distanceTransform->SquaredDistanceOff();
    //distanceTransform->SquaredDistanceOn();
    distanceTransform->SetInput( inverseFilter->GetOutput() );
    distanceTransform->Update();
    
    // Reading bifurcation file
    std::ifstream f;
    f.open(bifurcationFileName);
    std::string s_x;
    std::string s_y;
    std::string s_z;

    float x,y,z;
    ImageType::PointType point;
    while( f.peek() != EOF)
    {
        std::cout<<"reading inputs"<<std::endl;
        std::getline( f,s_x,',' );
        std::getline( f,s_y,',' );
        std::getline( f,s_z,'\n' );

        std::cout<<"x:"<<s_x<<std::endl;
        std::cout<<"y:"<<s_y<<std::endl;
        std::cout<<"z:"<<s_z<<std::endl;
        std::cout<<"----"<<std::endl;
        x = std::stof(s_x);
        y = std::stof(s_y);
        z = std::stof(s_z);

        ImageType::PointType bifurcationCoordinatesFloat;
        bifurcationCoordinatesFloat[0] = x;
        bifurcationCoordinatesFloat[1] = y;
        bifurcationCoordinatesFloat[2] = z;

        ImageType::IndexType bifurcationCoordinates = maskImg->TransformPhysicalPointToIndex( bifurcationCoordinatesFloat );

        std::cout<<"bifurcationsCoordinates " << bifurcationCoordinates <<std::endl;
        // creating ellipses

        using EllipseType = itk::EllipseSpatialObject<3>;
        using SpacialObjectToImageFilterType = itk::SpatialObjectToImageFilter<EllipseType,ImageType>;

        int dist = distanceTransform->GetOutput()->GetPixel( bifurcationCoordinates );
        std::cout<<"distTransform :"<<dist<<std::endl;
        /*
        if(dist %2 == 0)
            if(dist != 0)
                dist--;
        */
        int radius = 1;//= dist; 
        //int radius = static_cast<int>( dist ) | 1 ;
        /*
        if( radius <= 1 )
            radius = 3;
        */
        auto ellipseToImageFilter = SpacialObjectToImageFilterType::New();
        ellipseToImageFilter->SetSize( maskImg->GetLargestPossibleRegion().GetSize() );
        ellipseToImageFilter->SetSpacing( maskImg->GetSpacing() );

        auto ellipse = EllipseType::New();
        EllipseType::ArrayType radiusArray;
        radiusArray[0] = radius;
        radiusArray[1] = radius;
        radiusArray[2] = radius;

        ellipse->SetRadiusInObjectSpace(radiusArray);
        // move the ellipse
        auto transform = EllipseType::TransformType::New();
        transform->SetIdentity();
        EllipseType::TransformType::OutputVectorType translation;
        ImageType::IndexType index;
        index = bifurcationCoordinates;

        std::cout<<"index:"<<index<<" radius: "<<radius<<std::endl;

        translation[0] = index[0];
        translation[1] = index[1];
        translation[2] = index[2];
        transform->Translate(translation);

        ellipse->SetObjectToParentTransform(transform);

        ellipseToImageFilter->SetInput(ellipse);
        ellipse->SetDefaultInsideValue(255);
        ellipse->SetDefaultOutsideValue(0);
        ellipseToImageFilter->SetUseObjectValue(true);
        ellipseToImageFilter->SetOutsideValue(0);
        ellipseToImageFilter->Update();

        auto maxFilter = itk::MaximumImageFilter<ImageType,ImageType>::New();
        
        maxFilter->SetInput(0,maskImg);
        maxFilter->SetInput(1,ellipseToImageFilter->GetOutput());
        maxFilter->Update();
        maskImg = maxFilter->GetOutput();
    }

    using WriterType = itk::ImageFileWriter<ImageType>;
    WriterType::Pointer writer = WriterType::New();

    writer->SetFileName(gtFileName);
    writer->SetInput(maskImg);

    try
    {
        writer->Update();
    }
    catch (itk::ExceptionObject & excp)
    {
        std::cerr << excp << std::endl;
        return EXIT_FAILURE;
    }


    f.close();

    return 0;
}