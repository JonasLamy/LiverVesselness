#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"



#include "itkXorImageFilter.h"
#include "itkMaskImageFilter.h"

#include "itkFlatStructuringElement.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryMorphologicalOpeningImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"

#include "itkBinaryImageToShapeLabelMapFilter.h"
#include "itkCropImageFilter.h"
#include "itkChangeLabelImageFilter.h"

#include <string>

#include "utils.h"


// settings images types
using RawImageType = itk::Image<int16_t,3>;
using MaskImageType = itk::Image<uint8_t,3>;
using VesselsImageType = MaskImageType;


void cropImages(std::string dataFileName,std::string outputDataFileName,
                std::string maskFileName,std::string outputMaskFileName,
                std::string vesselsFileName, std::string outputvesselsFileName,
                std::string skelFileName, std::string outputSkelFileName)
{
    // setting files paths

    auto imgPatient = vUtils::readImage<RawImageType>(dataFileName,false);
    auto imgMask = vUtils::readImage<MaskImageType>(maskFileName,false);
    auto imgVessels = vUtils::readImage<VesselsImageType>(vesselsFileName,false);
    auto imgSkel = vUtils::readImage<MaskImageType>(skelFileName,false);

    std::cout<<"patient "<<imgPatient->GetLargestPossibleRegion().GetSize()<<std::endl;
    std::cout<<"mask "<<imgMask->GetLargestPossibleRegion().GetSize()<<std::endl;
    std::cout<<"vessels "<<imgVessels->GetLargestPossibleRegion().GetSize()<<std::endl;
    std::cout<<"skeleton "<<imgSkel->GetLargestPossibleRegion().GetSize()<<std::endl;

    using ChangeLabelImageFilterType = itk::ChangeLabelImageFilter<MaskImageType,MaskImageType>;
    auto changeLabelImageFilter = ChangeLabelImageFilterType::New();

    changeLabelImageFilter->SetChange(1,255);
    changeLabelImageFilter->SetInput(imgMask);
    changeLabelImageFilter->Update();
    imgMask = changeLabelImageFilter->GetOutput();

    using BinaryImageToShapeLabelMapFilterType = itk::BinaryImageToShapeLabelMapFilter<MaskImageType>;
    auto bShapeLabelMapFilter = BinaryImageToShapeLabelMapFilterType::New();
    bShapeLabelMapFilter->SetInput(imgMask);
    bShapeLabelMapFilter->Update();

    int nbLabels = bShapeLabelMapFilter->GetOutput()->GetNumberOfLabelObjects();
    if( nbLabels != 1 )
        std::cout<<"incorrect number of liver components, check your data..."<<std::endl;
    std::cout<<"number of components:"<<nbLabels<<std::endl;

    // Loop over all of the blobs
    int biggest = 0;
    int biggestArea = 0;
    BinaryImageToShapeLabelMapFilterType::OutputImageType::LabelObjectType * labelObject;
    for (unsigned int i = 0; i < bShapeLabelMapFilter->GetOutput()->GetNumberOfLabelObjects(); i++)
    {
        labelObject = bShapeLabelMapFilter->GetOutput()->GetNthLabelObject(i);
        // Output the bounding box (an example of one possible property) of the ith region

        std::cout << "Object " << i << " has bounding box " << labelObject->GetBoundingBox() << std::endl;
        itk::Size<3> area = labelObject->GetBoundingBox().GetSize();
        if( biggestArea < area[0] * area[1] * area[2] )
        {
            biggestArea = area[0] * area[1] * area[2];
            biggest = i;
        }
    }
    labelObject = bShapeLabelMapFilter->GetOutput()->GetNthLabelObject(biggest);

    std::cout<<"biggest object :"<<biggest<<std::endl;

    // croping filter based on mask's bounding box
    
    using CropPatientImageFilterType = itk::CropImageFilter<RawImageType,RawImageType>;
    using CropMaskImageFilterType = itk::CropImageFilter<MaskImageType,MaskImageType>;
    using CropVesselsImageFilterType = CropMaskImageFilterType;

    auto cropPatientImageFilter = CropPatientImageFilterType::New(); 
    auto cropMaskImageFilter = CropMaskImageFilterType::New();
    auto cropSkelFilter = CropMaskImageFilterType::New();
    auto cropVesselsImageFilter = CropVesselsImageFilterType::New();

    itk::Size<3> lowerSize;
    itk::Size<3> upperSize;

    MaskImageType::IndexType maxIndex;

    // Vessels in livers are often <15 pixels, since most methods 
    int border = 20;
    lowerSize[0] = labelObject->GetBoundingBox().GetIndex()[0] - border;
    lowerSize[1] = labelObject->GetBoundingBox().GetIndex()[1] - border;
    lowerSize[2] = labelObject->GetBoundingBox().GetIndex()[2];

    if( lowerSize[2] > border )
        lowerSize[2] -= border;

    maxIndex = imgMask->GetLargestPossibleRegion().GetUpperIndex();
    labelObject->GetBoundingBox().GetUpperIndex()[0];


    upperSize[0] = maxIndex[0] - labelObject->GetBoundingBox().GetUpperIndex()[0] - border;
    upperSize[1] = maxIndex[1] - labelObject->GetBoundingBox().GetUpperIndex()[1] - border;
    upperSize[2] = maxIndex[2] - labelObject->GetBoundingBox().GetUpperIndex()[2];

    if( upperSize[2] > border)
        upperSize[2] -= border;

    cropPatientImageFilter->SetLowerBoundaryCropSize( lowerSize );
    cropPatientImageFilter->SetUpperBoundaryCropSize( upperSize );

    cropPatientImageFilter->SetInput(imgPatient);
    cropPatientImageFilter->Update();

    cropVesselsImageFilter->SetInput(imgVessels);
    cropVesselsImageFilter->SetLowerBoundaryCropSize( lowerSize);
    cropVesselsImageFilter->SetUpperBoundaryCropSize( upperSize);
    cropVesselsImageFilter->Update();

    cropMaskImageFilter->SetInput(imgMask);
    cropMaskImageFilter->SetLowerBoundaryCropSize(lowerSize);
    cropMaskImageFilter->SetUpperBoundaryCropSize(upperSize);
    cropMaskImageFilter->Update();

    cropSkelFilter->SetInput(imgSkel);
    cropSkelFilter->SetLowerBoundaryCropSize(lowerSize);
    cropSkelFilter->SetUpperBoundaryCropSize(upperSize);
    cropSkelFilter->Update();

    std::cout<<"size after crop:"<<cropPatientImageFilter->GetOutput()->GetLargestPossibleRegion().GetSize()<<std::endl;
    /*
    using MaskFilterType = itk::MaskImageFilter<DicomImageType,MaskImageType>;
    auto maskFilter = MaskFilterType::New();

    maskFilter->SetInput(cropPatientImageFilter->GetOutput());
    maskFilter->SetMaskImage(cropMaskImageFilter->GetOutput());
    */
    using RawWriterType = itk::ImageFileWriter<RawImageType>;
    auto writer = RawWriterType::New();
    writer->SetInput(cropPatientImageFilter->GetOutput() );
    writer->SetFileName( outputDataFileName);
    writer->Update();
    /*
    writer->SetInput(maskFilter->GetOutput());
    writer->SetFileName(outputMaskedLiverFileName);
    writer->Update();
    */
    using VesselsWriterType = itk::ImageFileWriter<VesselsImageType>;
    auto vesselsWriter = VesselsWriterType::New();
    vesselsWriter->SetInput(cropVesselsImageFilter->GetOutput() );
    vesselsWriter->SetFileName( outputvesselsFileName);
    vesselsWriter->Update();

    using MaskWriterType = itk::ImageFileWriter<MaskImageType>;
    auto maskWriter = MaskWriterType::New();
    maskWriter->SetInput(cropMaskImageFilter->GetOutput() );
    maskWriter->SetFileName( outputMaskFileName);
    maskWriter->Update();

    using MaskWriterType = itk::ImageFileWriter<MaskImageType>;
    auto skelWriter = MaskWriterType::New();
    skelWriter->SetInput(cropSkelFilter->GetOutput() );
    skelWriter->SetFileName( outputSkelFileName);
    skelWriter->Update();

}




int main(int argc,char** argv)
{
    // Bullit
    // For now only as ground truth and data
    std::string patientFileName( argv[1] );
    std::string croppedPatientFileName( argv[2] );
    std::string outputPatientFileName( argv[3] );

    std::string maskName( argv[4] );
    std::string croppedMaskName( argv[5] );
    std::string outputMaskName( argv[6] );

    std::string vesselsName( argv[7] );
    std::string croppedVesselsName( argv[8] );
    std::string outputVesselsName( argv[9] );

    std::string skelName( argv[10] );
    std::string croppedSkelName( argv[11] );
    std::string outputSkelName( argv[12] );

    std::string outputMaskedVolume( argv[13] );

    bool identitySpacing = std::atoi(argv[14]);

    // One) crop images - including, volume, mask, vessels
    // Two) make cropped images iso
    cropImages(patientFileName,croppedPatientFileName,
                maskName,croppedMaskName,
                vesselsName, croppedVesselsName,
                skelName,croppedSkelName);

    // opening files
    auto imgPatient = vUtils::readImage<RawImageType>(croppedPatientFileName,false);
    auto imgVessels = vUtils::readImage<VesselsImageType>(croppedVesselsName,false);
    
    auto imgMask = vUtils::readImage<MaskImageType>(croppedMaskName,false);
    auto imgSkel = vUtils::readImage<MaskImageType>(croppedSkelName,false);
    // making maskedLiver

    using MaskFilterType = itk::MaskImageFilter<RawImageType,MaskImageType>;
    auto maskFilter = MaskFilterType::New();

    maskFilter->SetInput(imgPatient);
    maskFilter->SetMaskImage( imgMask ); //maskFilter->SetMaskImage( maskDilateFilter->GetOutput() );
    maskFilter->Update();
    
    auto imgPatientIso = vUtils::makeIso<RawImageType>(imgPatient,false,identitySpacing);
    auto imgVesselsIso = vUtils::makeIso<VesselsImageType>(imgVessels,true,identitySpacing);

    auto imgMaskIso = vUtils::makeIso<MaskImageType>(imgMask,true,identitySpacing);
    auto imgSkelIso = vUtils::makeIso<MaskImageType>(imgSkel,true,identitySpacing); 
    auto imgMaskedIso = vUtils::makeIso<RawImageType>(maskFilter->GetOutput(),false,identitySpacing);

    std::cout<<"patient "<<imgPatientIso->GetLargestPossibleRegion().GetSize()<<std::endl;
    std::cout<<"gt "<<imgVesselsIso->GetLargestPossibleRegion().GetSize()<<std::endl;
    std::cout<<"mask "<<imgMaskIso->GetLargestPossibleRegion().GetSize()<<std::endl;
    std::cout<<"skel "<<imgSkelIso->GetLargestPossibleRegion().GetSize()<<std::endl;
    std::cout<<"maskedLiver "<<imgMaskedIso->GetLargestPossibleRegion().GetSize()<<std::endl;

    using RawWriterType = itk::ImageFileWriter<RawImageType>;
    auto writer = RawWriterType::New();
    writer->SetInput(imgPatientIso );
    writer->SetFileName( outputPatientFileName);
    writer->Update();

    writer->SetInput(imgMaskedIso);
    writer->SetFileName(outputMaskedVolume);
    writer->Update();

    using VesselsWriterType = itk::ImageFileWriter<VesselsImageType>;
    auto GTWriter = VesselsWriterType::New();
    GTWriter->SetInput(imgVesselsIso );
    GTWriter->SetFileName( outputVesselsName);
    GTWriter->Update();

    
    using MaskWriterType = itk::ImageFileWriter<MaskImageType>;
    auto maskWriter = MaskWriterType::New();
    maskWriter->SetInput(imgMaskIso );
    maskWriter->SetFileName( outputMaskName);
    maskWriter->Update();
    
    using WriterType = itk::ImageFileWriter<MaskImageType>;
    auto SkelWriter = WriterType::New();
    SkelWriter->SetInput(imgSkelIso);
    SkelWriter->SetFileName(outputSkelName);
    SkelWriter->Update();
    
    return 0;
}
