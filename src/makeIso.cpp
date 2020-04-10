#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"



#include "itkXorImageFilter.h"
#include "itkMaskImageFilter.h"

#include "itkFlatStructuringElement.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryMorphologicalOpeningImageFilter.h"

#include "itkBinaryThresholdImageFilter.h"


#include <string>

#include "utils.h"


int main(int argc,char** argv)
{
    // setting files paths
    std::string patientFileName( argv[1] );
    std::string maskLiverFileName( argv[2] );
    std::string vesselsFileName( argv[3] );
    std::string portalFileName( argv[4] );

    std::string outputPatientFileName( argv[5] );
    std::string outputMaskFileName( argv[6] );
    std::string outputVesselsFileName( argv[7] );
    std::string outputVesselsMaskFileName( argv[8] );
    std::string outputMaskedLiverFileName( argv[9] );

    bool identitySpacing = std::atoi(argv[10]);
    int radiusValue = 7;

    // settings images types
    using DicomImageType = itk::Image<int16_t,3>;
    using MaskImageType = itk::Image<uint8_t,3>;
    using VesselsImageType = itk::Image<uint8_t,3>;

    // opening files
    auto imgPatient = vUtils::readImage<DicomImageType>(patientFileName,false);
    auto imgMask = vUtils::readImage<MaskImageType>(maskLiverFileName,false);
    auto imgVenacava = vUtils::readImage<VesselsImageType>(vesselsFileName,false);
    auto imgPortal = vUtils::readImage<VesselsImageType>(portalFileName,false);


    // making vessels GT only one image

    auto xorImageFilter = itk::XorImageFilter<VesselsImageType,VesselsImageType>::New();
    xorImageFilter->SetInput1(imgVenacava);
    xorImageFilter->SetInput2(imgPortal);
    xorImageFilter->Update();
    auto imgVessels = xorImageFilter->GetOutput();

    // making mask bigger on iso data ( not needed for now )
    using StructuringElementType = itk::FlatStructuringElement<3>;
    /*
    
    StructuringElementType::RadiusType MaskradiusOpening;
    MaskradiusOpening.Fill(3);
    StructuringElementType MaskstructuringElementOpening = StructuringElementType::Ball(MaskradiusOpening);


    using BinaryOpeningFilter = itk::BinaryMorphologicalOpeningImageFilter<MaskImageType,MaskImageType,StructuringElementType>;
    auto openingFilter = BinaryOpeningFilter::New();
    openingFilter->SetKernel(MaskstructuringElementOpening);
    openingFilter->SetInput(imgMask);
    openingFilter->Update();

    StructuringElementType::RadiusType Maskradius;
    Maskradius.Fill(9);
    StructuringElementType MaskstructuringElement = StructuringElementType::Ball(Maskradius);

    
    using BinaryDilateImageFilterType = itk::BinaryDilateImageFilter<MaskImageType, MaskImageType, StructuringElementType>;

    BinaryDilateImageFilterType::Pointer maskDilateFilter = BinaryDilateImageFilterType::New();
    maskDilateFilter->SetInput( openingFilter->GetOutput() );
    maskDilateFilter->SetKernel(MaskstructuringElement);
    maskDilateFilter->Update();
    */

    // making maskedLiver

    using MaskFilterType = itk::MaskImageFilter<DicomImageType,MaskImageType>;
    auto maskFilter = MaskFilterType::New();

    maskFilter->SetInput(imgPatient);
    maskFilter->SetMaskImage( imgMask ); //maskFilter->SetMaskImage( maskDilateFilter->GetOutput() );
    maskFilter->Update();
    
    auto imgMaskedLiverIso = vUtils::makeIso<DicomImageType>(maskFilter->GetOutput(),false,identitySpacing);
    auto imgPatientIso = vUtils::makeIso<DicomImageType>(imgPatient,false,identitySpacing);
    auto imgMaskIso = vUtils::makeIso<MaskImageType>(imgMask,true,identitySpacing); //auto imgMaskIso = makeIso<MaskImageType>(maskDilateFilter->GetOutput(),true,identitySpacing);
    auto imgVesselsIso = vUtils::makeIso<VesselsImageType>(imgVessels,true,identitySpacing);

    // creating the iso dilated vessels mask
    StructuringElementType::RadiusType radius;
    radius.Fill(radiusValue);
    StructuringElementType structuringElement = StructuringElementType::Ball(radius);

    using BinaryDilateImageFilterType = itk::BinaryDilateImageFilter<VesselsImageType, MaskImageType, StructuringElementType>;

    BinaryDilateImageFilterType::Pointer dilateFilter = BinaryDilateImageFilterType::New();
    dilateFilter->SetInput( imgVesselsIso );
    dilateFilter->SetKernel(structuringElement);
    dilateFilter->Update();
    auto imgMaskedDilatedVessels = dilateFilter->GetOutput();

    std::cout<<"patient "<<imgPatientIso->GetLargestPossibleRegion().GetSize()<<std::endl;
    std::cout<<"mask "<<imgMaskIso->GetLargestPossibleRegion().GetSize()<<std::endl;
    std::cout<<"vessels "<<imgVesselsIso->GetLargestPossibleRegion().GetSize()<<std::endl;
    std::cout<<"maskedLiver "<<imgMaskedLiverIso->GetLargestPossibleRegion().GetSize()<<std::endl;
    std::cout<<"maskDilatedVessels "<<imgMaskedDilatedVessels->GetLargestPossibleRegion().GetSize()<<std::endl;



    using DicomWriterType = itk::ImageFileWriter<DicomImageType>;
    auto writer = DicomWriterType::New();
    writer->SetInput(imgPatientIso );
    writer->SetFileName( outputPatientFileName);
    writer->Update();

    writer->SetInput(imgMaskedLiverIso);
    writer->SetFileName(outputMaskedLiverFileName);
    writer->Update();

    using VesselsWriterType = itk::ImageFileWriter<VesselsImageType>;
    auto vesselsWriter = VesselsWriterType::New();
    vesselsWriter->SetInput(imgVesselsIso );
    vesselsWriter->SetFileName( outputVesselsFileName);
    vesselsWriter->Update();

    using MaskWriterType = itk::ImageFileWriter<MaskImageType>;
    auto maskWriter = MaskWriterType::New();
    maskWriter->SetInput(imgMaskIso );
    maskWriter->SetFileName( outputMaskFileName);
    maskWriter->Update();

    using WriterType = itk::ImageFileWriter<MaskImageType>;
    WriterType::Pointer vesselsMaskWriter = WriterType::New();
    vesselsMaskWriter->SetInput(imgMaskedDilatedVessels);
    vesselsMaskWriter->SetFileName(outputVesselsMaskFileName);
    vesselsMaskWriter->Update();

    return 0;
}