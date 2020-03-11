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

// system depedant Unix include to create folders....
#ifdef __WIN32__ // assume windows
  #include <conio.h>
  #include <dir.h>
  #include <process.h>
  #include <stdio.h>
#else  // linux and mac os
  #include <sys/stat.h>
  #include <sys/types.h>
#endif


std::ofstream initCSVFile(std::string csvFileName)
{
  std::ofstream csvFileStream;
  std::cout<<"creating csv file :"<<csvFileName<<std::endl;
  csvFileStream.open(csvFileName, std::ios::out | std::ios::trunc); // if the file already exists, we discard content
  if( csvFileStream.is_open() )
  {
    csvFileStream <<"SerieName,Name,Threshold,TP,TN,FP,FN,sensitivity,specificity,precision,accuracy,Dice,MCC,Hausdorff"<<std::endl;
  } 
  else{ 
    throw "Error opening csv file....";
  }

  return csvFileStream;
}

int main(int argc, char** argv)
{
  // ------------------
  // Reading arguments 
  // ------------------
  namespace po = boost::program_options;
  // parsing arguments
  bool computeMetricsOnly = false;
  bool removeResultsVolumes = false;
  po::options_description general_opt("Allowed options are ");
  general_opt.add_options()
    ("help,h", "display this message")
    ("input,i",po::value<std::string>(),"Input image ")
    ("parametersFile,p",po::value<std::string>()->default_value("parameters.json"),"ParameterFile : input json file")
    ("computeMetricsOnly,c",po::bool_switch(&computeMetricsOnly),"if true, assume that algo outputs are already calculated")
    ("removeResultsVolumes,r",po::bool_switch(&removeResultsVolumes),"if true, assume that algo outputs are already calculated");
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
      std::cout<<"\n Usage : benchmark [input file] [optionnal parameterFile] \n\n"
	       <<" input : input file listing patient name, image,mask and ground truth\n"
	       <<" parametersFile : input Json parameter file. If none provided, default is parameters.json\n"<<std::endl;
    
      return 0;
    }

  std::string inputFileName = vm["input"].as<std::string>();
  std::string parameterFileName = vm["parametersFile"].as<std::string>();

  // -----------------
  // Reading JSON file
  // -----------------

  std::ifstream configFile(parameterFileName,std::ifstream::binary);
  if(!configFile.is_open())
    {
      std::cout<<"couldn't find "<<parameterFileName<<" file...aborting"<<std::endl;
      exit(-1);
    }
      
  std::cout<<"opening JSON file...";
  Json::Value root;
  configFile >> root;
  configFile.close();
  std::cout<<"done\n"<<std::endl;

  // -----------------
  // Reading inputFileList
  // -----------------

  std::ifstream f;
  f.open(inputFileName);
  if( !f.is_open() )
  {
    std::cout<<"couldn't open "<< inputFileName<<std::endl;
    return 2;
  };
  std::string patientName; // name of the folder
  std::string imgName; // name of the input image
  std::string maskName; // name of the liver mask
  std::string maskBifurcationsName; // name of the bifurcation mask
  std::string maskDilatedVesselsName; // name of the dilated vessels mask
  std::string gtName;

  std::string benchDir = "bench/";

  // making composed name for csv (inputFile + param file) to avoid overwritting results with same parameter files but different dataset.
  size_t posInput = inputFileName.find(".");
  int backSlashInput = inputFileName.find_last_of("/");

  size_t posParam = parameterFileName.find(".");
  int backSlashParam = parameterFileName.find_last_of("/");
  std::string benchName;

  if(backSlashParam)
  {
    if( backSlashInput)
      benchName = inputFileName.substr(backSlashInput+1,posInput-backSlashInput-1) + std::string("_") + parameterFileName.substr(backSlashParam+1,posParam-backSlashParam-1);
    else
      benchName = inputFileName.substr(0,posInput-backSlashInput-1) + std::string("_") + parameterFileName.substr (backSlashParam+1,posParam-backSlashParam-1);
  }
  else
  {
    if(backSlashInput)
      benchName = inputFileName.substr(backSlashInput+1,posInput-backSlashInput-1) + std::string("_") + parameterFileName.substr (0,posParam-backSlashParam-1);
    else
      benchName = inputFileName.substr(0,posInput-backSlashInput-1) + std::string("_") + parameterFileName.substr (0,posParam-backSlashParam-1);
  }

  std::string csvFileMaskLiver;
  std::string csvFileMaskBifurcation;
  std::string csvFileMaskDilatedVessels;

  csvFileMaskLiver = benchDir + benchName +".csv";
  csvFileMaskBifurcation = benchDir + benchName +"_bifurcations.csv";
  csvFileMaskDilatedVessels = benchDir + benchName +"_dilatedVessels.csv";


  
  // opening resultFileStream
  std::ofstream csvFileStreamMaskLiver = initCSVFile(csvFileMaskLiver);
  std::ofstream csvFileStreamMaskDilatedVessels = initCSVFile(csvFileMaskBifurcation);
  std::ofstream csvFileStreamMaskBifurcations = initCSVFile(csvFileMaskDilatedVessels);
  
  //creating root directory
  #ifdef __WIN32__
    mkdir("bench");
  #else
    int error = mkdir(benchDir.c_str(),S_IRWXG | S_IRWXO | S_IRWXU);
    if( errno != EEXIST)
      {
        std::cout<<"couldn't create root directory "<<benchDir.c_str()<<std::endl; 
        csvFileStreamMaskLiver.close();
        csvFileStreamMaskDilatedVessels.close();
        csvFileStreamMaskBifurcations.close();
        return 1;
      }
  #endif

  using ImageType = itk::Image<double,3>;
  using GroundTruthImageType = itk::Image<int,3>;
  using MaskImageType = itk::Image<int,3>;

  using DicomImageType = itk::Image<int16_t,3>;
  using DicomGroundTruthImageType = itk::Image<uint8_t,3>;
  using DicomMaskImageType = itk::Image<uint8_t,3>;
  
  while(std::getline(f,patientName))
  {
    std::getline(f,imgName);
    std::getline(f,maskName);
    std::getline(f,maskBifurcationsName);
    std::getline(f,maskDilatedVesselsName);
    std::getline(f,gtName);

    std::cout<<patientName<<std::endl;
    std::cout<<imgName<<std::endl;
    std::cout<<maskName<<std::endl;
    std::cout<<maskBifurcationsName<<std::endl;
    std::cout<<maskDilatedVesselsName<<std::endl;
    std::cout<<gtName<<std::endl;

    //creating root directory
    #ifdef __unix__
      mkdir( (benchDir+benchName).c_str(),S_IRWXG | S_IRWXO | S_IRWXU);
      if( errno != EEXIST)
      {
        std::cout<<"couldn't create directory " << (benchDir+benchName).c_str() << std::endl; 
        std::cout<<errno<<std::endl;
      }
      else{
        std::cout<<"creating directory "<< (benchDir+benchName).c_str()<<std::endl;
      }
    #else
      throw "Non Unix directory creation not supported";
    #endif

    //creating subdirectory with patient
    #ifdef __unix__
      mkdir( (benchDir+benchName+"/"+patientName).c_str(),S_IRWXG | S_IRWXO | S_IRWXU);
      if( errno != EEXIST)
      {
        std::cout<<"couldn't create directory " << (benchDir+benchName+"/"+patientName).c_str() << std::endl; 
        std::cout<<errno<<std::endl;
      }
      else{
        std::cout<<"creating directory "<< (benchDir+benchName+"/"+patientName).c_str()<<std::endl;
      }
    #else
      throw "Non Unix directory creation not supported";
    #endif

    // creating subdir for patient - best  * TODO work in progess *
    #ifdef __unix__
      mkdir( (benchDir+benchName+"/"+patientName+"/best").c_str(),S_IRWXG | S_IRWXO | S_IRWXU);
    #endif

    // reading groundTruthImage path, if it is Directory, we assume all inputs are full DICOM 16 bits
    // Mask is only useful for statistics during segmentation assessment, 
    // drawback : Computation is done on full image with ircad DB, advantages : No registration required, no heavy refactoring needed
    
    if( vUtils::isDir( gtName ) ) // boolean choice for now, 0 is nifti & 1 is DICOM 
    {
      
      std::cout<<"Using dicom groundTruth data...."<<std::endl;
      DicomGroundTruthImageType::Pointer groundTruth = vUtils::readImage<DicomGroundTruthImageType>(gtName,false);
      DicomMaskImageType::Pointer maskImage = vUtils::readImage<DicomMaskImageType>(maskName,false);
      DicomMaskImageType::Pointer maskBifurcationImage = vUtils::readImage<DicomMaskImageType>(maskBifurcationsName,false);
      DicomMaskImageType::Pointer maskDilatedVesselsImage = vUtils::readImage<DicomMaskImageType>(maskDilatedVesselsName,false);
      
      Benchmark<DicomImageType,DicomGroundTruthImageType,DicomMaskImageType> b(root,
                                                                              imgName,
                                                                              groundTruth,
                                                                              csvFileStreamMaskLiver,
                                                                              maskImage,
                                                                              csvFileStreamMaskDilatedVessels,
                                                                              maskDilatedVesselsImage,
                                                                              csvFileStreamMaskBifurcations,
                                                                              maskBifurcationImage);
      b.SetOutputDirectory(benchDir+benchName+"/"+patientName);
      b.SetPatientDirectory(benchName+"/"+patientName);
      b.SetDicomInput();
      b.SetComputeMetricsOnly(computeMetricsOnly);
      b.SetremoveResultsVolume(removeResultsVolumes);
      b.run();
    }
    else
    {
      std::cout<<"Using NIFTI groundtruth data...."<<std::endl;
      GroundTruthImageType::Pointer groundTruth = vUtils::readImage<GroundTruthImageType>(gtName,false);
      MaskImageType::Pointer maskImage = vUtils::readImage<MaskImageType>(maskName,false);
      MaskImageType::Pointer maskBifurcationImage = vUtils::readImage<MaskImageType>(maskBifurcationsName,false);
      MaskImageType::Pointer maskDilatedVesselsImage = vUtils::readImage<MaskImageType>(maskDilatedVesselsName,false);
      
      Benchmark<ImageType,GroundTruthImageType,MaskImageType> b(root,
                                                                imgName,
                                                                groundTruth,
                                                                csvFileStreamMaskLiver,
                                                                maskImage,
                                                                csvFileStreamMaskDilatedVessels,
                                                                maskDilatedVesselsImage,
                                                                csvFileStreamMaskBifurcations,
                                                                maskBifurcationImage);
      b.SetOutputDirectory(benchDir+benchName+"/"+patientName);
      b.SetPatientDirectory(benchName+"/"+patientName);
      b.SetNiftiInput();
      b.SetComputeMetricsOnly(computeMetricsOnly);
      b.SetremoveResultsVolume(removeResultsVolumes);
      b.run();
    }
    
    if( !f.is_open())
    {
      std::cout<<"we are doomed"<<std::endl;
    }
  }
  f.close();
  csvFileStreamMaskLiver.close();
  csvFileStreamMaskDilatedVessels.close();
  csvFileStreamMaskBifurcations.close();
}
