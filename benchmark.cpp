#include "bench_evaluation.h"
#include "utils.h"

#include <json/json.h>

#include <string>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <map>

#include "itkImageFileReader.h"
#include "itkBinaryThresholdImageFilter.h"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>


// opening ref image
using ImageType = itk::Image<double,3>;
using GroundTruthImageType = itk::Image<int,3>;
using MaskImageType = itk::Image<int,3>;


typedef itk::ImageFileReader<GroundTruthImageType> GroundTruthReaderType;
typedef itk::ImageFileReader<ImageType> ImageReaderType;
typedef itk::ImageFileReader<MaskImageType> MaskImageReaderType;

typedef itk::BinaryThresholdImageFilter<ImageType,GroundTruthImageType> ThresholdFilterType;

void launchScript(const std::string &commandLine,GroundTruthImageType::Pointer groundTruth,MaskImageType::Pointer maskImage,const std::string &outputName, VoxelsMap &vMap, MetricsMap &mMap)
{
  
  std::cout<<commandLine<<std::endl;
  // starting external algorithm
  system(commandLine.c_str());

  auto outputImage = readImage<ImageType>(outputName,false);
  std::cout<<"comparing output to ground truth....\n";
  // TODO do some stats here
  if( outputImage->GetLargestPossibleRegion().GetSize() != groundTruth->GetLargestPossibleRegion().GetSize() )
    {
      std::cout<<"output from program and groundTruth size does not match...No stats computed"<<std::endl;
      return;
    }
      
  // Computing roc curve for the image segmentation
  double bestThreshold = 0;
  double minDist = 1000;
  for(float i=1.0f; i>=0.0f;i-=0.1f)
  {
    // thresholding for all values ( keeping upper value and adding more incertainty as lower probabilities are accepted )
    auto tFilter = ThresholdFilterType::New();
    tFilter->SetInput( outputImage );
    tFilter->SetInsideValue( 1.0f );
    tFilter->SetOutsideValue( 0.0f );

    tFilter->SetLowerThreshold(i);
    tFilter->SetUpperThreshold(1);
    tFilter->Update();
    auto segmentationImage = tFilter->GetOutput();

    Eval<GroundTruthImageType,GroundTruthImageType,MaskImageType> eval(segmentationImage,groundTruth,maskImage);
    std::cout<<"true positive rate : " << eval.sensitivity() << "\n"
            << " false positive rate : " << 1.0f - eval.specificity() << "\n";
    
    // perfect qualifier (0,1), our segmentation (TPR,FPR)
    float euclideanDistance =  (eval.sensitivity()*eval.sensitivity()) + ( 1.0f - eval.specificity() ) * ( 1.0f - eval.specificity() );
    if( minDist >  euclideanDistance )
    {
      minDist = euclideanDistance;
      bestThreshold = i;
    }
    eval.print();
  }
  std::cout<<"best threshold from ROC:"<<bestThreshold<<std::endl;

  /* 
  
  vMap["TP"].push_back( eval.TP() );
  vMap["TN"].push_back( eval.TN() );
  vMap["FP"].push_back( eval.FP() );
  vMap["FN"].push_back( eval.FN() );

  mMap["Dice"].push_back( eval.dice() );
  mMap["MCC"].push_back( eval.matthewsCorrelation() );

  mMap["sensitivity"].push_back( eval.sensitivity() );
  mMap["accuracy"].push_back( eval.accuracy() );
  mMap["precision"].push_back( eval.precision() );
  mMap["specificity"].push_back( eval.specificity() );

  */
      
  std::cout<<"done"<<std::endl;
}

void printResults(VoxelsMap &vMap, MetricsMap &mMap)
{
  //
  int nbResults = vMap["TP"].size();

  for(int i=0; i<nbResults; i++)
  {
    std::cout<<"algorithm : "<<i<<"\n"
	   <<"TP (1,1) :"<<vMap["TP"][i]<<std::endl
	   <<"TN (0,0):"<<vMap["TN"][i]<<std::endl
	   <<"FP (1,0):"<<vMap["FP"][i]<<std::endl
	   <<"FN (0,1):"<<vMap["FN"][i]<<std::endl<<std::endl
	   <<"Sensitivity:"<<mMap["sensitivity"][i]<<std::endl
	   <<"Specificity:"<<mMap["specificity"][i]<<std::endl
	   <<"Precision:"<<mMap["precision"][i]<<std::endl
	   <<"Accuracy:"<<mMap["accuracy"][i]<<std::endl
	   <<"Matthews correlation:"<<mMap["MCC"][i]<<std::endl
	   <<"Dice:"<<mMap["Dice"][i]<<std::endl<<std::endl;
  }
}

int main(int argc, char** argv)
{
  // ------------------
  // Reading arguments 
  // ------------------

  namespace po = boost::program_options;
  // parsing arguments
  po::options_description general_opt("Allowed options are ");
  general_opt.add_options()
    ("help,h", "display this message")
    ("input,i",po::value<std::string>(),"Input image ")
    ("inputIsDicom,n",po::value<bool>()->default_value(false),"specify dicom input")
    ("groundTruth,g", po::value<std::string>(), "GroundTruth : input img" )
    ("gtIsDicom,t",po::value<bool>()->default_value(false),"specify dicom groundTruth")
    ("mask,m", po::value<std::string>(), "mask : mask image")
    ("maskIsDicom,a",po::value<bool>()->default_value(false),"specify dicom input")
    ("parametersFile,p",po::value<std::string>()->default_value("parameters.json"),"ParameterFile : input json file");
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
      std::cout<<"\n Usage : benchmark [input image] [ground truth] [optionnal parameterFile] \n\n"
	       << " ground truth : ground truth image (.nii float/double type)\n"
	       <<" input : input image for benchmark (.nii float/double type)\n"
         <<" mask : mask image for the benchmarl (.nii int type) \n"
	       <<" parametersFile : input Json parameter file. If none provided, default is parameters.json\n"<<std::endl;
    
      return 0;
    }

  std::string inputFileName = vm["input"].as<std::string>();
  std::string groundTruthFileName = vm["groundTruth"].as<std::string>();
  std::string parameterFileName = vm["parametersFile"].as<std::string>();
  std::string maskFileName = vm["mask"].as<std::string>();
  bool isMaskDicom = vm["maskIsDicom"].as<bool>();
  bool isGTDicom = vm["gtIsDicom"].as<bool>();
  bool isInputDicom = vm["inputIsDicom"].as<bool>();

  //if( isMaskDicom)
    std::cout << "mask is dicom directory : "<<isMaskDicom<<std::endl;
  //if( isGTDicom)
    std::cout<< "ground truth is dicom directory : "<<isGTDicom<<std::endl;
  //if( isInputDicom )
    std::cout<< "input is dicom directory : "<<isInputDicom <<std::endl;

  // -----------------
  // Reading JSON file
  // -----------------

  std::ifstream configFile(parameterFileName,std::ifstream::binary);
  if(!configFile.is_open())
    {
      std::cout<<"couldn't find parameter.json file...aborting"<<std::endl;
      exit(-1);
    }
      
  std::cout<<"opening JSON file...";
  Json::Value root;
  configFile >> root;
  configFile.close();
  std::cout<<"done\n"<<std::endl;

  // ------------------------
  // Reading GroundTruthImage
  // ------------------------

  auto groundTruth = readImage<GroundTruthImageType>(groundTruthFileName,isGTDicom);

  // ------------------------
  // Reading MaskImage
  // ------------------------

  auto maskImage = readImage<MaskImageType>(maskFileName,isMaskDicom);

  // ------------------------
  // Initializing dataMaps
  // -------------------------

    VoxelsMap vMap;
    MetricsMap mMap;

    vMap["TP"] = std::vector<long>();
    vMap["TN"] = std::vector<long>();
    vMap["FP"] = std::vector<long>();
    vMap["FN"] = std::vector<long>();

    mMap["Dice"] = std::vector<double>();
    mMap["MCC"] = std::vector<double>();

    mMap["sensitivity"] = std::vector<double>();
    mMap["accuracy"] = std::vector<double>();
    mMap["precision"] = std::vector<double>();
    mMap["specificity"] = std::vector<double>();

  // ------------------------
  // Starting benchmark
  // -------------------------

  std::cout<<"starting benchmark"<<std::endl;
  std::cout<<"------------------"<<std::endl;

  Json::Value::Members algoNames = root.getMemberNames();
  int nbAlgorithms = 0;
  for (auto &algoName : algoNames)
  {
    std::cout << "Algorithm n°" << nbAlgorithms << " " << algoName << std::endl;
    nbAlgorithms++;

    const Json::Value algo = root[algoName];

    if (algo.isArray()) // the algorithm contains several sets of parameters
    {
      for (auto &p : algo)
      {
        const std::string outputName = p["Output"].asString();
        const Json::Value arguments = p["Arguments"];

        std::stringstream sStream;
        sStream << "./algorithms/" << algoName << " "
                << "--input"
                << " " << inputFileName << " "
                << "--output " << outputName << " ";

        for (auto &arg : arguments)
        {
          std::string m = arg.getMemberNames()[0]; // only one name in the array
          sStream << "--" << m << " " << arg[m].asString() << " ";
        }
        launchScript(sStream.str(), groundTruth, maskImage, outputName,vMap,mMap);
      }
    }
    else
    {
      const std::string outputName = algo["Output"].asString();
      const Json::Value arguments = algo["Arguments"];

      std::stringstream sStream;
      sStream << "./" << algoName << " "
              << "--input"
              << " " << inputFileName << " "
              << "--output " << outputName << " ";

      for (auto &arg : arguments)
      {
        std::string m = arg.getMemberNames()[0]; // only one name in the array
        sStream << m << " " << arg[m].asString() << " ";
      }
      launchScript(sStream.str(), groundTruth,maskImage, outputName, vMap, mMap);
    }
  }
printResults(vMap,mMap);
}
