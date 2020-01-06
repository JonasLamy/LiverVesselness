#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkBinaryThresholdImageFilter.h"
#include "itkBinaryThinningImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkConvolutionImageFilter.h"

#include <string>

using PixelType = unsigned char;
using GTPixelType = unsigned char;
using ImageType = itk::Image<PixelType,3>;
using GTImageType = itk::Image<PixelType,3>;


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

  while (!imageIterator.IsAtEnd())
  {
    // imageIterator.Set(255);
    imageIterator.Set(1);

    ++imageIterator;
  }
}



int main(int argc,char** argv)
{
    std::string inputFile(argv[1]);
    std::string gtFile(argv[2]);

    // read input

    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(inputFile);
    reader->Update();
    auto img = reader->GetOutput();

    // binarize input to create binary GT

    using BinaryFilterType = itk::BinaryThresholdImageFilter<ImageType,GTImageType>;
    auto binaryFilter = BinaryFilterType::New();
    binaryFilter->SetInsideValue(255);
    binaryFilter->SetOutsideValue(0);
    binaryFilter->SetLowerThreshold(1);
    binaryFilter->SetUpperThreshold(255);
    binaryFilter->SetInput(img);

    binaryFilter->Update();

    auto gt = binaryFilter->GetOutput();

    using GTWriterType = itk::ImageFileWriter<GTImageType>;
    auto gtWriter = GTWriterType::New();
    gtWriter->SetFileName(gtFile);
    gtWriter->SetInput(gt);
    gtWriter->Update();


    return 0;
}