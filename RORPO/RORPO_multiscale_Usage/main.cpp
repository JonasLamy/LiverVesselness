/* Copyright (C) 2014 Odyssee Merveille
odyssee.merveille@gmail.com

    This software is a computer program whose purpose is to compute RORPO.
    This software is governed by the CeCILL-B license under French law and
    abiding by the rules of distribution of free software.  You can  use,
    modify and/ or redistribute the software under the terms of the CeCILL-B
    license as circulated by CEA, CNRS and INRIA at the following URL
    "http://www.cecill.info".

    As a counterpart to the access to the source code and  rights to copy,
    modify and redistribute granted by the license, users are provided only
    with a limited warranty  and the software's author,  the holder of the
    economic rights,  and the successive licensors  have only  limited
    liability.

    In this respect, the user's attention is drawn to the risks associated
    with loading,  using,  modifying and/or developing or reproducing the
    software by the user in light of its specific status of free software,
    that may mean  that it is complicated to manipulate,  and  that  also
    therefore means  that it is reserved for developers  and  experienced
    professionals having in-depth computer knowledge. Users are therefore
    encouraged to load and test the software's suitability as regards their
    requirements in conditions enabling the security of their systems and/or
    data to be ensured and,  more generally, to use and operate it in the
    same conditions as regards security.

    The fact that you are presently reading this means that you have had
    knowledge of the CeCILL-B license and that you accept its terms.
*/

#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <string>
#include <vector>
#include <typeinfo>
#include <sstream>
#include <itkCommonEnums.h>
#include "CLI11.hpp"

#include "Image/Image_IO_ITK.hpp"

#include "Image/Image.hpp"
#include "RORPO/RORPO_multiscale.hpp"

typedef uint16_t u_int16_t;


// Split a string
std::vector<std::string> split(std::string str, char delimiter) {
  std::vector<std::string> internal;
  std::stringstream ss(str); // Turn the string into a stream.
  std::string tok;

  while(getline(ss, tok, delimiter)) {
    internal.push_back(tok);
  }
  return internal;
}


template<typename PixelType>
int RORPO_multiscale_usage(Image3D<PixelType>& image,
                std::string outputFile,
                std::vector<int>& scaleList,
                std::vector<int>& window,
                int nbCores,
                int dilationSize,
                int verbose,
                std::string maskPath)
{
    unsigned int dimz = image.dimZ();
    unsigned int dimy = image.dimY();
    unsigned int dimx= image.dimX();

    float  spacingX = image.spacingX();
    float  spacingY = image.spacingY();
    float  spacingZ = image.spacingZ();

    if (verbose){
        std::cout << "dimensions: [" << dimx << ", " << dimy << ", " << dimz << "]" << std::endl;
        std::cout << "spacing: [" << spacingX << ", " << spacingY << ", " << spacingZ << "]" << std::endl;
	}

    // ------------------ Compute input image intensity range ------------------

    std::pair<PixelType,PixelType> minmax = image.min_max_value();

    if (verbose){
        std::cout<< "Image intensity range: "<< (int)minmax.first << ", "
                 << (int)minmax.second << std::endl;
        std::cout<<std::endl;
	}

    // -------------------------- mask Image -----------------------------------

    Image3D<uint8_t> mask;

    if (!maskPath.empty()) // A mask image is given
	{
        mask = Read_Itk_Image<uint8_t>(maskPath);

        if (mask.dimX() != dimx || mask.dimY() != dimy || mask.dimZ() != dimz){
            std::cerr<<"Size of the mask image (dimx= "<<mask.dimX()
                    <<" dimy= "<<mask.dimY()<<" dimz="<<mask.dimZ()
                   << ") is different from size of the input image"<<std::endl;
            return 1;
        }
    }

    // #################### Convert input image to char #######################

    if (window[2] == 1 || typeid(PixelType) == typeid(float) ||
            typeid(PixelType) == typeid(double))
    {
        if (minmax.first > (PixelType) window[0])
            window[0] = minmax.first;

        if (minmax.second < (PixelType) window[1])
            window[1] = minmax.second;

        if(verbose){
            std::cout<<"Convert image intensity range from: [";
            std::cout<<minmax.first<<", "<<minmax.second<<"] to [";
            std::cout<<window[0]<<", "<<window[1]<<"]"<<std::endl;
        }

        image.window_dynamic(window[0], window[1]);

        if(verbose)
            std::cout << "Convert image to uint8" << std::endl;

        minmax.first = 0;
        minmax.second = 255;
        Image3D<uint8_t> imageChar = image.copy_image_2_uchar();

        // Run RORPO multiscale
        Image3D<uint8_t> multiscale =
                RORPO_multiscale<uint8_t, uint8_t>(imageChar,
                                                   scaleList,
                                                   nbCores,
                                                   dilationSize,
                                                   verbose,
                                                   mask);

        // Write the result to nifti image
        Write_Itk_Image<uint8_t>( multiscale, outputFile );
    }

    // ################## Keep input image in PixelType ########################

    else {

        // ------------------------ Negative intensities -----------------------

        if (minmax.first < 0)
        {
            image -= minmax.first;

            if(verbose){
                std::cout << "Convert image intensity range from [";
                std::cout << (int)minmax.first << ", " << (int)minmax.second << "] to [";
                std::cout << "0" << ", " << (int)minmax.second - (int)minmax.first << "]"
                            << std::endl;
            }
        }

        // Run RORPO multiscale
        Image3D<PixelType> multiscale =
                RORPO_multiscale<PixelType, uint8_t>(image,
                                                    scaleList,
                                                    nbCores,
                                                    dilationSize,
                                                    verbose,
                                                    mask);

        // getting min and max from multiscale image
        PixelType min = 255;
        PixelType max = 0;
 
        for( PixelType &value:multiscale.get_data() )
        {
            if( value > max)
                max = value;
            if( value < min)
                min = value;
        }


        // normalize output
        Image3D<float> multiscale_normalized(multiscale.dimX(),
                                            multiscale.dimY(),
                                            multiscale.dimZ(),
                                            multiscale.spacingX(),
                                            multiscale.spacingY(),
                                            multiscale.spacingZ(),
                                            multiscale.originX(),
                                            multiscale.originY(),
                                            multiscale.originZ()
                                            );


        for(unsigned int i=0; i<multiscale.size();i++)
        {
            multiscale_normalized.get_data()[i] = (multiscale.get_data()[i])/(float)(max); //
        }

        if(verbose)
        {
            std::cout<<"converting output image intensity :"<<(int)min<<"-"<<(int)max<<" to [0;1]"<<std::endl;
        }

        // Write the result to nifti image
        Write_Itk_Image<float>(multiscale_normalized, outputFile);
        //Write_Itk_Image<PixelType>(multiscale, outputFile);
    }

    return 0;
} // RORPO_multiscale_usage


// Parse command line with docopt
static const char USAGE[] =
R"(RORPO_multiscale_usage.

    USAGE:
    RORPO_multiscale_usage --input=inputFile --output=OutputPath --scaleMin=MinScale --factor=F --nbScales=NBS [--window=min,max] [--core=nbCores] [--dilationSize=Size] [--mask=maskPath] [--verbose] [--series]

    Options:
         --core=<nbCores>      Number of CPUs used for RPO computation \
         --dilationSize=<Size> Size of the dilation for the noise robustness step \ 
         --window=min,max      Convert intensity range [min, max] of the intput \
                               image to [0,255] and convert to uint8 image\
                               (strongly decrease computation time).
         --mask=maskPath       Path to a mask for the input image \
                               (0 for the background; not 0 for the foreground).\
                               mask image type must be uint8.
         --verbose             Activation of a verbose mode.
         --dicom               Specify that <inputFile> is a DICOM image.
        )";


int main(int argc, char **argv)
{
  
  // parse command line using CLI ----------------------------------------------
  CLI::App app;
  app.description("Apply the Jerman algorithm");
  std::string inputFile ;
  std::string outputFile;
  float scaleMin;
  float factor;
  float tau {-0.75};
  int nbScales;
  bool isInputDicom {false};
  bool verbose {false};
  std::vector<int> window;
  int nbCores{1};
  int dilationSize = 3;
  std::string maskPath;
  bool dicom;
  
  int nbSigmaSteps;
  std::string maskFile;
  app.add_option("-i,--input,1", inputFile, "inputName : input img")
  ->required()
  ->check(CLI::ExistingFile);
  app.add_option("--dilationSize",dilationSize, "dilatation  size");
  app.add_option("--window", window, "Window size: 3 params: windows size 1; windows size 2; 0|1: 0=use window, 1: not use window ")
  ->expected(3);
  app.add_option("--scaleMin",scaleMin, "Min scale");

  app.add_option("--nbCores",nbCores, "nb Cores");
  app.add_option("--factor",scaleMin, "factor");
  app.add_option("--nbScales,-n", nbScales,  "nb scales");
  app.add_option("--output,-o",outputFile, "ouputName : output img");
  app.add_option("--tau,-t", tau, "Jerman's tau" ,true);
  app.add_option("--mask,-k",maskPath,"mask response by image")
  ->check(CLI::ExistingFile);
  app.add_flag("--verbose", verbose, "verboe");
  app.add_flag("--dicom", dicom, "dicom");
  
  app.get_formatter()->column_width(40);
  CLI11_PARSE(app, argc, argv);
  // END parse command line using CLI ----------------------------------------------
  
  


    // -------------------------- Scales computation ---------------------------

    std::vector<int> scaleList(nbScales);
    scaleList[0] = scaleMin;

    for (int i = 1; i < nbScales; ++i)
        scaleList[i] = int(scaleMin * pow(factor, i));

    if (verbose){
        std::cout<<"Scales : ";
        std::cout<<scaleList[0];
        for (int i = 1; i < nbScales; ++i)
            std::cout<<','<<scaleList[i];
    }

    // -------------------------- Read ITK Image -----------------------------
    Image3DMetadata imageMetadata = Read_Itk_Metadata(inputFile);

    // ---------------- Find image type and run RORPO multiscale ---------------
    int error;
    if (verbose){
        std::cout<<" "<<std::endl;
        std::cout << "------ INPUT IMAGE -------" << std::endl;
        std::cout << "Input image type: " << imageMetadata.pixelTypeString << std::endl;
    }

    if ( imageMetadata.nbDimensions != 3 ) {
        std::cout << "Error: input image dimension is " << imageMetadata.nbDimensions << " but should be 3 " << std::endl;
        return 1;
    }

    switch (imageMetadata.pixelType){
      case (uint)(itk::ImageIOBase::IOComponentEnum::UCHAR) :
        {
            Image3D<unsigned char> image = dicom?Read_Itk_Image_Series<unsigned char>(inputFile):Read_Itk_Image<unsigned char>(inputFile);
            error = RORPO_multiscale_usage<unsigned char>(image,
                                                    outputFile,
                                                    scaleList,
                                                    window,
                                                    nbCores,
                                                    dilationSize,
                                                    verbose,
                                                    maskPath);
            break;
        }
        case (uint)(itk::ImageIOBase::IOComponentEnum::CHAR):
        {
            Image3D<char> image = dicom?Read_Itk_Image_Series<char>(inputFile):Read_Itk_Image<char>(inputFile);
            error = RORPO_multiscale_usage<char>(image,
                                                    outputFile,
                                                    scaleList,
                                                    window,
                                                    nbCores,
                                                    dilationSize,
                                                    verbose,
                                                    maskPath);
            break;
        }
        case (uint)(itk::ImageIOBase::IOComponentEnum::USHORT):
        {
            Image3D<unsigned short> image = dicom?Read_Itk_Image_Series<unsigned short>(inputFile):Read_Itk_Image<unsigned short>(inputFile);
            error = RORPO_multiscale_usage<unsigned short>(image,
                                                    outputFile,
                                                    scaleList,
                                                    window,
                                                    nbCores,
                                                    dilationSize,
                                                    verbose,
                                                    maskPath);
            break;
        }
        case (uint)(itk::ImageIOBase::IOComponentEnum::SHORT):
        {
            Image3D<short> image = dicom?Read_Itk_Image_Series<short>(inputFile):Read_Itk_Image<short>(inputFile);
            error = RORPO_multiscale_usage<short>(image,
                                                    outputFile,
                                                    scaleList,
                                                    window,
                                                    nbCores,
                                                    dilationSize,
                                                    verbose,
                                                    maskPath);
            break;
        }
        case (uint)(itk::ImageIOBase::IOComponentEnum::UINT):
        {
            Image3D<unsigned int> image = dicom?Read_Itk_Image_Series<unsigned int>(inputFile):Read_Itk_Image<unsigned int>(inputFile);
            error = RORPO_multiscale_usage<unsigned int>(image,
                                                    outputFile,
                                                    scaleList,
                                                    window,
                                                    nbCores,
                                                    dilationSize,
                                                    verbose,
                                                    maskPath);
            break;
        }
        case (uint)(itk::ImageIOBase::IOComponentEnum::INT):
        {
            Image3D<int> image = dicom?Read_Itk_Image_Series<int>(inputFile):Read_Itk_Image<int>(inputFile);
            error = RORPO_multiscale_usage<int>(image,
                                                    outputFile,
                                                    scaleList,
                                                    window,
                                                    nbCores,
                                                    dilationSize,
                                                    verbose,
                                                    maskPath);
            break;
        }
        case (uint)(itk::ImageIOBase::IOComponentEnum::ULONG):
        {
            Image3D<unsigned long> image = dicom?Read_Itk_Image_Series<unsigned long>(inputFile):Read_Itk_Image<unsigned long>(inputFile);
            error = RORPO_multiscale_usage<unsigned long>(image,
                                                    outputFile,
                                                    scaleList,
                                                    window,
                                                    nbCores,
                                                    dilationSize,
                                                    verbose,
                                                    maskPath);
            break;
        }
        case (uint)(itk::ImageIOBase::IOComponentEnum::LONG):
        {
            Image3D<long> image = dicom?Read_Itk_Image_Series<long>(inputFile):Read_Itk_Image<long>(inputFile);
            error = RORPO_multiscale_usage<long>(image,
                                                    outputFile,
                                                    scaleList,
                                                    window,
                                                    nbCores,
                                                    dilationSize,
                                                    verbose,
                                                    maskPath);
            break;
        }
      case (uint)(itk::ImageIOBase::IOComponentEnum::ULONGLONG):
        {
            Image3D<unsigned long long> image = dicom?Read_Itk_Image_Series<unsigned long long>(inputFile):Read_Itk_Image<unsigned long long>(inputFile);
            error = RORPO_multiscale_usage<unsigned long long>(image,
                                                    outputFile,
                                                    scaleList,
                                                    window,
                                                    nbCores,
                                                    dilationSize,
                                                    verbose,
                                                    maskPath);
            break;
        }
        case (uint)(itk::ImageIOBase::IOComponentEnum::LONGLONG):
        {
            Image3D<long long> image = dicom?Read_Itk_Image_Series<long long>(inputFile):Read_Itk_Image<long long>(inputFile);
            error = RORPO_multiscale_usage<long long>(image,
                                                    outputFile,
                                                    scaleList,
                                                    window,
                                                    nbCores,
                                                    dilationSize,
                                                    verbose,
                                                    maskPath);
            break;
        }
        case (uint)(itk::ImageIOBase::IOComponentEnum::FLOAT
                    ):
        {
            Image3D<float> image = dicom?Read_Itk_Image_Series<float>(inputFile):Read_Itk_Image<float>(inputFile);
            error = RORPO_multiscale_usage<float>(image,
                                                    outputFile,
                                                    scaleList,
                                                    window,
                                                    nbCores,
                                                    dilationSize,
                                                    verbose,
                                                    maskPath);
            break;
        }
        case (uint)(itk::ImageIOBase::IOComponentEnum::DOUBLE):
        {
            Image3D<double> image = dicom?Read_Itk_Image_Series<double>(inputFile):Read_Itk_Image<double>(inputFile);
            error = RORPO_multiscale_usage<double>(image,
                                                    outputFile,
                                                    scaleList,
                                                    window,
                                                    nbCores,
                                                    dilationSize,
                                                    verbose,
                                                    maskPath);
            break;
        }
        case (uint)(itk::ImageIOBase::IOComponentEnum::UNKNOWNCOMPONENTTYPE):
        default:
            error = 1;
            std::cout << "Error: pixel type unknown." << std::endl;
            break;
    }
    return error;

}//end main
