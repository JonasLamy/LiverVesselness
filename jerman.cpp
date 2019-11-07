#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkMultiScaleHessianBasedMeasureImageFilter.h"
#include "itkHessianToJermanMeasureImageFilter.h"

#include "itkStatisticsImageFilter.h"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <string>

int main( int argc, char* argv[] )
{
    namespace po = boost::program_options;
    // parsing arguments
    po::options_description general_opt("Allowed options are ");
    general_opt.add_options()
    ("help,h", "display this message")
    ("input,i", po::value<std::string>(), "inputName : input img" )
    ("output,o", po::value<std::string>(), "ouputName : output img" )
    ("tau,t", po::value<float>()->default_value(0.75), "Jerman's tau" )
    ("sigmaMin,m", po::value<float>(), "scale space sigma min")
    ("sigmaMax,M", po::value<float>(), "scale space sigma max")
    ("nbSigmaSteps,n",po::value<int>(),"nb steps sigma");

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
      std::cout<<"\n Usage : ./JermanVesselness --input=<inputname> --output=<outputName> \
                --tau=<tau> --sigmaMin=<sigmaMin> --sigmaMax=<sigmaMax> --nbSigmaSteps=<nbSigmaSteps> \n\n"
                << " inputName : Name of the input image\n"
                << " outputName : Name of the output image\n"
                << " tau : scale space normalization coefficient (between [0,1]) \n"
                << " sigmaMin : scale space minimum size \n"
		            << " sigmaMax : scale space maximum size \n"
                << " nbSigmaSteps : scale space length \n\n"
                <<"example : ./JermanVesselness --input liver.nii --output result.nii --tau 0.75 --sigmaMin 0.3 --sigmaMax 5 --nbSigmaSteps 8 \n" << std::endl;
    
      return 0;
    }

    std::string inputFile = vm["input"].as<std::string>();
    std::string outputFile = vm["output"].as<std::string>();
    float sigmaMin = vm["sigmaMin"].as<float>();
    float sigmaMax = vm["sigmaMax"].as<float>();
    int nbSigmaSteps = vm["nbSigmaSteps"].as<int>();
    float tau = vm["tau"].as<float>();

    constexpr unsigned int Dimension = 3;
    using PixelType = double;
    using ImageType = itk::Image< PixelType, Dimension >;

    typedef itk::ImageFileReader<ImageType> ReaderType;
    auto reader = ReaderType::New();
    reader->SetFileName( inputFile );

    // Antiga vesselness operator

    using HessianPixelType = itk::SymmetricSecondRankTensor< double, Dimension >;
    using HessianImageType = itk::Image< HessianPixelType, Dimension >;
    
    using OutputImageType = itk::Image< double, Dimension >;

    using JermanFilterType = itk::HessianToJermanMeasureImageFilter<HessianImageType, OutputImageType>;
    auto jermanFilter = JermanFilterType::New();
    jermanFilter->SetTau(tau);

    using MultiScaleEnhancementFilterType = itk::MultiScaleHessianBasedMeasureImageFilter< ImageType, HessianImageType, OutputImageType >;
    MultiScaleEnhancementFilterType::Pointer multiScaleEnhancementFilter =  MultiScaleEnhancementFilterType::New();
    multiScaleEnhancementFilter->SetInput( reader->GetOutput() );
    multiScaleEnhancementFilter->SetHessianToMeasureFilter( jermanFilter );
    //multiScaleEnhancementFilter->SetSigmaStepMethodToLogarithmic();
    multiScaleEnhancementFilter->SetSigmaMinimum( sigmaMin );
    multiScaleEnhancementFilter->SetSigmaMaximum( sigmaMax );
    multiScaleEnhancementFilter->SetNumberOfSigmaSteps( nbSigmaSteps );

    auto stats = itk::StatisticsImageFilter<OutputImageType>::New();
    stats->SetInput(multiScaleEnhancementFilter->GetOutput());
    stats->Update();

    std::cout<<"multiscale"<<std::endl<<"min"
    <<stats->GetMinimum()<<std::endl
    <<"mean:"<<stats->GetMean()<<std::endl
    <<"max:"<<stats->GetMaximum()<<std::endl;

    using imageWriterType = OutputImageType;
    typedef  itk::ImageFileWriter< imageWriterType  > WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetInput( multiScaleEnhancementFilter->GetOutput() );
    writer->SetFileName( std::string(outputFile) );
    writer->Update();


    return EXIT_SUCCESS;
}
