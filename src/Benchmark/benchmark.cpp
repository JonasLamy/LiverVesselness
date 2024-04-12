#include "benchmark.h"
#include "bench_evaluation.h"
#include "utils.h"

#include <json/json.h>

#include <string>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <map>

#include <filesystem>
#include <exception>

#include "itkImageFileReader.h"
#include "itkBinaryThresholdImageFilter.h"

#include "CLI11.hpp"


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

namespace fs = std::filesystem;

bool createDirectory(const fs::path& path)  
{
  bool isOk = false;

  try {
    bool isOk = fs::create_directories(path);
    std::cout << "created directories :" << path << std::endl;
  }
  catch (const fs::filesystem_error& e)
  {
    std::cout << "File hierarchy creation failed : " << e.path1() << "\n\t" << e.what() << std::endl;
    throw e; // Rethrow to crash the program
  }
  catch (const std::exception& e)
  {
    std::cout << "Something went wrong... " << e.what() << std::endl;
    throw e; // Rethrow to crash the program
  }

  return isOk;
}

std::ofstream initCSVFile(std::string csvFileName)
{
  std::ofstream csvFileStream;
  csvFileStream.open(csvFileName, std::ios::out | std::ios::trunc); // if the file already exists, we discard content
  if( csvFileStream.is_open() )
  { 
    // Note : In case new metrics are used, this line should be edited to match the output of operator << of the bench_evaluation class
    csvFileStream <<"SerieName,VolumeName,Threshold,TP,TN,FP,FN,sensitivity,specificity,precision,accuracy,Dice,MCC,snr,psnr"<<std::endl;
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
  // parse command line using CLI ----------------------------------------------
  CLI::App app;
  
  std::string configFileName;
  app.description("Apply benchmark from json file.");
  app.add_option("--settingsFile,-s", configFileName, "settings Json File path")
  ->required()
  ->check(CLI::ExistingFile);
  
  app.get_formatter()->column_width(40);
  CLI11_PARSE(app, argc, argv);
  // END parse command line using CLI ----------------------------------------------

    
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
  std::cout<<"done\n"<<std::endl;
  const Json::Value benchParameters = root["Settings"];
  // display first two categories
  std::string benchName = benchParameters["name"].asString();
  std::string benchPath = benchParameters["path"].asString();
  std::string inputVolumesListPath = benchParameters["inputVolumesList"].asString();
  std::string algorithmSetsPath = benchParameters["algorithmSets"].asString();
  const Json::Value benchMaskList = benchParameters["maskList"];
  std::string enhancementMask = benchParameters["enhancementMask"].asString();
  bool removeResultsVolumes = benchParameters["removeResultsVolumes"].asBool();
  bool rescaleFilters = benchParameters["rescaleFilters"].asBool();
  int nbThresholds = benchParameters["nbThresholds"].asInt();
  bool computeMetricsOnly = benchParameters["computeMetricsOnly"].asBool();
  

  
  
  
  
 
  std::cout<< "Benchmark Name : "<<benchName << std::endl
            <<"Benchmark Path : "<<benchPath << std::endl
            <<"Input volumes list : "<<inputVolumesListPath << std::endl
            <<"Algorithms set list : "<<algorithmSetsPath << std::endl
            <<"NbThresholds : "<<nbThresholds << std::endl
            <<"Remove volumes :"<< removeResultsVolumes << std::endl
            <<"Rescale filters :"<< rescaleFilters << std::endl
            <<"Mask type :"<< benchMaskList << std::endl;



  // creating benchmark root directories
  fs::path benchDir(benchPath);
  benchDir.append(benchName);

#ifdef __WIN32__
  std::cout << "Windows not supported" << std::endl;
  throw;
  //mkdir("bench");
#else
  createDirectory( benchDir / "csv" );
#endif

  std::cout<< "---------------------" << std::endl;
  std::cout<< "csv files" << std::endl;

  std::vector<std::string> csvFileMaskList;
  std::vector<std::ofstream> csvFileMaskStreamList;
  int enhancementMaskIndex = -1;
  int nbMasks = 0;
  for(auto &mask : benchMaskList)
  {
    if(enhancementMask == mask.asString() )
    {
      enhancementMaskIndex = nbMasks;
    }

    csvFileMaskList.push_back( (benchDir / "csv" / "").string() + benchName + "_" + mask.asString() + ".csv");
    csvFileMaskStreamList.push_back( initCSVFile(csvFileMaskList[nbMasks]) );
    std::cout<<"Opening csv file for mask : "<<mask.asString()<<std::endl;
    nbMasks++;
  }

  std::cout<<"enhancement mask: "<<enhancementMask<<std::endl;

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
      // image path
      std::getline(f,imgName);
      // Gt image
      std::getline(f,gtName);
      // masks path
      std::vector<std::string> maskPathList;
      std::string maskPath;
      for(int i=0; i<nbMasks;i++)
      {
        std::getline(f,maskPath);
        maskPathList.push_back(maskPath);
      }

      std::string enhancementMaskPath = "";
      if(enhancementMaskIndex >= 0)
      {
        enhancementMaskPath = maskPathList[enhancementMaskIndex];
      }
      
      std::cout<<patientName<<std::endl;
      std::cout<<imgName<<std::endl;
      std::cout<<gtName<<std::endl;

      for(int i=0; i<nbMasks;i++)
      {
        std::cout<<maskPathList[i]<<std::endl;
      }

      //creating root directory
#ifdef __WIN32__
      std::cout<<"Non Unix directory creation not supported 1"<<std::endl;
      throw;
#else
      createDirectory( benchDir / patientName );
#endif

    // reading groundTruthImage path, if it is Directory, we assume all inputs are full DICOM 16 bits
    // Mask is only useful for statistics during segmentation assessment, 
    // drawback : Computation is done on full image with ircad DB, advantages : No registration required, no heavy refactoring needed
        
    if( vUtils::isDir( gtName ) ) // boolean choice for now, 0 is nifti & 1 is DICOM 
    {
      
      std::cout<<"Using dicom groundTruth data...."<<std::endl;
      DicomGroundTruthImageType::Pointer groundTruth = vUtils::readImage<DicomGroundTruthImageType>(gtName,false);

      std::vector<DicomMaskImageType::Pointer> dicomMaskImageList;
      for(int i=0; i<nbMasks;i++)
      {
        dicomMaskImageList.push_back( vUtils::readImage<DicomMaskImageType>(maskPathList[i],true) );
      }
      
        Benchmark<DicomImageType,DicomGroundTruthImageType,DicomMaskImageType> b(root,
                                                                                imgName,
                                                                                groundTruth,
                                                                                dicomMaskImageList,
                                                                                csvFileMaskStreamList);
        b.SetOutputDirectory( (benchDir / patientName).string() );
        b.SetPatientDirectory( (fs::path(benchName) / patientName).string() );
        b.SetDicomInput();
        b.SetComputeMetricsOnly(computeMetricsOnly);
        b.SetRemoveResultsVolume(removeResultsVolumes);
        b.SetEnhancementMaskName(enhancementMaskPath);
        b.SetNbThresholds(nbThresholds);
        b.SetRescaleFilters(rescaleFilters);
        
        b.run();
    }
        else
      {
        std::cout<<"Using NIFTI groundtruth data...."<<std::endl;
        GroundTruthImageType::Pointer groundTruth = vUtils::readImage<GroundTruthImageType>(gtName,false);

        std::vector<MaskImageType::Pointer> maskImageList;
        for(int i=0; i<nbMasks;i++)
        {
          std::cout<<maskPathList[i]<<std::endl;
          maskImageList.push_back( vUtils::readImage<MaskImageType>(maskPathList[i],false) );
        }
        
        Benchmark<ImageType,GroundTruthImageType,MaskImageType> b(root,
                                                                  imgName,
                                                                  groundTruth,
                                                                  maskImageList,
                                                                  csvFileMaskStreamList);
        b.SetOutputDirectory( (benchDir / patientName).string() );
        b.SetPatientDirectory( (fs::path(benchName) / patientName).string() );
        b.SetNiftiInput();
        b.SetComputeMetricsOnly(computeMetricsOnly);
        b.SetRemoveResultsVolume(removeResultsVolumes);
        b.SetEnhancementMaskName(enhancementMaskPath);
        b.SetNbThresholds(nbThresholds);
        b.SetRescaleFilters(rescaleFilters);
        b.run();
      }
      
      if( !f.is_open())
      {
        std::cout<<"we are doomed"<<std::endl;
      }
  }
  
  f.close();

  for(int i=0;i<nbMasks;i++)
  {
    csvFileMaskStreamList[i].close();
  } 
  return 0;
}
