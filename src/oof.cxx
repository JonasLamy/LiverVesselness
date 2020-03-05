/*
Author : Jonas lamy
Based on work of Turetken & Fethallah Benmansour
*/

#include "itkOptimallyOrientedFlux.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkTimeProbe.h"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "utils.h"

int main(int argc, char** argv)
{
    //********************************************************
    //                   Reading arguments
    //********************************************************
    bool isInputDicom;
 
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
    ("sigma,s",po::value<double>(),"sigma for smoothing")
    ("inputIsDicom,d",po::bool_switch(&isInputDicom),"specify dicom input");

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
      std::cout<<"\n Usage : ./OOF --input=<inputImg> --output=<inputImg> --sigmaMin=<sMin> --sigmaMax=<sMax> --nbSigmaSteps=<nbSteps> --sigma=<S> \n\n"
                << " inputImg : input image (nifti)\n"
                << " outputImg : output image (nifti)\n"
                << " sigmaMin : min scale space value \n"
                << " sigmaMax : max scale space value \n"
		<< " nbSigmaSteps : number of scales\n"
	        << " sigma : fixed sigma smoothing \n\n"
                <<"example : ./OOF --input liver.nii --output result.nii --sigmaMin 1 --sigmaMax 5 --nbSigmaSteps 5 \n" << std::endl;
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

    typedef double PixelType;
    typedef itk::Image<PixelType,maxDimension> InputImageType;

    InputImageType::Pointer inputImage = vUtils::readImage<InputImageType>(inputFile,isInputDicom);

    //********************************************************
    //                   Filter
    //********************************************************

    auto OOFfilter = itk::OptimallyOrientedFlux<InputImageType,InputImageType>::New();
    OOFfilter->SetInput(inputImage);
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

    //ScoreImageType::Pointer scoreImage = ofMultiscaleFilter->GetOutput();
    

    typedef itk::ImageFileWriter<InputImageType> ScoreImageWriter;
    auto writer = ScoreImageWriter::New();
    writer->SetInput( OOFfilter->GetOutput() );
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
