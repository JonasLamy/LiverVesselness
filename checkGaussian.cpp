#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkDiscreteGaussianImageFilter.h"
#include "itkDiscreteGaussianDerivativeImageFilter.h"

#include "itkRecursiveGaussianImageFilter.h"
#include "itkGradientRecursiveGaussianImageFilter.h"

int main( int argc, char** argv)
{   
     namespace po = boost::program_options;
    // parsing arguments
    po::options_description general_opt("Allowed options are ");
    general_opt.add_options()
    ("help,h", "display this message")
    ("input,i", po::value<std::string>(), "inputName : input img" );

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
        std::cout<<"provide input file"<<std::endl;
      return 0;
    }

    std::string inputFile = vm["input"].as<std::string>();



    constexpr unsigned int Dimension = 3;
    using PixelType = float;
    using ImageType = itk::Image< PixelType, Dimension >;

    typedef itk::ImageFileReader<ImageType> ReaderType;
    
    using imageWriterType = ImageType;
    typedef  itk::ImageFileWriter< imageWriterType  > WriterType;
    
    auto reader = ReaderType::New();
    reader->SetFileName( inputFile );
    auto img  = reader->GetOutput();

    using DiscreteFilterType = itk::DiscreteGaussianImageFilter<ImageType,ImageType>;
    using DiscreteDerivativeFilterType = itk::DiscreteGaussianDerivativeImageFilter<ImageType,ImageType>;
    using RecursiveFilterType = itk::RecursiveGaussianImageFilter<ImageType,ImageType>;
    using GradientRecusiveFilterType = itk::GradientRecursiveGaussianImageFilter<ImageType,ImageType>;
     
    auto discreteFilter = DiscreteFilterType::New();
    auto discreteDerivativeFilter = DiscreteDerivativeFilterType::New();
    auto recursiveFilter = RecursiveFilterType::New();
    auto recursiveDerivativeFilter = RecursiveFilterType::New();
    auto gradientRecursiveFilter = GradientRecusiveFilterType::New();

    discreteFilter->SetInput(img);
    discreteFilter->SetVariance(1.0);

    recursiveFilter->SetInput(img);
    recursiveFilter->SetSigma(1.0);
    
    discreteDerivativeFilter->SetInput(img);
    discreteDerivativeFilter->SetUseImageSpacing(false);
    discreteDerivativeFilter->SetOrder(1);
    discreteDerivativeFilter->SetVariance(1.0);

    recursiveDerivativeFilter->SetInput(img);
    recursiveDerivativeFilter->SetOrder(RecursiveFilterType::FirstOrder);
    recursiveDerivativeFilter->SetSigma(1.0);

    gradientRecursiveFilter->SetSigma(1.0);
    gradientRecursiveFilter->SetInput(img);

    WriterType::Pointer writer = WriterType::New();
    writer->SetInput( discreteFilter->GetOutput() );
    writer->SetFileName( std::string("discreteG.nii") );
    writer->Update();
    
    writer->SetInput( discreteDerivativeFilter->GetOutput() );
    writer->SetFileName( std::string("discreteDerivativeG.nii") );
    writer->Update();

    writer->SetInput( recursiveFilter->GetOutput() );
    writer->SetFileName( std::string("recursiveG.nii") );
    writer->Update();

    writer->SetInput( recursiveDerivativeFilter->GetOutput() );
    writer->SetFileName( std::string("recursiveDerivativeG.nii") );
    writer->Update();

    writer->SetInput( gradientRecursiveFilter->GetOutput() );
    writer->SetFileName( std::string("gradientRecursiveG.nii") );
    writer->Update();

    return EXIT_SUCCESS;
}