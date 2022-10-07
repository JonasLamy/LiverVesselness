#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkBinaryImageToShapeLabelMapFilter.h"
#include "itkCropImageFilter.h"
//#include "itkMaskImageFilter.h"

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
    std::string outputPortalFileName( argv[8] );
    //std::string outputMaskedLiverFileName( argv[9] );

    // settings images types
    using DicomImageType = itk::Image<int16_t,3>;
    using MaskImageType = itk::Image<uint8_t,3>;
    using VesselsImageType = itk::Image<uint8_t,3>;

    // opening files
    auto imgPatient = vUtils::readImage<DicomImageType>(patientFileName,true);
    auto imgMask = vUtils::readImage<MaskImageType>(maskLiverFileName,true);
    auto imgVessels = vUtils::readImage<VesselsImageType>(vesselsFileName,true);
    auto imgPortal = vUtils::readImage<VesselsImageType>(portalFileName,true);
    
    std::cout<<"patient "<<imgPatient->GetLargestPossibleRegion().GetSize()<<std::endl;
    std::cout<<"mask "<<imgMask->GetLargestPossibleRegion().GetSize()<<std::endl;
    std::cout<<"vessels "<<imgVessels->GetLargestPossibleRegion().GetSize()<<std::endl;
    std::cout<<"portal "<<imgPortal->GetLargestPossibleRegion().GetSize()<<std::endl;

    // Todo : need to check if data is beetween [0,1]. If it is, rescale to 255

    MaskImageType::IndexType index;
    index[0] = 255;
    index[1] = 233;
    index[2] = 125;
    std::cout<<(int)imgMask->GetPixel(index)<<std::endl;

    // computing mask bounding box

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
    
    using CropPatientImageFilterType = itk::CropImageFilter<DicomImageType,DicomImageType>;
    using CropMaskImageFilterType = itk::CropImageFilter<MaskImageType,MaskImageType>;
    using CropVesselsImageFilterType = itk::CropImageFilter<VesselsImageType,VesselsImageType>;
    using CropPortalImageFilterType = itk::CropImageFilter<VesselsImageType,VesselsImageType>;

    auto cropPatientImageFilter = CropPatientImageFilterType::New(); 
    auto cropMaskImageFilter = CropMaskImageFilterType::New();
    auto cropVesselsImageFilter = CropVesselsImageFilterType::New();
    auto cropPortalImageFilter = CropPortalImageFilterType::New();

    itk::Size<3> lowerSize;
    itk::Size<3> upperSize;

    MaskImageType::IndexType maxIndex;

    // Vessels in livers are often <15 pixels, since most methods 
    int border = 15;
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

    cropPortalImageFilter->SetInput(imgPortal);
    cropPortalImageFilter->SetLowerBoundaryCropSize(lowerSize);
    cropPortalImageFilter->SetUpperBoundaryCropSize(upperSize);
    cropPortalImageFilter->Update();


    std::cout<<"size after crop:"<<cropPatientImageFilter->GetOutput()->GetLargestPossibleRegion().GetSize()<<std::endl;
    /*
    using MaskFilterType = itk::MaskImageFilter<DicomImageType,MaskImageType>;
    auto maskFilter = MaskFilterType::New();

    maskFilter->SetInput(cropPatientImageFilter->GetOutput());
    maskFilter->SetMaskImage(cropMaskImageFilter->GetOutput());
    */
    using DicomWriterType = itk::ImageFileWriter<DicomImageType>;
    auto writer = DicomWriterType::New();
    writer->SetInput(cropPatientImageFilter->GetOutput() );
    writer->SetFileName( outputPatientFileName);
    writer->Update();
    /*
    writer->SetInput(maskFilter->GetOutput());
    writer->SetFileName(outputMaskedLiverFileName);
    writer->Update();
    */
    using VesselsWriterType = itk::ImageFileWriter<VesselsImageType>;
    auto vesselsWriter = VesselsWriterType::New();
    vesselsWriter->SetInput(cropVesselsImageFilter->GetOutput() );
    vesselsWriter->SetFileName( outputVesselsFileName);
    vesselsWriter->Update();

    using MaskWriterType = itk::ImageFileWriter<MaskImageType>;
    auto maskWriter = MaskWriterType::New();
    maskWriter->SetInput(cropMaskImageFilter->GetOutput() );
    maskWriter->SetFileName( outputMaskFileName);
    maskWriter->Update();

    using PortalWriterType = itk::ImageFileWriter<VesselsImageType>;
    auto portalWriter = PortalWriterType::New();
    portalWriter->SetInput(cropPortalImageFilter->GetOutput() );
    portalWriter->SetFileName( outputPortalFileName);
    portalWriter->Update();

    return 0;
}