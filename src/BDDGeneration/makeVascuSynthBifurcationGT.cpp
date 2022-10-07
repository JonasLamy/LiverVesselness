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
using PixelDistanceType = float;

using FloatImageType = itk::Image<PixelDistanceType,3>;
using DistanceImageType = FloatImageType;

int main(int argc,char** argv)
{
    std::string inputFileName(argv[1]);
    std::string bifurcationFileName(argv[2]);
    std::string gtFileName(argv[3]);

    // read input

    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(inputFileName);
    reader->Update();
    auto binaryVessels = reader->GetOutput();

    // making empty image
    auto maskImg = ImageType::New();
    maskImg->SetRegions(binaryVessels->GetLargestPossibleRegion() );
    maskImg->SetOrigin( binaryVessels->GetOrigin() );
    maskImg->SetSpacing( binaryVessels->GetSpacing() );
    maskImg->Allocate();
    maskImg->FillBuffer(0);


    // reading distance transform image

    auto inverseFilter = itk::BinaryThresholdImageFilter<ImageType,ImageType>::New();
    inverseFilter->SetLowerThreshold(0);
    inverseFilter->SetUpperThreshold(0);
    inverseFilter->SetInsideValue(254);
    inverseFilter->SetOutsideValue(0);

    inverseFilter->SetInput( binaryVessels );
    inverseFilter->Update();

    auto distanceTransform = itk::SignedMaurerDistanceMapImageFilter<ImageType,DistanceImageType>::New();
    distanceTransform->SquaredDistanceOff();
    //distanceTransform->SquaredDistanceOn();
    distanceTransform->SetInput( inverseFilter->GetOutput() );
    distanceTransform->Update();

    // finding largest vessel radius on the skeleton
    auto distanceImage = distanceTransform->GetOutput();

    using WriterFloatType = itk::ImageFileWriter<FloatImageType>;
    
    WriterFloatType::Pointer distanceWriter = WriterFloatType::New();

    distanceWriter->SetFileName("distanceTransform.nii");
    distanceWriter->SetInput(distanceImage);
    try
    {
        distanceWriter->Update();
    }
    catch (itk::ExceptionObject & excp)
    {
        std::cerr << excp << std::endl;
        return EXIT_FAILURE;
    }
    
    // Reading bifurcation file
    std::ifstream f;
    f.open(bifurcationFileName);
    std::string s_x;
    std::string s_y;
    std::string s_z;

    float x,y,z;
    ImageType::PointType point;

    std::set<ImageType::IndexType> bifurcationsList;

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
        bifurcationsList.insert(bifurcationCoordinates);
    }
    f.close();


            // creating image
    auto resultImage = ImageType::New();
    resultImage->SetRegions( binaryVessels->GetLargestPossibleRegion() );
    
    resultImage->SetSpacing(binaryVessels->GetSpacing() );
    resultImage->Allocate();
    resultImage->FillBuffer(0);
    std::cout<<"result image spacing \n"<<resultImage->GetSpacing() <<std::endl;
    // creating ellipses

    using EllipseType = itk::EllipseSpatialObject<3>;
    using SpacialObjectToImageFilterType = itk::SpatialObjectToImageFilter<EllipseType,ImageType>;
    int radius;

    for(auto &bifurcationIndex : bifurcationsList )
    {
        std::cout<<"bifurcationIndex"<<std::endl;
        std::cout<<bifurcationIndex<<std::endl;
        
        auto ellipse = EllipseType::New();
        EllipseType::ArrayType radiusArray;
        int radius = static_cast<int>( distanceImage->GetPixel(bifurcationIndex) ) | 1 ;
        
        switch(radius)
        {
        case 1:
            radius *= 3;
            break;
        default:
            //radius *=2;
        break;
        }
        radiusArray[0] = radius;
        radiusArray[1] = radius;
        radiusArray[2] = radius;

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
        

        std::cout<<"index:"<<bifurcationIndex<<" radius: "<<radius<<"   | distance transform:"<<distanceImage->GetPixel(bifurcationIndex)<<std::endl;

        // move the ellipse
        auto transform = EllipseType::TransformType::New();
        transform->SetIdentity();
        EllipseType::TransformType::OutputVectorType translation;
        ImageType::IndexType index;
        //index = it.GetIndex();

        //std::cout<<"index:"<<index<<" radius: "<<radius<<std::endl;

        translation[0] = radius;
        translation[1] = radius;
        translation[2] = radius;
        transform->Translate(translation);

        ellipse->SetObjectToParentTransform(transform);


        ellipseToImageFilter->SetInput(ellipse);
        ellipse->SetDefaultInsideValue(255);
        ellipse->SetDefaultOutsideValue(0);
        ellipseToImageFilter->SetUseObjectValue(true);
        ellipseToImageFilter->SetOutsideValue(0);
        ellipseToImageFilter->Update();

        // iterate on ball and translate to real image
        std::cout<<"-- drawing ball --"<<std::endl;
        auto ball = ellipseToImageFilter->GetOutput();
        std::cout<<"ball size :"<<ball->GetLargestPossibleRegion().GetSize()<<std::endl;
        std::cout<<"bifurcation index : "<<bifurcationIndex<<std::endl;

        itk::ImageRegionConstIterator<ImageType> itBall(ball,ball->GetLargestPossibleRegion());
        itBall.GoToBegin();
        while( !itBall.IsAtEnd() )
        {
            if(itBall.Value() == 0 ) 
            {
                ++itBall;
                continue;
            }

            // get iterator index
            auto ballIndex = itBall.GetIndex();

            //std::cout<<"ball index before translation: "<<ballIndex<<std::endl;              
            // translate indexes
            ballIndex[0] += bifurcationIndex[0]-radius;
            ballIndex[1] += bifurcationIndex[1]-radius;
            ballIndex[2] += bifurcationIndex[2]-radius;
            //write value
            //std::cout<<"ball index : "<<ballIndex<<std::endl;
            //std::cout<<"bifurcation index : "<<bifurcationIndex<<std::endl;

            auto resSize = resultImage->GetLargestPossibleRegion().GetSize();

            if( ballIndex[0] >= 0 && ballIndex[0] < resSize[0] && 
            ballIndex[1] >= 0 && ballIndex[1] < resSize[1] &&
            ballIndex[2] >= 0 && ballIndex[2] < resSize[2])
            {
            resultImage->SetPixel(ballIndex, 254 ); //itBall.Value()               
            }
            
            ++itBall;
        }
        std::cout<<"done"<<std::endl;
    }

    resultImage->SetOrigin( binaryVessels->GetOrigin() );

    
    auto maskFilter = itk::MaskImageFilter<ImageType,ImageType,ImageType>::New();
    maskFilter->SetInput( binaryVessels );
    maskFilter->SetMaskImage( resultImage );
    maskFilter->SetOutsideValue(0);
    maskFilter->SetMaskingValue(0);
    maskFilter->Update();

    std::cout<<"5"<<std::endl;
    using OutputWriterType = itk::ImageFileWriter<ImageType>;
    auto writer = OutputWriterType::New();

    writer->SetFileName(gtFileName);
    writer->SetInput( resultImage ); // maskFilter->GetOutput()
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