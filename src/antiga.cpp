#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkHessianToObjectnessMeasureImageFilter.h"
#include "itkMultiScaleHessianBasedMeasureImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkMaskImageFilter.h"

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
    ("alpha,a", po::value<float>()->default_value(0.5), "Frangi's alpha" )
    ("beta,b",po::value<float>()->default_value(0.5),"Frangi's beta" )
    ("gamma,",po::value<float>()->default_value(5.0),"max vessel intensity")
    ("sigmaMin,m", po::value<float>(), "scale space sigma min")
    ("sigmaMax,M", po::value<float>(), "scale space sigma max")
    ("nbSigmaSteps,n",po::value<int>(),"nb steps sigma")
    ("inputIsDicom,d",po::bool_switch(&isInputDicom),"specify dicom input")
    ("mask,k",po::value<std::string>()->default_value(""),"mask response by image ( Warning !!!! Dummy option for now");

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
      std::cout<< general_opt << std::endl;
    
      return 0;
    }

    std::string inputFile = vm["input"].as<std::string>();
    std::string outputFile = vm["output"].as<std::string>();
    float sigmaMin = vm["sigmaMin"].as<float>();
    float sigmaMax = vm["sigmaMax"].as<float>();
    int nbSigmaSteps = vm["nbSigmaSteps"].as<int>();
    float alpha = vm["alpha"].as<float>();
    float beta = vm["beta"].as<float>();
    float gamma = vm["gamma"].as<float>();
    std::string maskFile = vm["mask"].as<std::string>();

    constexpr unsigned int Dimension = 3;
    using PixelType = float;
    using ImageType = itk::Image< PixelType, Dimension >;

    ImageType::Pointer image = vUtils::readImage<ImageType>(inputFile,isInputDicom);

    // Antiga vesselness operator

    using HessianPixelType = itk::SymmetricSecondRankTensor< double, Dimension >;
    using HessianImageType = itk::Image< HessianPixelType, Dimension >;
    using ObjectnessFilterType = itk::HessianToObjectnessMeasureImageFilter< HessianImageType, ImageType >;
    
    ObjectnessFilterType::Pointer objectnessFilter = ObjectnessFilterType::New();
    objectnessFilter->SetBrightObject( true );
    objectnessFilter->SetScaleObjectnessMeasure( false );
    objectnessFilter->SetAlpha( alpha );
    objectnessFilter->SetBeta( beta );
    objectnessFilter->SetGamma( gamma );

    using MultiScaleEnhancementFilterType = itk::MultiScaleHessianBasedMeasureImageFilter< ImageType, HessianImageType, ImageType >;
    MultiScaleEnhancementFilterType::Pointer multiScaleEnhancementFilter =  MultiScaleEnhancementFilterType::New();
    multiScaleEnhancementFilter->SetInput( image );
    multiScaleEnhancementFilter->SetHessianToMeasureFilter( objectnessFilter );
    multiScaleEnhancementFilter->SetSigmaStepMethodToLogarithmic();
    multiScaleEnhancementFilter->SetSigmaMinimum( sigmaMin );
    multiScaleEnhancementFilter->SetSigmaMaximum( sigmaMax );
    multiScaleEnhancementFilter->SetNumberOfSigmaSteps( nbSigmaSteps );

    // For antiga, no global parameters (like jerman's lambda_max)
    // we still have to mask the answer though

    using imageWriterType = itk::Image<PixelType,Dimension>;
    typedef  itk::ImageFileWriter< imageWriterType  > WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName( std::string(outputFile) );

    typedef itk::Image<uint8_t, Dimension> MaskImageType;
    MaskImageType::Pointer maskImage;
    if( !maskFile.empty() )
    {
      maskImage = vUtils::readImage<MaskImageType>(maskFile,isInputDicom);
    
      auto maskFilter = itk::MaskImageFilter<ImageType,MaskImageType>::New();
      maskFilter->SetInput( multiScaleEnhancementFilter->GetOutput() );
      maskFilter->SetMaskImage(maskImage);
      maskFilter->SetMaskingValue(0);
      maskFilter->SetOutsideValue(0);

      maskFilter->Update();
    
      writer->SetInput( maskFilter->GetOutput() );
    }
    else
    {
      writer->SetInput( multiScaleEnhancementFilter->GetOutput() );
    }

    writer->Update();
    
    return EXIT_SUCCESS;
}
