//**********************************************************
//Copyright 2012 Engin Turetken & Fethallah Benmansour
//
//Licensed under the Apache License, Version 2.0 (the "License");
//you may not use this file except in compliance with the License.
//You may obtain a copy of the License at
//
//http://www.apache.org/licenses/LICENSE-2.0
//
//Unless required by applicable law or agreed to in writing, software
//distributed under the License is distributed on an "AS IS" BASIS,
//WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//See the License for the specific language governing permissions and
//limitations under the License.
//**********************************************************

#if defined(_MSC_VER)
#pragma warning(disable : 4786)
#endif

#include "itkMultiScaleTubularityMeasureImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkShiftScaleImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImage.h"
#include "itkExpImageFilter.h"

#include "itkOrientedFluxCrossSectionTraceMeasure.h"

const unsigned int maxDimension = 3;

template <class TInputPixel, unsigned int VDimension>
int Execute(int argc, char *argv[]);

// Get the PixelType, the ComponentType and the number of dimensions
// from the fileName

void GetImageType(std::string fileName, itk::ImageIOBase::IOPixelType &pixelType, itk::ImageIOBase::IOComponentType &componentType, unsigned int &noOfDimensions)
{
  typedef itk::Image<unsigned char, maxDimension> ImageType;
  typedef itk::ImageFileReader<ImageType> ReaderType;

  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(fileName);
  reader->UpdateOutputInformation();

  pixelType = reader->GetImageIO()->GetPixelType();
  componentType = reader->GetImageIO()->GetComponentType();
  noOfDimensions = reader->GetImageIO()->GetNumberOfDimensions();
}

void Usage(char *argv[])
{
  std::cerr << "Usage: " << std::endl
            << argv[0] << std::endl
            << " <input image file>" << std::endl
            << " <output tubularity score image file> " << std::endl
            << " <generate (N+1)-D scale space tubularity score image (yes=1/no=0)> <output (N+1)-D tubularity score image file> " << std::endl
            << " <generate Oriented Flux matrix image (yes=1/no=0)> <output Oriented Flux matrix image file> " << std::endl
            << " <generate (N+1)-D Oriented Flux matrix image (yes=1/no=0)> <output (N+1)-D Oriented Flux matrix image file> " << std::endl
            << " <generate scale image (yes=1/no=0)> <output scale (sigma) image file> " << std::endl
            << " <sigma min> <sigma max> <number of scales> " << std::endl
            << " <fixed sigma for smoothing>" << std::endl
            << " <object relative grey level (bright=1/dark=0)> " << std::endl
            << " <Take exponential of score image (yes=1/no=0)> " << std::endl
            << " < if Take ponential of score image == 1>" << std::endl
            << "			< Maximal Contrast Ratio>" << std::endl
            << std::endl
            << std::endl;
}

// Check the arguments and try to parse the input image.
int main(int argc, char *argv[])
{
  if (argc < 16)
  {
    Usage(argv);
    return EXIT_FAILURE;
  }

  itk::ImageIOBase::IOPixelType pixelType;
  itk::ImageIOBase::IOComponentType componentType;
  unsigned int noOfDimensions;

  try
  {
    GetImageType(argv[1], pixelType, componentType, noOfDimensions);

    switch (noOfDimensions)
    {
    case 2:
      switch (componentType)
      {
      case itk::ImageIOBase::UCHAR:
        return Execute<unsigned char, 2>(argc, argv);
        break;
        //				case itk::ImageIOBase::CHAR:
        //					return Execute<char, 2>( argc, argv );
        //					break;
      case itk::ImageIOBase::USHORT:
        return Execute<unsigned short, 2>(argc, argv);
        break;
        //				case itk::ImageIOBase::SHORT:
        //					return Execute<short, 2>( argc, argv );
        //					break;
        //				case itk::ImageIOBase::UINT:
        //					return Execute<unsigned int, 2>( argc, argv );
        //					break;
        //				case itk::ImageIOBase::INT:
        //					return Execute<int, 2>( argc, argv );
        //					break;
        //				case itk::ImageIOBase::ULONG:
        //					return Execute<unsigned long, 2>( argc, argv );
        //					break;
        //				case itk::ImageIOBase::LONG:
        //					return Execute<long, 2>( argc, argv );
        //					break;
        //				case itk::ImageIOBase::DOUBLE:
        //					return Execute<double, 2>( argc, argv );
        //					break;
        //				case itk::ImageIOBase::FLOAT:
        //					return Execute<float, 2>( argc, argv );
        //					break;
      case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
      default:
        std::cout << "Unknown pixel component type" << std::endl;
        break;
      }
      break;
    case 3:
      switch (componentType)
      {
      case itk::ImageIOBase::UCHAR:
        return Execute<unsigned char, 3>(argc, argv);
        break;
        //				case itk::ImageIOBase::CHAR:
        //					return Execute<char, 3>( argc, argv );
        //					break;
      case itk::ImageIOBase::USHORT:
        return Execute<unsigned short, 3>(argc, argv);
        break;
        //				case itk::ImageIOBase::SHORT:
        //					return Execute<short, 3>( argc, argv );
        //					break;
        //				case itk::ImageIOBase::UINT:
        //					return Execute<unsigned int, 3>( argc, argv );
        //					break;
        //				case itk::ImageIOBase::INT:
        //					return Execute<int, 3>( argc, argv );
        //					break;
        //				case itk::ImageIOBase::ULONG:
        //					return Execute<unsigned long, 3>( argc, argv );
        //					break;
        //				case itk::ImageIOBase::LONG:
        //					return Execute<long, 3>( argc, argv );
        //					break;
        //				case itk::ImageIOBase::DOUBLE:
        //					return Execute<double, 3>( argc, argv );
        //					break;
        //				case itk::ImageIOBase::FLOAT:
        //					return Execute<float, 3>( argc, argv );
        //					break;
      case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
      default:
        std::cout << "Unknown pixel component type" << std::endl;
        break;
      }
      break;
    default:
      std::cout << std::endl;
      std::cout << "ERROR: Only dimensions 2D and 3D are supported currently. "
                << "Add the routines to extend it." << std::endl;
      break;
    }
  }
  catch (itk::ExceptionObject &excep)
  {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

// Main code goes here!
template <class TInputPixel, unsigned int VDimension>
int Execute(int argc, char *argv[])
{
  // Define the dimension of the images
  const unsigned int Dimension = VDimension;

  // Typedefs
  typedef TInputPixel InputPixelType;
  typedef itk::Image<InputPixelType, Dimension> InputImageType;

  typedef float OutputPixelType;
  typedef itk::Image<OutputPixelType, Dimension> OutputImageType;
  typedef itk::Image<OutputPixelType, Dimension + 1> OutputScaleSpaceImageType;

  typedef itk::ImageFileReader<InputImageType> FileReaderType;
  typedef itk::ImageFileWriter<OutputImageType> FileWriterType;
  typedef itk::ImageFileWriter<OutputScaleSpaceImageType> ScaleSpaceImageFileWriterType;

  typedef float OFPixelScalarType;
  typedef itk::SymmetricSecondRankTensor<
      OFPixelScalarType, Dimension>
      OFPixelType;

  typedef itk::Image<OFPixelType, Dimension> OFImageType;
  typedef itk::Image<OFPixelType, Dimension + 1> NPlus1DOFImageType;

  typedef float ScalePixelType;
  typedef itk::Image<ScalePixelType, Dimension> ScaleImageType;

  typedef itk::ImageFileWriter<OFImageType> OFFileWriterType;
  typedef itk::ImageFileWriter<NPlus1DOFImageType> NPlus1DOFFileWriterType;
  typedef itk::ImageFileWriter<ScaleImageType> ScaleFileWriterType;

  typedef itk::ShiftScaleImageFilter<OutputImageType, OutputImageType> ShiftScaleFilterType;
  typedef itk::MinimumMaximumImageCalculator<OutputImageType> MinMaxCalculatorType;
  typedef itk::ShiftScaleImageFilter<OutputScaleSpaceImageType, OutputScaleSpaceImageType> ShiftScaleFilterForScaleSpaceImageType;
  typedef itk::MinimumMaximumImageCalculator<OutputScaleSpaceImageType> MinMaxCalculatorForScaleSpaceImageType;
  typedef itk::ExpImageFilter<OutputImageType, OutputImageType> ExpFilterType;
  typedef itk::ExpImageFilter<OutputScaleSpaceImageType, OutputScaleSpaceImageType> ScaleSpaceExpFilterType;

  typedef itk::OrientedFluxCrossSectionTraceMeasureFilter<OFImageType, OutputImageType> OFCrossSectionTraceObjectnessFilterType;

  typedef itk::MultiScaleTubularityMeasureImageFilter<InputImageType,
                                                      OFImageType,
                                                      ScaleImageType,
                                                      OFCrossSectionTraceObjectnessFilterType,
                                                      OutputImageType>
      OFCrossSectionTraceMultiScaleFilterType;

  // Parse the input arguments.
  unsigned int argumentOffset = 1;
  std::string inputImageFilePath = argv[argumentOffset++];
  std::string outputTubularityScoreImageFilePath = argv[argumentOffset++];

  bool generateScaleSpaceTubularityScoreImage = (bool)atoi(argv[argumentOffset++]);
  std::string outputScaleSpaceTubularityScoreImageFilePath = argv[argumentOffset++];

  bool generateOFMatrixImage = (bool)atoi(argv[argumentOffset++]);
  std::string outputOFMatrixImageFilePath = argv[argumentOffset++];

  bool generateNPlus1DOFMatrixImage = (bool)atoi(argv[argumentOffset++]);
  std::string outputNPlus1DOFMatrixImageFilePath = argv[argumentOffset++];

  bool generateScaleImage = (bool)atoi(argv[argumentOffset++]);
  std::string outputScaleImageFilePath = argv[argumentOffset++];

  double sigmaMin = atof(argv[argumentOffset++]);
  double sigmaMax = atof(argv[argumentOffset++]);
  unsigned int numberOfScales = atof(argv[argumentOffset++]);

  double fixedSigmaForOrientedFluxComputation = atof(argv[argumentOffset++]);
  bool brightObject = (bool)atoi(argv[argumentOffset++]);
  bool takeExponentialOfScoreImage = (bool)atoi(argv[argumentOffset++]);
  double maxContrastRatio = 10;

  if (takeExponentialOfScoreImage)
  {
    maxContrastRatio = atof(argv[argumentOffset++]);
  }

  // Read the input image.
  typename FileReaderType::Pointer imageReader = FileReaderType::New();
  imageReader->SetFileName(inputImageFilePath);
  try
  {
    imageReader->Update();
  }
  catch (itk::ExceptionObject &ex)
  {
    std::cout << ex << std::endl;
    return EXIT_FAILURE;
  }
  typename InputImageType::Pointer inputImage = imageReader->GetOutput();
  inputImage->DisconnectPipeline();

  typename OFCrossSectionTraceMultiScaleFilterType::Pointer ofMultiScaleFilter =
      OFCrossSectionTraceMultiScaleFilterType::New();
  ofMultiScaleFilter->SetBrightObject(brightObject);

  // Check the validity of some of the parameters.

  ofMultiScaleFilter->SetInput(inputImage);

  ofMultiScaleFilter->SetSigmaMinimum(sigmaMin);
  ofMultiScaleFilter->SetSigmaMaximum(sigmaMax);
  ofMultiScaleFilter->SetNumberOfSigmaSteps(numberOfScales);
  ofMultiScaleFilter->SetFixedSigmaForOrientedFluxImage(fixedSigmaForOrientedFluxComputation);
  ofMultiScaleFilter->SetGenerateScaleOutput(generateScaleImage);
  ofMultiScaleFilter->SetGenerateOrientedFluxOutput(generateOFMatrixImage);
  ofMultiScaleFilter->SetGenerateNPlus1DOrientedFluxOutput(generateNPlus1DOFMatrixImage);
  ofMultiScaleFilter->SetGenerateNPlus1DOrientedFluxMeasureOutput(generateScaleSpaceTubularityScoreImage);

  try
  {
    itk::TimeProbe timer;
    timer.Start();
    ofMultiScaleFilter->Update();
    timer.Stop();
    std::cout << "Total Computation time is " << timer.GetMean() << std::endl;
  }
  catch (itk::ExceptionObject &e)
  {
    std::cerr << e << std::endl;
  }

  // Writing the output (N+1)-D (i.e., scale-space) tubularity score image.
  double maxTubularityValue = -1e9;
  double minTubularityValue = 1e9;
  typename OutputScaleSpaceImageType::Pointer scaleSpaceTubularityScoreImage;
  if (generateScaleSpaceTubularityScoreImage)
  {
    if (takeExponentialOfScoreImage)
    {
      typename MinMaxCalculatorForScaleSpaceImageType::Pointer minMaxCalc =
          MinMaxCalculatorForScaleSpaceImageType::New();
      minMaxCalc->SetImage(ofMultiScaleFilter->GetNPlus1DImageOutput());
      minMaxCalc->Compute();
      maxTubularityValue = minMaxCalc->GetMaximum();
      minTubularityValue = minMaxCalc->GetMinimum();

      std::cout << "minTubularityValue " << minTubularityValue << std::endl;
      std::cout << "maxTubularityValue " << maxTubularityValue << std::endl;

      if (std::fabs(minMaxCalc->GetMaximum() - minMaxCalc->GetMinimum()) <
          itk::NumericTraits<float>::epsilon())
      {
        std::cerr << "Score image pixel values are all the same: ";
        std::cerr << minMaxCalc->GetMaximum() << std::endl;
        return EXIT_FAILURE;
      }

      double expFactor = std::log(maxContrastRatio) /
                         static_cast<double>(minMaxCalc->GetMaximum() - minMaxCalc->GetMinimum());

      std::cout << "expFactor " << expFactor << std::endl;

      // Carry out the exponential mapping.
      //std::cout << "Carrying out the exponential normalization of the score image." << std::endl;
      typename ShiftScaleFilterForScaleSpaceImageType::Pointer shiftScaleFilter = ShiftScaleFilterForScaleSpaceImageType::New();
      shiftScaleFilter->SetInput(ofMultiScaleFilter->GetNPlus1DImageOutput());
      shiftScaleFilter->SetShift(0.0);
      shiftScaleFilter->SetScale(expFactor);

      typename ScaleSpaceExpFilterType::Pointer expFilter = ScaleSpaceExpFilterType::New();
      expFilter->SetInput(shiftScaleFilter->GetOutput());
      expFilter->Update();
      scaleSpaceTubularityScoreImage = expFilter->GetOutput();
      //std::cout << "Exponential normalization done!" << std::endl;
    }
    else
    {
      scaleSpaceTubularityScoreImage = ofMultiScaleFilter->GetNPlus1DImageOutput();
    }

    typename ScaleSpaceImageFileWriterType::Pointer scaleSpaceWriter = ScaleSpaceImageFileWriterType::New();
    scaleSpaceWriter->SetFileName(outputScaleSpaceTubularityScoreImageFilePath);
    // scaleSpaceWriter->UseCompressionOn(); // Do not use compression since paraview can't read it.
    scaleSpaceWriter->SetInput(scaleSpaceTubularityScoreImage);
    try
    {
      scaleSpaceWriter->Update();
    }
    catch (itk::ExceptionObject &e)
    {
      std::cerr << e << std::endl;
    }
  }

  // Writing the output N-D image.
  typename OutputImageType::Pointer tubularityScoreImage;
  if (takeExponentialOfScoreImage)
  {
    // If the (N+1)-D tubularity image is not generated, then
    // use min and max values of the N-D tubularity image
    // for normalization.
    double expFactor;

    if (generateScaleSpaceTubularityScoreImage)
    {
      typename MinMaxCalculatorForScaleSpaceImageType::Pointer minMaxCalc =
          MinMaxCalculatorForScaleSpaceImageType::New();
      minMaxCalc->SetImage(ofMultiScaleFilter->GetNPlus1DImageOutput());
      minMaxCalc->Compute();
      maxTubularityValue = minMaxCalc->GetMaximum();
      minTubularityValue = minMaxCalc->GetMinimum();

      if (std::fabs(minMaxCalc->GetMaximum() - minMaxCalc->GetMinimum()) <
          itk::NumericTraits<float>::epsilon())
      {
        std::cerr << "Score image pixel values are all the same: ";
        std::cerr << minMaxCalc->GetMaximum() << std::endl;
        return EXIT_FAILURE;
      }
      expFactor = std::log(maxContrastRatio) /
                  static_cast<double>(minMaxCalc->GetMaximum() - minMaxCalc->GetMinimum());
    }
    typename ShiftScaleFilterType::Pointer shiftScaleFilter = ShiftScaleFilterType::New();
    shiftScaleFilter->SetInput(ofMultiScaleFilter->GetOutput());
    shiftScaleFilter->SetShift(0.0);
    shiftScaleFilter->SetScale(expFactor);

    typename ExpFilterType::Pointer expFilter = ExpFilterType::New();
    expFilter->SetInput(shiftScaleFilter->GetOutput());
    expFilter->Update();
    tubularityScoreImage = expFilter->GetOutput();
  }
  else
  {
    tubularityScoreImage = ofMultiScaleFilter->GetOutput();
  }

  typename FileWriterType::Pointer writer = FileWriterType::New();
  writer->SetFileName(outputTubularityScoreImageFilePath);
  // writer->UseCompressionOn();	// Do not use compression since paraview can't read it.
  writer->SetInput(tubularityScoreImage);
  try
  {
    writer->Update();
  }
  catch (itk::ExceptionObject &e)
  {
    std::cerr << e << std::endl;
  }

  // Writing the OrientedFlux matrix image.
  if (generateOFMatrixImage)
  {
    typename OFFileWriterType::Pointer oFluxWriter = OFFileWriterType::New();
    oFluxWriter->SetInput(ofMultiScaleFilter->GetOrientedFluxOutput());
    oFluxWriter->SetFileName(outputOFMatrixImageFilePath);

    try
    {
      oFluxWriter->Update();
    }
    catch (itk::ExceptionObject &e)
    {
      std::cerr << e << std::endl;
    }
  }

  if (generateNPlus1DOFMatrixImage)
  {
    typename NPlus1DOFFileWriterType::Pointer oFluxWriter = NPlus1DOFFileWriterType::New();
    oFluxWriter->SetFileName(outputNPlus1DOFMatrixImageFilePath);
    oFluxWriter->SetInput(ofMultiScaleFilter->GetNPlus1DOrientedFluxOutput());

    try
    {
      oFluxWriter->Update();
    }
    catch (itk::ExceptionObject &e)
    {
      std::cerr << e << std::endl;
    }
  }

  // Writing the scale image.
  if (generateScaleImage)
  {
    typename ScaleFileWriterType::Pointer scaleWriter = ScaleFileWriterType::New();
    scaleWriter->SetFileName(outputScaleImageFilePath);
    scaleWriter->SetInput(ofMultiScaleFilter->GetScaleOutput());

    try
    {
      scaleWriter->Update();
    }
    catch (itk::ExceptionObject &e)
    {
      std::cerr << e << std::endl;
    }
  }

  std::cout << "Exiting with success." << std::endl;

  return EXIT_SUCCESS;
}
