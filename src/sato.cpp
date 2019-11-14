#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkHessian3DToVesselnessMeasureImageFilter.h"
#include "itkMultiScaleHessianBasedMeasureImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <string>

#include "utils.h"

int main( int argc, char* argv[] )
{
    bool isInputDicom;

    namespace po = boost::program_options;
    // parsing arguments
    po::options_description general_opt("Allowed options are ");
    general_opt.add_options()
    ("help,h", "display this message")
    ("input,i", po::value<std::string>(), "inputName : input img" )
    ("output,o", po::value<std::string>(), "ouputName : output img" )
    ("alpha1,a", po::value<float>()->default_value(0.5), "Sato's alpha1" )
    ("alpha2,b",po::value<float>()->default_value(0.5),"Sato's alpha2" )
    ("sigmaMin,m", po::value<float>(), "scale space sigma min")
    ("sigmaMax,M", po::value<float>(), "scale space sigma max")
    ("nbSigmaSteps,n",po::value<int>(),"nb steps sigma")
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
    float alpha = vm["alpha1"].as<float>();
    float beta = vm["alpha2"].as<float>();

    constexpr unsigned int Dimension = 3;
    using PixelType = float;
    using ImageType = itk::Image< PixelType, Dimension >;

    auto inputImage = vUtils::readImage<ImageType>(inputFile,isInputDicom);

    // Antiga vesselness operator

    using HessianPixelType = itk::SymmetricSecondRankTensor< double, Dimension >;
    using HessianImageType = itk::Image< HessianPixelType, Dimension >;
    using ObjectnessFilterType = itk::Hessian3DToVesselnessMeasureImageFilter<PixelType>;
    
    
    ObjectnessFilterType::Pointer objectnessFilter = ObjectnessFilterType::New();
    objectnessFilter->SetAlpha1( alpha );
    objectnessFilter->SetAlpha2( beta );

    using MultiScaleEnhancementFilterType = itk::MultiScaleHessianBasedMeasureImageFilter< ImageType, HessianImageType, ImageType >;
    MultiScaleEnhancementFilterType::Pointer multiScaleEnhancementFilter =  MultiScaleEnhancementFilterType::New();
    multiScaleEnhancementFilter->SetInput( inputImage );
    multiScaleEnhancementFilter->SetNonNegativeHessianBasedMeasure(true);
    multiScaleEnhancementFilter->SetHessianToMeasureFilter( objectnessFilter );
    multiScaleEnhancementFilter->SetSigmaStepMethodToLogarithmic();
    multiScaleEnhancementFilter->SetSigmaMinimum( sigmaMin );
    multiScaleEnhancementFilter->SetSigmaMaximum( sigmaMax );
    multiScaleEnhancementFilter->SetNumberOfSigmaSteps( nbSigmaSteps );

    // end Antiga vesselness operator
/*
    using OutputImageType = itk::Image< unsigned char, Dimension >;
    using RescaleFilterType = itk::RescaleIntensityImageFilter< ImageType, OutputImageType >;
    RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
    rescaleFilter->SetInput( multiScaleEnhancementFilter->GetOutput() );
*/
    using imageWriterType = itk::Image<PixelType,Dimension>;
    typedef  itk::ImageFileWriter< imageWriterType  > WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetInput( multiScaleEnhancementFilter->GetOutput() );
    writer->SetFileName( std::string(outputFile) );
    writer->Update();


    return EXIT_SUCCESS;
}
