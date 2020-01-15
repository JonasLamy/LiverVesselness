#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMaximumImageFilter.h"
#include  "itkBinaryThresholdImageFilter.h"

#include <string>
#include <iostream>
#include <boost/filesystem.hpp>
#include <algorithm>
#include <vector>
#include <fstream>


#include "bench_evaluation.h"

using namespace boost::filesystem;

int main(int argc, char ** argv)
{
    std::string outputDirName = argv[1];
    std::string directoryName = argv[2];
    std::string csvFileName = argv[3];

    std::string inputFileName = argv[4];
    std::string groundTruthFileName = argv[5];
    std::string maskFileName = argv[6];

    // Opening file list
    std::ifstream fileNames;
    fileNames.open( inputFileName );
    if( !fileNames.is_open() )
    {
        std::cout<<"couldn't find file list"<<std::endl;
        exit(0);
    }

    int nbFiles = 0;
    int steps = 10;
    int maxStep = 5;

    // load all files
    std::string fileName;
    std::vector<std::string> scaleList;
    while( std::getline(fileNames,fileName) )
    {
        scaleList.push_back(fileName);
        nbFiles++;
        std::cout<<fileName<<std::endl;
        if( nbFiles >= steps * maxStep)
            break;
    }

    fileNames.close();

    std::cout<<nbFiles<<std::endl;

    using ImageType = itk::Image<float,3>;
    using GTImageType = itk::Image<unsigned char,3>;
    using MaskImageType = itk::Image<unsigned char,3>;

    auto readerGT = itk::ImageFileReader<GTImageType>::New();
    auto readerMask = itk::ImageFileReader<MaskImageType>::New();

    readerGT->SetFileName(groundTruthFileName);
    readerGT->Update();
    readerMask->SetFileName(maskFileName);
    readerMask->Update();

    std::cout<<"reading mask and gt image done."<<std::endl;

    auto gtImage = readerGT->GetOutput();

    std::cout<<gtImage->GetLargestPossibleRegion()<<std::endl;

    auto maskImage = readerMask->GetOutput();

    std::cout<<maskImage->GetLargestPossibleRegion()<<std::endl;

    std::ofstream csvFile;
    csvFile.open(csvFileName);
    csvFile<<"Name,Threshold,TP,TN,FP,FN,sensitivity,specificity,precision,accuracy,Dice,MCC"<<std::endl;

    using MaximumImageFilterType = itk::MaximumImageFilter<ImageType>;
    using ThresholdFilterType = itk::BinaryThresholdImageFilter<ImageType,GTImageType>;
    // making range of scale space
    for(int i=0; i<scaleList.size(); i++)
    {
        for(int j=i+1; j<scaleList.size();j++)
        {
            std::string minScaleFileName = scaleList[i];
            std::string maxScaleFileName = scaleList[j];
     
            // selecting all files names in range
            std::cout<<scaleList[i]<<" "<<scaleList[j]<<" :";
            std::vector<std::string> scaleRange;

            
            auto maxImage = ImageType::New();
            maxImage->SetRegions(gtImage->GetLargestPossibleRegion() );
            maxImage->SetOrigin(gtImage->GetOrigin() );
            maxImage->Allocate();
            maxImage->FillBuffer(0);
            
            std::cout<<maxImage->GetLargestPossibleRegion()<<std::endl;

            std::cout<<"creating max image for "<<scaleList[i]<<" "<<scaleList[j]<<" interval done"<<std::endl;

            for(int k=i;k<=j;k++)
            {
                // opening all images in a scales 
                auto reader = itk::ImageFileReader<ImageType>::New();
                reader->SetFileName(directoryName+"/"+ scaleList[k]);
                reader->Update();

                auto img = reader->GetOutput();
                std::cout<<img->GetLargestPossibleRegion()<<std::endl;

                MaximumImageFilterType::Pointer maximumImageFilter = MaximumImageFilterType::New();

                maximumImageFilter->SetInput(0, maxImage);
                maximumImageFilter->SetInput(1,img);
                maximumImageFilter->Update();
                maxImage = maximumImageFilter->GetOutput();
            }

            auto r = itk::ImageFileWriter<itk::Image<float,3>>::New();
            r->SetFileName( std::string(outputDirName+"/stackedScales/") + scaleList[i] + std::string("-") + scaleList[j] + std::string(".nii"));
            r->SetInput(maxImage);
            r->Update();

            // Computing roc curve for the image segmentation
            double bestThreshold = 0;
            double minDist = 1000;
            for(float t=1.0f; t>=0.0f;t-=0.01f)
            {
                // thresholding for all values ( keeping upper value and adding more incertainty as lower probabilities are accepted )
                auto tFilter = ThresholdFilterType::New();
                tFilter->SetInput( maxImage );
                tFilter->SetInsideValue( 1 );
                tFilter->SetOutsideValue( 0 );

                tFilter->SetLowerThreshold(t);
                tFilter->SetUpperThreshold(1.01f);
                tFilter->Update();
                auto segmentationImage = tFilter->GetOutput();
                
                
                Eval<GTImageType,GTImageType,MaskImageType> eval(segmentationImage,gtImage,maskImage,std::to_string(t));
                std::cout<<"true positive rate : " << eval.sensitivity() << "\n"
                        << " false positive rate : " << 1.0f - eval.specificity() << "\n";
                
                // perfect qualifier (0,1), our segmentation (TPR,FPR)
                float euclideanDistance =  (eval.sensitivity()*eval.sensitivity()) + ( 1.0f - eval.specificity() ) * ( 1.0f - eval.specificity() );
                if( minDist >  euclideanDistance )
                {
                    minDist = euclideanDistance;
                    bestThreshold = t;
                }
                std::cout<<scaleList[i]<<"-"<<scaleList[j] <<","<<t<<","<< eval<<std::endl;
                csvFile<< scaleList[i]<<"-"<<scaleList[j] <<","<<t<<","<< eval;
            }  
            std::cout<<std::endl;   
        }
    }

    csvFile.close();

}