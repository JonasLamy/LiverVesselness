#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkBinaryThresholdImageFilter.h"
#include "itkBinaryThinningImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkConvolutionImageFilter.h"

#include "itkEllipseSpatialObject.h"
#include "itkSpatialObjectToImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkImageDuplicator.h"
#include "itkConstantPadImageFilter.h"

#include "QuickView.h"

#include <string>

using PixelType = unsigned char;
using GTPixelType = unsigned char;
using ImageType = itk::Image<PixelType,3>;
using GTImageType = itk::Image<PixelType,3>;

int main(int argc,char** argv)
{
    std::string inputFileName(argv[1]);
    std::string bifurcationFileName(argv[2]);
    std::string gtFileName(argv[3]);
    int boxSize = std::atoi(argv[4]);

    // read input

    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(inputFileName);
    reader->Update();
    auto img = reader->GetOutput();

    // making empty image
    auto duplicator = itk::ImageDuplicator<ImageType>::New();
    duplicator->SetInputImage(img);
    duplicator->Update();
    auto maskImg = duplicator->GetOutput();
    maskImg->FillBuffer(0);
    
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

        ImageType::PointType boxCenterPhys;
        boxCenterPhys[0] = x;
        boxCenterPhys[1] = y;
        boxCenterPhys[2] = z;
        
        ImageType::IndexType boxCenterIndex = maskImg->TransformPhysicalPointToIndex( boxCenterPhys );
        ImageType::IndexType boxOriginIndex;
        boxOriginIndex[0] = boxCenterIndex[0] - boxSize/2;
        boxOriginIndex[1] = boxCenterIndex[1] - boxSize/2;
        boxOriginIndex[2] = boxCenterIndex[2] - boxSize/2;

        ImageType::IndexType boxUpperIndex;
        boxUpperIndex[0] = boxCenterIndex[0] + boxSize/2;
        boxUpperIndex[1] = boxCenterIndex[1] + boxSize/2;
        boxUpperIndex[2] = boxCenterIndex[2] + boxSize/2;

        ImageType::RegionType boxRegion;
        boxRegion.SetIndex(boxOriginIndex);
        boxRegion.SetUpperIndex(boxUpperIndex);

        maskImg->SetRequestedRegion( boxRegion );
        std::cout<<boxRegion;
        try{
            itk::ImageRegionIterator<ImageType> it(maskImg,maskImg->GetRequestedRegion());
            while(!it.IsAtEnd())
            {   
                /*
                ImageType::IndexType indexIt= it.GetIndex();
                int lX = (int)indexIt[0] - (int)center[0];
                int lY = (int)indexIt[1] - (int)center[1];
                int lZ = (int)indexIt[2] - (int)center[2];

                if( (lX*lX + lY * lY + lZ * lZ) <= (boxSize/2)*(boxSize/2) )
                {
                    it.Set(255);
                }*/
                it.Set(255);
                ++it;
            }
        }
        catch(itk::ExceptionObject e)
        {

        }
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