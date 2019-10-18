#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkHessianToRuiZhangMeasureImageFilter.h"
#include "itkMultiScaleHessianBasedMeasureImageFilter.h"
#include "itkScalarImageKmeansImageFilter.h"
#include "itkSigmoidImageFilter.h"

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
    ("sigmaMin,m", po::value<float>(), "scale space sigma min")
    ("sigmaMax,M", po::value<float>(), "scale space sigma max")
    ("tau,t", po::value<float>()->default_value(0.75), "Jerman's tau" )
    ("nbSeeds,s",po::value<int>(),"number of kmean seeds")
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
      std::cout<<"\n Usage : ./RuiZhangVesselness --input=<inputname> --output=<outputName> \
                --tau=<tau> --sigmaMin=<sigmaMin> --sigmaMax=<sigmaMax> --nbSigmaSteps=<nbSigmaSteps> \n\n"
                << " inputName : Name of the input image\n"
                << " outputName : Name of the output image\n"
                << " tau : scale space normalization coefficient (between [0,1]) \n"
                << " sigmaMin : scale space minimum size \n"
		            << " sigmaMax : scale space maximum size \n"
                << " nbSigmaSteps : scale space length \n\n"
                <<"example : ./RuiZhangVesselness --input liver.nii --output result.nii --tau 0.75 --sigmaMin 0.3 --sigmaMax 5 --nbSigmaSteps 8 \n" << std::endl;
    
      return 0;
    }

    std::string inputFile = vm["input"].as<std::string>();
    std::string outputFile = vm["output"].as<std::string>();
    float sigmaMin = vm["sigmaMin"].as<float>();
    float sigmaMax = vm["sigmaMax"].as<float>();
    int nbSigmaSteps = vm["nbSigmaSteps"].as<int>();
    float tau = vm["tau"].as<float>();
    unsigned int nbClasses = 6; // 5 classes inside the liver, class 1 is background //vm["nbSeeds"].as<int>();

    constexpr unsigned int Dimension = 3;
    using PixelType = double;
    using ImageType = itk::Image< PixelType, Dimension >;

    typedef itk::ImageFileReader<ImageType> ReaderType;
    auto reader = ReaderType::New();
    reader->SetFileName( inputFile );
    auto img = reader->GetOutput();
    // Filtering image

    typedef itk::ScalarImageKmeansImageFilter<ImageType> KmeanFilterType;
    auto kMeansFilter = KmeanFilterType::New();
    kMeansFilter->SetInput( img );
    kMeansFilter->SetUseNonContiguousLabels(true);

    auto stats = itk::StatisticsImageFilter<ImageType>::New();
    stats->SetInput( img );
    stats->Update();
    
    double min = stats->GetMinimum(); // 0/4
    double max = stats->GetMaximum(); // 4/4
    
    std::cout<<"min seed: "<<min<<std::endl;

    kMeansFilter->AddClassWithInitialMean(min);
    double step = (max - min) / nbClasses;
    for(int i=1; i<nbClasses-1;i++)
    {
        kMeansFilter->AddClassWithInitialMean(step * i);
        std::cout<<"seed: "<<step*i<<std::endl;
    }
    kMeansFilter->AddClassWithInitialMean(max);

    std::cout<<"max seed: "<<max<<std::endl;

    kMeansFilter->Update();

    KmeanFilterType::ParametersType estimatedMeans = kMeansFilter->GetFinalMeans();
    for( unsigned int i=0;i<nbClasses;i++)
    {
        std::cout<<"cluster["<<i<<"]"<<std::endl;
        std::cout<<"estimated mean : "<<estimatedMeans[i]<<std::endl;
    }

    double alpha = (estimatedMeans[nbClasses-1] - estimatedMeans[nbClasses-2])/2.0;
    double beta  = (estimatedMeans[nbClasses-1] + estimatedMeans[nbClasses-1])/2.0;

    // filtering with sigmoid

    typedef itk::SigmoidImageFilter<ImageType,ImageType> SigmoidFilterType;
    auto sigmoidFilter = SigmoidFilterType::New();

    sigmoidFilter->SetOutputMaximum(1);
    sigmoidFilter->SetOutputMinimum(0);

    sigmoidFilter->SetAlpha(alpha);
    sigmoidFilter->SetBeta(beta);

    sigmoidFilter->SetInput( img );

    using HessianPixelType = itk::SymmetricSecondRankTensor< double, Dimension >;
    using HessianImageType = itk::Image< HessianPixelType, Dimension >;
    
    using OutputImageType = itk::Image< double, Dimension >;

    using RuiZhangFilterType = itk::HessianToRuiZhangMeasureImageFilter<HessianImageType, OutputImageType>;
    auto ruiZhangFilter = RuiZhangFilterType::New();
    ruiZhangFilter->SetTau(tau);

    using MultiScaleEnhancementFilterType = itk::MultiScaleHessianBasedMeasureImageFilter< ImageType, HessianImageType, OutputImageType >;
    MultiScaleEnhancementFilterType::Pointer multiScaleEnhancementFilter =  MultiScaleEnhancementFilterType::New();
    multiScaleEnhancementFilter->SetInput( reader->GetOutput() );
    multiScaleEnhancementFilter->SetHessianToMeasureFilter( ruiZhangFilter );
    multiScaleEnhancementFilter->SetSigmaStepMethodToLogarithmic();
    multiScaleEnhancementFilter->SetSigmaMinimum( sigmaMin );
    multiScaleEnhancementFilter->SetSigmaMaximum( sigmaMax );
    multiScaleEnhancementFilter->SetNumberOfSigmaSteps( nbSigmaSteps );

    // Saving image
    
    using imageWriterType = ImageType;
    typedef  itk::ImageFileWriter< imageWriterType  > WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetInput( sigmoidFilter->GetOutput() );
    writer->SetFileName( std::string(outputFile) );
    writer->Update();


    return EXIT_SUCCESS;
}
