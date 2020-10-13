/*
Author : Jonas lamy
Based on work of Turetken & Fethallah Benmansour
*/

#include "itkOptimallyOrientedFlux.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkTimeProbe.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkMaskImageFilter.h"

#include "CLI11.hpp"


#include "utils.h"

int main(int argc, char** argv)
{
  //********************************************************
  //                   Reading arguments
  //********************************************************
  bool isInputDicom;
  // parse command line using CLI ----------------------------------------------
  CLI::App app;
  app.description("Apply the OOF algorithm");
  std::string inputFile ;
  std::string outputFile;
  double sigmaMin;
  double sigmaMax;
  int nbSigmaSteps;
  double fixedSigma;
  std::string maskFile;
  
  app.add_option("-i,--input,1", inputFile, "inputName : input img" )
      ->required()
      ->check(CLI::ExistingFile);
  
  app.add_option("--output,-o",outputFile, "ouputName : output img");
  app.add_option("--sigmaMin,-m", sigmaMin, "scale space sigma min");
  app.add_option("--sigmaMax,-M", sigmaMax, "scale space sigma max");
  app.add_option("--nbSigmaSteps,-n",nbSigmaSteps,  "nb steps sigma");
  app.add_option("--sigma,-s",fixedSigma,"sigma for smoothing");
  app.add_flag("--inputIsDicom,-d",isInputDicom ,"specify dicom input");
  app.add_option("--mask,-k",maskFile,"mask response by image")
  ->check(CLI::ExistingFile);

  
    //********************************************************
    //                    Reading inputs
    //********************************************************


    const unsigned int Dimension = 3;

    typedef double PixelType;
    typedef itk::Image<PixelType,Dimension> InputImageType;
    typedef itk::Image<uint8_t, Dimension> MaskImageType;

    InputImageType::Pointer inputImage = vUtils::readImage<InputImageType>(inputFile,isInputDicom);
    
    
    //********************************************************
    //                   Filter
    //********************************************************

    using GaussianFilterType = itk::DiscreteGaussianImageFilter<InputImageType,InputImageType>;
    auto gFilter = GaussianFilterType::New();
    gFilter->SetVariance(fixedSigma);
    gFilter->SetInput(inputImage);

    // creating radii from parameters

    double step = (sigmaMax - sigmaMin) / (double)(nbSigmaSteps-2+1);
    
    std::vector<double> radii;
    double i=sigmaMin;
    while(i<sigmaMax)
    {
      radii.push_back(i);
      i += step;
    }
    radii.push_back( sigmaMax );

    std::cout<<"scales: ";
    for(auto r :radii )
      std::cout<<r<<" ; ";
    std::cout<<std::endl;


    auto OOFfilter = itk::OptimallyOrientedFlux<InputImageType,InputImageType>::New();
    OOFfilter->SetInput( gFilter->GetOutput() );
    OOFfilter->SetRadii(radii);
    try{
        itk::TimeProbe timer;
        timer.Start();
        //
        OOFfilter->Update();

        //
        timer.Stop();
        std::cout<<"Computation time:"<<timer.GetMean()<<std::endl;
    }
    catch(itk::ExceptionObject &e)
    {
        std::cerr << e << std::endl;
    }

  
    auto rescaleFilter = itk::RescaleIntensityImageFilter<InputImageType>::New();
    
    MaskImageType::Pointer maskImage;
    if( !maskFile.empty() )
    {
      maskImage = vUtils::readImage<MaskImageType>(maskFile,isInputDicom);
    
      auto maskFilter = itk::MaskImageFilter<InputImageType,MaskImageType>::New();
      maskFilter->SetInput( OOFfilter->GetOutput() );
      maskFilter->SetMaskImage(maskImage);
      maskFilter->SetMaskingValue(0);
      maskFilter->SetOutsideValue(0);

      maskFilter->Update();
      rescaleFilter->SetInput( maskFilter->GetOutput() );
    }
    else{
      rescaleFilter->SetInput(OOFfilter->GetOutput());
    }
  
    rescaleFilter->SetOutputMinimum(0);
    rescaleFilter->SetOutputMaximum(1);
    

    typedef itk::ImageFileWriter<InputImageType> ScoreImageWriter;
    auto writer = ScoreImageWriter::New();
    writer->SetInput( rescaleFilter->GetOutput() );
    writer->SetFileName(outputFile);

    try{
        writer->Update();
    }
    catch(itk::ExceptionObject &e)
    {
        std::cerr << e << std::endl;
    }

    return 0;
}
