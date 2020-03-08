#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkBinaryThinningImageFilter3D.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkConvolutionImageFilter.h"
#include "itkFlatStructuringElement.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkMaskImageFilter.h"


//#include "QuickView.h"



#include <string>

using PixelType = unsigned char;
using ImageType = itk::Image<PixelType,3>;


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

    std::cout<<"2"<<std::endl;


    // creating structuring element

    auto kernel = ImageType::New();
    CreateKernel(kernel,5);

    auto convFilter = itk::ConvolutionImageFilter<ImageType,ImageType>::New();
    convFilter->SetInput(maskFilter->GetOutput());
    convFilter->SetKernelImage(kernel);
    convFilter->Update();

    std::cout<<"3"<<std::endl;

    auto thresholdFilter = itk::BinaryThresholdImageFilter<ImageType,ImageType>::New();
    thresholdFilter->SetLowerThreshold(7);
    thresholdFilter->SetUpperThreshold(255);
    thresholdFilter->SetInsideValue(255);
    thresholdFilter->SetOutsideValue(0);
    thresholdFilter->SetInput(convFilter->GetOutput());
    thresholdFilter->Update();
    
    std::cout<<"4"<<std::endl;

    using StructuringElementType = itk::FlatStructuringElement<3>;
    StructuringElementType::RadiusType radius;
    radius.Fill(boxSize);
    StructuringElementType structuringElement = StructuringElementType::Box(radius);

    auto dilateFilter = itk::BinaryDilateImageFilter<ImageType,ImageType,StructuringElementType>::New();
    dilateFilter->SetInput( thresholdFilter->GetOutput() );
    dilateFilter->SetForegroundValue(255);
    dilateFilter->SetKernel( structuringElement );
    dilateFilter->Update();

    std::cout<<"5"<<std::endl;

    WriterType::Pointer writer = WriterType::New();

    writer->SetFileName(gtFileName);
    writer->SetInput(dilateFilter->GetOutput());
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