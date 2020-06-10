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

int createDirectory(std::string path)
{
  int error = mkdir(path.c_str(), S_IRWXG | S_IRWXU | S_IROTH | S_IXOTH);
  if (error)
  {
    if (errno != EEXIST)
    {
      std::cout << "directory creation error: " << errno << " " << path.c_str() << std::endl;
      throw 1;
    }
  }
  std::cout << "created directory :"<<path<< std::endl;
}

std::ofstream initCSVFile(std::string csvFileName)
{
  std::ofstream csvFileStream;
  csvFileStream.open(csvFileName, std::ios::out | std::ios::trunc); // if the file already exists, we discard content
  if( csvFileStream.is_open() )
  {
    csvFileStream <<"SerieName,VolumeName,Threshold,TP,TN,FP,FN,sensitivity,specificity,precision,accuracy,Dice,MCC,Hausdorff"<<std::endl;
  } 
  else{ 
    std::cout<<"error couldn't open csv file..."<<std::endl; // *TODO remove cout and do proper exceptions messages
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
  po::options_description general_opt("Allowed options are ");
  general_opt.add_options()
    ("help,h", "display this message")
    ("settingsFile,s",po::value<std::string>(),"settings Json File path");
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
      std::cout<< general_opt <<std::endl;
    
      return 0;
    }

  std::string configFileName = vm["settingsFile"].as<std::string>();

  // -----------------
  // Reading config JSON file
  // -----------------

  std::ifstream configFile(configFileName,std::ifstream::binary);
  if(!configFile.is_open())
    {
      std::cout<<"couldn't find "<<configFileName<<" file...aborting"<<std::endl;
      exit(-1);
    }
      
  std::cout<<"opening configuration file...";
  Json::Value root;
  configFile >> root;
  configFile.close();
  std::cout<<"done\n"<<std::endl
           <<"---------------------"<<std::endl;

  // display first two categories
  const Json::Value benchParameters = root["Settings"];
  std::string benchName = benchParameters["name"].asString();
  std::string benchPath = benchParameters["path"].asString();
  std::string inputVolumesListPath = benchParameters["inputVolumesList"].asString();
  std::string algorithmSetsPath = benchParameters["algorithmSets"].asString();
  std::string benchMaskType = benchParameters["maskType"].asString();
  bool removeResultsVolumes = benchParameters["removeResultsVolumes"].asBool();
  bool computeMetricsOnly = false; // feature comming soon
  int nbThresholds = benchParameters["nbThresholds"].asInt();

  std::cout<< "Benchmark Name : "<<benchName << std::endl
            <<"Benchmark Path : "<<benchPath << std::endl
            <<"Input volumes list : "<<inputVolumesListPath << std::endl
            <<"Algorithms set list : "<<algorithmSetsPath << std::endl
            <<"NbThresholds : "<<nbThresholds << std::endl
            <<"Remove volumes :"<< removeResultsVolumes << std::endl
            <<"Mask type :"<< benchMaskType << std::endl;



  // creating benchmark root directory
  std::string benchDir = benchPath + "/" + benchName;

#ifdef __WIN32__
  std::cout << "Windows not supported" << std::endl;
  throw;
  //mkdir("bench");
#else
  createDirectory(benchPath);
  createDirectory(benchDir);
  createDirectory( benchDir+"/csv" );
#endif

  std::string csvFileMask = benchDir + "/csv/" + benchName +".csv";
  std::string csvFileMaskBifurcation = benchDir + "/csv/" + benchName +"_bifurcations.csv";
  std::string csvFileMaskDilatedVessels = benchDir + "/csv/" + benchName +"_dilatedVessels.csv";
  
  std::cout<< "---------------------" << std::endl;
  std::cout<< "csv files" << std::endl;

  std::cout<<"Opening main csv file :"<<csvFileMask<<std::endl;
  // opening resultFileStream
  std::ofstream csvFileStreamMask = initCSVFile(csvFileMask);
  std::cout<<"Opening Bifurcation csv file :"<<csvFileMaskBifurcation<<std::endl;
  std::ofstream csvFileStreamMaskBifurcations = initCSVFile(csvFileMaskBifurcation);
  std::cout<<"Opening dilated vessels csv file :"<<csvFileMaskDilatedVessels<<std::endl;  
  std::ofstream csvFileStreamMaskDilatedVessels = initCSVFile(csvFileMaskDilatedVessels);

  // -----------------
  // Reading algorithms parameters sets
  // -----------------

  std::ifstream algorithmSetsFile(algorithmSetsPath,std::ifstream::binary);
  if(!algorithmSetsFile.is_open())
    {
      std::cout<<"couldn't find "<<algorithmSetsPath<<" file...aborting"<<std::endl;
      exit(-1);
    }
      
  std::cout<<"opening algorithms sets file...";
  algorithmSetsFile >> root;
  algorithmSetsFile.close();
  std::cout<<"done\n"<<std::endl
           <<"---------------------"<<std::endl;

  // -----------------
  // Reading inputFileList
  // -----------------

  std::ifstream f;
  f.open(inputVolumesListPath);
  if( !f.is_open() )
  {
    std::cout<<"couldn't open "<< inputVolumesListPath <<std::endl;
    return 2;
  };
  std::string patientName; // name of the folder
  std::string imgName; // name of the input image
  std::string maskName; // name of the liver mask
  std::string maskBifurcationsName; // name of the bifurcation mask
  std::string maskDilatedVesselsName; // name of the dilated vessels mask
  std::string gtName;

  // Images types
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
#ifdef __WIN32__
      std::cout<<"Non Unix directory creation not supported 1"<<std::endl;
      throw;
#else
      createDirectory( benchDir+"/"+patientName );
#endif
      
/*
    // creating subdir for patient - best  * TODO work in progess *
#ifdef __WIN32__
      std::cout<<"Non Unix directory creation not supported 2"<<std::endl;
      throw;
#else
      mkdir( (benchDir+"/"+patientName+"/best").c_str(),S_IRWXG | S_IRWXU | S_IROTH | S_IXOTH);
#endif
*/

  // dealing with potential masking
  std::string maskOrganName("");
  if(benchMaskType == "Organ")
  {
    maskOrganName = maskName;
  }
  if( benchMaskType == "DilatedVessels")
  {
    maskOrganName = maskDilatedVesselsName;
  }
  if( benchMaskType == "Bifurcations")
  {
    maskOrganName = maskBifurcationsName;  
  }

  std::cout<< "Mask : "<< maskOrganName << std::endl;
  

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
									       csvFileStreamMask,
									       maskImage,
									       csvFileStreamMaskDilatedVessels,
									       maskDilatedVesselsImage,
									       csvFileStreamMaskBifurcations,
									       maskBifurcationImage);
      b.SetOutputDirectory(benchDir+"/"+patientName);
      b.SetPatientDirectory(benchName+"/"+patientName);
      b.SetDicomInput();
      b.SetComputeMetricsOnly(computeMetricsOnly);
      b.SetremoveResultsVolume(removeResultsVolumes);
      b.SetMaskName(maskOrganName);
      b.SetNbThresholds(nbThresholds);
      
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
                                                                csvFileStreamMask,
                                                                maskImage,
                                                                csvFileStreamMaskDilatedVessels,
                                                                maskDilatedVesselsImage,
                                                                csvFileStreamMaskBifurcations,
                                                                maskBifurcationImage);
      b.SetOutputDirectory(benchDir+"/"+patientName);
      b.SetPatientDirectory(benchName+"/"+patientName);
      b.SetNiftiInput();
      b.SetComputeMetricsOnly(computeMetricsOnly);
      b.SetremoveResultsVolume(removeResultsVolumes);
      b.SetMaskName(maskOrganName);
      b.SetNbThresholds(nbThresholds);
      b.run();
    }
    
    if( !f.is_open())
    {
      std::cout<<"we are doomed"<<std::endl;
    }
  }
  
  f.close();
  csvFileStreamMask.close();
  csvFileStreamMaskDilatedVessels.close();
  csvFileStreamMaskBifurcations.close();
  
}
