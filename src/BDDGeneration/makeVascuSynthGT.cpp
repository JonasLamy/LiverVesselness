#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkBinaryThresholdImageFilter.h"
#include "itkBinaryThinningImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkConvolutionImageFilter.h"

#include "itkFlatStructuringElement.h"
#include "itkBinaryDilateImageFilter.h"

#include <string>

using PixelType = unsigned char;
using GTPixelType = unsigned char;
using ImageType = itk::Image<PixelType,3>;
using GTImageType = itk::Image<PixelType,3>;




int main(int argc,char** argv)
{
    std::string inputFile(argv[1]);
    std::string gtFile(argv[2]);
    std::string gtDilatedFile(argv[3]);
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

    // creating the iso dilated vessels mask

    using StructuringElementType = itk::FlatStructuringElement<3>;
    StructuringElementType::RadiusType radius;
    radius.Fill(3);
    StructuringElementType structuringElement = StructuringElementType::Ball(radius);

    using BinaryDilateImageFilterType = itk::BinaryDilateImageFilter<GTImageType, GTImageType, StructuringElementType>;

    BinaryDilateImageFilterType::Pointer dilateFilter = BinaryDilateImageFilterType::New();
    dilateFilter->SetInput(gt);
    dilateFilter->SetKernel(structuringElement);
    dilateFilter->Update();
    auto dilatedGT = dilateFilter->GetOutput();

    auto gtDilatedWriter = GTWriterType::New();
    gtDilatedWriter->SetFileName(gtDilatedFile);
    gtDilatedWriter->SetInput(dilatedGT);
    gtDilatedWriter->Update();

    return 0;
}