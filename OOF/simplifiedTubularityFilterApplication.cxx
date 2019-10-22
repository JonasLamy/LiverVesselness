/*
Author : Jonas lamy
Based on work of Turetken & Fethallah Benmansour
*/

#include "itkMultiScaleTubularityMeasureImageFilter.h"
#include "itkOrientedFluxCrossSectionTraceMeasure.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImage.h"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

int main(int argc, char** argv)
{
    //********************************************************
    //                   Reading arguments
    //********************************************************
    namespace po = boost::program_options;
    // parsing arguments
    po::options_description general_opt("Allowed options are ");
    general_opt.add_options()
    ("help,h", "display this message")
    ("input,i", po::value<std::string>(), "inputName : input img" )
    ("output,o", po::value<std::string>(), "ouputName : output img" )
    ("sigmaMin,m", po::value<float>(), "scale space sigma min")
    ("sigmaMax,M", po::value<float>(), "scale space sigma max")
    ("nbSigmaSteps,n",po::value<int>(),"nb steps sigma")
    ("sigma,s",po::value<double>(),"sigma for smoothing");

    bool parsingOK = true;
    po::variables_map vm;

    try{
      po::store(po::parse_command_line(argc,argv,general_opt),vm);
    }catch(const std::exception& ex)
    {
      parsingOK = false;
      std::cout<<"Error checking program option"<<ex.what()<< std::endl;
    }

    po::notify(vm);
    if( !parsingOK || vm.count("help") || argc<=1 )
    {
      std::cout<<"\n Usage : vesselGenerator [backGroundIntensity] [gradMin] [gradMax] [gNoiseMean] [gNoiseStDev] \n\n"
                << " backgroundIntensity : intensity of the backGround (0-255)\n"
                << " gradMin - gradMax : greyscale gradient values (0-255)\n"
                << " gNoiseMean : gaussian additive white noise mean \n"
                << " gNoiseStDev : gaussian additive white noise standard deviation \n"
		            << " ouputName : output img without extension, the program will generate both nifti and image output\n\n"
                <<"example : ./vesselGenerator 128 0 255 128 50 \n" << std::endl;
    
      return 0;
    }

    std::string inputFile = vm["input"].as<std::string>();
    std::string outputFile = vm["output"].as<std::string>();
    float sigmaMin = vm["sigmaMin"].as<float>();
    float sigmaMax = vm["sigmaMax"].as<float>();
    int nbSigmaSteps = vm["nbSigmaSteps"].as<int>();
    double fixedSigma = vm["sigma"].as<double>();

    //********************************************************
    //                    Reading inputs
    //********************************************************


    const unsigned int maxDimension = 3;

    typedef float PixelType;
    typedef itk::Image<PixelType,maxDimension> InputImageType;
    typedef itk::ImageFileReader<InputImageType> ReaderType;

    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(inputFile);
    reader->Update();

    InputImageType::Pointer inputImage = reader->GetOutput();

    //********************************************************
    //                   Filter
    //********************************************************

    typedef InputImageType OutputImageType;
    typedef float OFScalarType;
    typedef itk::SymmetricSecondRankTensor< OFScalarType,maxDimension> OFPixelType;
    typedef itk::Image<OFPixelType,maxDimension> OFImageType;
    typedef itk::Image<PixelType,maxDimension> ScoreImageType;
    typedef itk::Image<PixelType, maxDimension+1> NPlus1ScoreImage;
    typedef itk::OrientedFluxCrossSectionTraceMeasureFilter<OFImageType, OutputImageType> OFCrossSectionTraceObjectnessFilterType;
    typedef itk::MultiScaleTubularityMeasureImageFilter<InputImageType,
                                                      OFImageType,
                                                      ScoreImageType,
                                                      OFCrossSectionTraceObjectnessFilterType,
                                                      OutputImageType>
      OFCrossSectionTraceMultiScaleFilterType;

    typename OFCrossSectionTraceMultiScaleFilterType::Pointer ofMultiscaleFilter = OFCrossSectionTraceMultiScaleFilterType::New();

    // setting parameters
    ofMultiscaleFilter->SetInput(inputImage);
    ofMultiscaleFilter->SetSigmaMinimum(sigmaMin);
    ofMultiscaleFilter->SetSigmaMaximum(sigmaMax);
    ofMultiscaleFilter->SetNumberOfSigmaSteps(nbSigmaSteps);
    ofMultiscaleFilter->SetFixedSigmaForOrientedFluxImage(fixedSigma);
    ofMultiscaleFilter->SetGenerateScaleOutput(false);
    ofMultiscaleFilter->SetGenerateOrientedFluxOutput(true);
    ofMultiscaleFilter->SetGenerateNPlus1DOrientedFluxOutput(false);
    ofMultiscaleFilter->SetGenerateNPlus1DOrientedFluxMeasureOutput(false);

    try{
        itk::TimeProbe timer;
        timer.Start();
        ofMultiscaleFilter->Update();
        timer.Stop();
        std::cout<<"Computation time:"<<timer.GetMean()<<std::endl;
    }
    catch(itk::ExceptionObject &e)
    {
        std::cerr << e << std::endl;
    }

    ScoreImageType::Pointer scoreImage = ofMultiscaleFilter->GetOutput();

    typedef itk::ImageFileWriter<ScoreImageType> ScoreImageWriter;
    auto writer = ScoreImageWriter::New();
    writer->SetInput(scoreImage);
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
