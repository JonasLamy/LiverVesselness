#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkBinaryThinningImageFilter3D.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkConvolutionImageFilter.h"
#include "itkFlatStructuringElement.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkMaskImageFilter.h"
#include "itkSignedMaurerDistanceMapImageFilter.h"

#include "itkEllipseSpatialObject.h"
#include "itkSpatialObjectToImageFilter.h"
#include "itkImageRegionConstIterator.h"
#include "itkMaximumImageFilter.h"

#include "itkConnectedComponentImageFilter.h"

#include "utils.h"



#include <string>


using PixelType = unsigned char;
using ImageType = itk::Image<PixelType,3>;

using PixelLabelType = int;
using LabelImageType = itk::Image<PixelLabelType,3>;

using PixelDistanceType = float;
using FloatImageType = itk::Image<PixelDistanceType,3>;

using VoxelLabelPair = std::pair<int,ImageType::IndexType>;

int addNeighbours(  int label,
                    bool &noNeighbourBif,
                    ImageType::IndexType centralIndex, 
                    ImageType::Pointer skeleton, 
                    ImageType::Pointer bifurcations,
                    std::deque<VoxelLabelPair> &nextVoxels,
                    std::set<ImageType::IndexType> &blackList,
                    ImageType::RegionType::SizeType imgSize
                    )
{
    ImageType::IndexType neighbourIndex;

    std::vector<ImageType::IndexType> neighbours;
     
    for(int i=-1; i<=1; i++)
      for(int j=-1; j<=1; j++)
        for(int k=-1; k<=1; k++)
        {
          if(i==0 && j==0 && k==0)
            continue;

          neighbourIndex = centralIndex;
          neighbourIndex[0] += i;
          neighbourIndex[1] += j;
          neighbourIndex[2] += k;

          if(neighbourIndex[0]<0 || neighbourIndex[0]>= imgSize[0] ){continue;}
          if(neighbourIndex[1]<0 || neighbourIndex[1]>= imgSize[1] ){continue;}
          if(neighbourIndex[2]<0 || neighbourIndex[2]>= imgSize[0] ){continue;}

          if( skeleton->GetPixel( neighbourIndex ) > 0 )
          {
            // check if it was a bifurcation before
            if( bifurcations->GetPixel(neighbourIndex) > 0 )
            {
              noNeighbourBif = false;
            }

            
            // checking if neighbour is not marked
            if( blackList.end() == blackList.find(neighbourIndex) )
            {
                neighbours.push_back(neighbourIndex); 
            }
          }
        }

    for(auto &voxel : neighbours)
    {
      auto pair = std::pair<int,ImageType::IndexType>(++label,voxel);
      // we don't want duplicates so we look in the waiting list container if voxels is already there
      if( std::find(nextVoxels.begin(),nextVoxels.end(),pair) == nextVoxels.end() )
        nextVoxels.push_front( pair );
    }
    return neighbours.size();
}

int main(int argc,char** argv)
 {


     /* what do we need ??
     1) Inputs
        - input file (binaryVessels)
        - input skeleton (centerline of vessels)
        - input mask file (binary liver)
        - output bifurcation (balls on all bifurcations)
     2) algorithms
        - find bifurcations
        - draw bifurcation on new image, no iteration on image
    */

    std::string inputFileName(argv[1]);
    std::string inputSkelName(argv[2]);
    std::string maskFileName(argv[3]);
    std::string gtFileName(argv[4]);
    
    bool isDicom = false;
    
    // read input

    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(inputFileName);
    reader->Update();

    auto binaryVessels = vUtils::readImage<ImageType>(inputFileName,isDicom);
    auto skeleton = vUtils::readImage<ImageType>(inputSkelName,isDicom);
    auto organMask = vUtils::readImage<ImageType>(maskFileName,isDicom);

    // find connected components of vessels

    auto filterCC = itk::ConnectedComponentImageFilter<ImageType,ImageType,ImageType>::New();
    filterCC->SetBackgroundValue(0);
    filterCC->SetFullyConnected(true);
    filterCC->SetInput(binaryVessels);
    filterCC->Update();
    int nbCC = filterCC->GetObjectCount();
    auto vesselsCC = filterCC->GetOutput();

    std::cout<<"number of connected components :"<<nbCC<<std::endl;
    
    auto rescaleFilter = itk::RescaleIntensityImageFilter<ImageType,ImageType>::New();
    rescaleFilter->SetOutputMaximum(1);
    rescaleFilter->SetOutputMinimum(0);
    rescaleFilter->SetInput(binaryVessels);
    rescaleFilter->Update();
    
    std::cout<<"1"<<std::endl;

    std::cout<<"Making bifurcation image..."<<std::endl;

    // creating bifurcation image

    auto bifurcations = ImageType::New();
    bifurcations->SetOrigin( skeleton->GetOrigin());
    bifurcations->SetRegions( skeleton->GetLargestPossibleRegion() );
    bifurcations->SetSpacing( skeleton->GetSpacing() );
    bifurcations->Allocate();
    bifurcations->FillBuffer(0);

    // pruned skeleton

    auto prunedSkeleton = ImageType::New();
    prunedSkeleton->SetOrigin( skeleton->GetOrigin());
    prunedSkeleton->SetRegions( skeleton->GetLargestPossibleRegion() );
    prunedSkeleton->SetSpacing( skeleton->GetSpacing() );
    prunedSkeleton->Allocate();
    prunedSkeleton->FillBuffer(0);

    
    auto maskFilter = itk::MaskImageFilter<ImageType,ImageType,ImageType>::New();
    maskFilter->SetInput( binaryVessels );
    maskFilter->SetMaskImage( organMask );
    maskFilter->SetOutsideValue(0);
    maskFilter->SetMaskingValue(0);
    maskFilter->Update();
    

    // create the gt

    // 1) inverse image
    // 2) distance transform
    // 3) mask filter with bifurcations
    // 4) draw balls from filter

    auto inverseFilter = itk::BinaryThresholdImageFilter<ImageType,ImageType>::New();
    inverseFilter->SetLowerThreshold(0);
    inverseFilter->SetUpperThreshold(0);
    inverseFilter->SetInsideValue(254);
    inverseFilter->SetOutsideValue(0);

    inverseFilter->SetInput( binaryVessels );
    inverseFilter->Update();
    
    auto distanceTransform = itk::SignedMaurerDistanceMapImageFilter<ImageType,FloatImageType>::New();
    distanceTransform->SquaredDistanceOff();
    //distanceTransform->SquaredDistanceOn();
    distanceTransform->SetInput( inverseFilter->GetOutput() );
    distanceTransform->Update();

    // finding largest vessel radius on the skeleton
    auto distanceImage = distanceTransform->GetOutput();

    using WriterFloatType = itk::ImageFileWriter<FloatImageType>;
    
    WriterFloatType::Pointer distanceWriter = WriterFloatType::New();

    distanceWriter->SetFileName("distanceTransform.nii");
    distanceWriter->SetInput(distanceImage);
    try
    {
        distanceWriter->Update();
    }
    catch (itk::ExceptionObject & excp)
    {
        std::cerr << excp << std::endl;
        return EXIT_FAILURE;
    }

    itk::ImageRegionIterator<ImageType> imageIterator( skeleton, skeleton->GetLargestPossibleRegion() );
    itk::ImageRegionIterator<FloatImageType> distanceImageIterator( distanceImage, distanceImage->GetLargestPossibleRegion() );
    itk::ImageRegionIterator<ImageType> ccImageIterator( vesselsCC, vesselsCC->GetLargestPossibleRegion());

    distanceImageIterator.GoToBegin();
    imageIterator.GoToBegin();
    ccImageIterator.GoToBegin();

    ImageType::IndexType bestIndex;
    FloatImageType::PixelType bestDistance = 0;

    std::cout<<"starting skeleton traversal"<<std::endl;
    std::cout<<"finding best starting point "<<std::endl;

    // looping to find the best part to start on the skeleton (biggest vessel radius)
    std::vector<int> startingDistance(nbCC);
    std::vector<ImageType::IndexType> startingIndex(nbCC);
    for(int i=0; i<nbCC;i++) startingDistance[i] = 0;
    while (!imageIterator.IsAtEnd())
    {
      if(imageIterator.Value() > 0)
      {
        for(int i=0; i<nbCC; i++)
        {
          if( i+1 == ccImageIterator.Value() )
          {
            if(distanceImageIterator.Value() > startingDistance[i])
            {
              startingIndex[i] = distanceImageIterator.GetIndex();
              startingDistance[i] = distanceImageIterator.Value();
            }
          }
        }
      }
      ++imageIterator;
      ++distanceImageIterator;
      ++ccImageIterator;
    }

    using CCWriterType = itk::ImageFileWriter<ImageType>;
    
    CCWriterType::Pointer ccWriter = CCWriterType::New();

    ccWriter->SetFileName("cc.nii");
    ccWriter->SetInput(vesselsCC);
    try
    {
        ccWriter->Update();
    }
    catch (itk::ExceptionObject & excp)
    {
        std::cerr << excp << std::endl;
        return EXIT_FAILURE;
    }


        // creating image
    auto resultImage = ImageType::New();
    resultImage->SetRegions( binaryVessels->GetLargestPossibleRegion() );
    auto binaryVesselsImageSize = binaryVessels->GetLargestPossibleRegion().GetSize();
    std::cout<<"image size:"<<binaryVesselsImageSize<<std::endl;

    resultImage->SetSpacing(binaryVessels->GetSpacing() );
    resultImage->Allocate();
    resultImage->FillBuffer(0);
    std::cout<<"result image spacing \n"<<resultImage->GetSpacing() <<std::endl;
    // creating ellipses

    for(int i=0; i<nbCC; i++)
    {
      std::cout<<"best starting point : "<<startingDistance[i]<<" "<<startingIndex[i]<<std::endl;
      if(startingDistance[i] < 1)
      {
        std::cout<<"skipping Connected Component : "<<i<<std::endl;
        continue;
      }
      

      // creating element
      // creating index and next label pair

      std::deque<VoxelLabelPair> nextVoxels;
      std::set<ImageType::IndexType> blackList;
      std::set<ImageType::IndexType> bifurcationsList;

      std::list<ImageType::IndexType> skelBranch;
      std::list<std::list<ImageType::IndexType>> skelBranchExtremityList;
      std::list<std::list<ImageType::IndexType>> skelBranchCoreList;
    
      
      
      // making depth first search
      ImageType::IndexType centralIndex;
      short nbNeighbours = 0;
      int nbBifurcations = 0;
      int nbBifurcationsAmbiguous = 0;
      int nbExtremity=0;

      bool noNeighbourBif = true;
      int label;

      nextVoxels.push_back( std::pair<int,ImageType::IndexType>(1,startingIndex[i]) );

      while( !nextVoxels.empty() )
      {
        /*************************/
        /* Logic for propagation */

        centralIndex = nextVoxels.front().second;
        label = nextVoxels.front().first;

        nextVoxels.pop_front();
        blackList.insert(centralIndex);

        nbNeighbours = addNeighbours(label,noNeighbourBif,centralIndex,skeleton,bifurcations,nextVoxels,blackList,binaryVesselsImageSize);

        /*************************************************/
        /* Logic of what do we do with the current voxel */

        //skelLabels->SetPixel(centralIndex,label);
        bifurcations->SetPixel(centralIndex,50);
        
        if( nbNeighbours == 0) // extremity
        {
          nbExtremity++; 

        }

        if(nbNeighbours == 2 ) // bifurcations
        {
            if(noNeighbourBif)//
            {
              bifurcations->SetPixel(centralIndex,254);
              nbBifurcations++;

              // add index to bifurcation 
              bifurcationsList.insert(centralIndex);

            }
        }
        if(nbNeighbours > 2) // lump
        {
          nbBifurcationsAmbiguous++;
          bifurcations->SetPixel(centralIndex,100);
          // add index to bifurcation 
          bifurcationsList.insert(centralIndex);
        }
        
        noNeighbourBif = true;

        // updating skelinnerSkelCandidateseton labels
      //std::cout<<centralIndex<<"   "<<label<<std::endl;
      }

      std::cout<<"Nb bifurcations:"<<nbBifurcations<<std::endl;
      std::cout<<"Nb bifurcations ambiguous:"<<nbBifurcationsAmbiguous<<std::endl;
      std::cout<<"Nb extremity:"<<nbExtremity<<std::endl; 


    }

    using OutputWriterType = itk::ImageFileWriter<ImageType>;
    auto writer = OutputWriterType::New();

    writer->SetFileName("bifurcation.nii");
    writer->SetInput( bifurcations );
    writer->Update();

    return 0;
}