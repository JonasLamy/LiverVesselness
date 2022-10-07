#include "itkImage.h"
#include "itkImageFileWriter.h"
#include "utils.h"
#include "itkMaskImageFilter.h"


#include <string>
int main(int argc, char ** argv)
{
    std::string input(argv[1]);
    std::string inputMask(argv[2]);
    std::string output(argv[3]);

    typedef itk::Image<uint8_t,3> ImageType;

    auto image = vUtils::readImage<ImageType>(input,false);
    auto liverMask = vUtils::readImage<ImageType>(inputMask,false);

    std::cout<<image->GetLargestPossibleRegion()<<std::endl;
    std::cout<<liverMask->GetLargestPossibleRegion()<<std::endl;

    using MaskFilterType = itk::MaskImageFilter<ImageType,ImageType>;
    auto maskFilter = MaskFilterType::New();

    maskFilter->SetInput(image);
    maskFilter->SetMaskImage( liverMask ); //maskFilter->SetMaskImage( maskDilateFilter->GetOutput() );
    maskFilter->SetOutsideValue(0);
    maskFilter->SetMaskingValue(0);
    maskFilter->Update();

    auto isoGT = vUtils::makeIso<ImageType>(maskFilter->GetOutput(),true,0);

    using WriterType = itk::ImageFileWriter<ImageType>;
    WriterType::Pointer vesselsMaskWriter = WriterType::New();
    vesselsMaskWriter->SetInput(isoGT);
    vesselsMaskWriter->SetFileName(output);
    vesselsMaskWriter->Update();
}