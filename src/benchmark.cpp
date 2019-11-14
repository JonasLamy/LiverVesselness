#include "benchmark.h"
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
    ("groundTruth,g", po::value<std::string>(), "GroundTruth : input img" )
    ("mask,m", po::value<std::string>(), "mask : mask image")
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


  using ImageType = itk::Image<double,3>;
  using GroundTruthImageType = itk::Image<int,3>;
  using MaskImageType = itk::Image<int,3>;

  using DicomImageType = itk::Image<int16_t,3>;
  using DicomGroundTruthImageType = itk::Image<uint8_t,3>;
  using DicomMaskImageType = itk::Image<uint8_t,3>;

  // reading groundTruthImage path, if it is Directory, we assume all inputs are full DICOM 16 bits
  // Mask is only useful for statistics during segmentation assessment, 
  // drawback : Computation is done on full image with ircad DB, advantages : No registration required, no heavy refactoring needed

  if( vUtils::isDir( groundTruthFileName ) ) // boolean choice for now, 0 is nifti & 1 is DICOM 
  {
    
    std::cout<<"Using dicom data...."<<std::endl;
    DicomGroundTruthImageType::Pointer groundTruth = vUtils::readImage<DicomGroundTruthImageType>(groundTruthFileName,true);
    DicomMaskImageType::Pointer maskImage = vUtils::readImage<DicomMaskImageType>(maskFileName,true);
    
    Benchmark<ImageType,DicomGroundTruthImageType,DicomMaskImageType> b(root,inputFileName,groundTruth,maskImage);
    b.SetDicomInput();
    b.run();
  }
  else
  {
    std::cout<<"Using NIFTI data...."<<std::endl;
    GroundTruthImageType::Pointer groundTruth = vUtils::readImage<GroundTruthImageType>(groundTruthFileName,false);
    MaskImageType::Pointer maskImage = vUtils::readImage<MaskImageType>(maskFileName,false);
    
    Benchmark<ImageType,GroundTruthImageType,MaskImageType> b(root,inputFileName,groundTruth,maskImage);
    b.SetNiftiInput();
    b.run();
  }
}
