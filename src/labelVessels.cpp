#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkBinaryThinningImageFilter3D.h"
//#include "itkBinaryThresholdImageFilter.h"
#include "itkMaskImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkImageRegionConstIterator.h"
//#include "itkMaskImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkSignedMaurerDistanceMapImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
//#include "itkConvolutionImageFilter.h"
#include "itkFlatStructuringElement.h"
#include "itkGrayscaleDilateImageFilter.h"

#include <itkReconstructionImageFilter.h>
#include <functional>

#include "utils.h"
#include <deque>          // std::queue
#include <stack>          // std::queue
#include <queue>


using PixelType = unsigned char;
using ImageType = itk::Image<PixelType,3>;

using PixelLabelType = int;
using LabelImageType = itk::Image<PixelLabelType,3>;

using PixelDistanceType = float;
using FloatImageType = itk::Image<PixelDistanceType,3>;

using VoxelLabelPair = std::pair<int,ImageType::IndexType>;

// bad practice but tired
static int labelNumber=6;

// We need two things
// A function going through a branch


template<class TIndex>
struct Branch{
  int typeBif;
  std::list<TIndex> voxels;
  TIndex nextBifurcation;
  std::list<TIndex> nextBifurcationNeighbours;
};

template<class TIndex>
struct BranchStart{
  TIndex start;
  TIndex firstVoxel;
  int label;



};

bool operator==(const BranchStart<ImageType::IndexType> &b1, const BranchStart<ImageType::IndexType> &b2)
{
  if(b1.start == b2.start and  b1.firstVoxel == b2.firstVoxel)
    return true;
  return false;
}

std::ostream & operator<<(std::ostream& os, const BranchStart<ImageType::IndexType> &b1)
{
  os <<"["<< b1.start << "-" << b1.firstVoxel << " " << b1.label <<"]";

  return os;
}

template<class TIndex>
Branch<TIndex> getBranch(TIndex start, TIndex firstVoxel, ImageType::Pointer skeletonImg )
{
  Branch<TIndex> branch;
  // blackList
  std::list<TIndex> blackList;
  // nextVoxels
  std::deque<TIndex> nextVoxels;

  blackList.push_back(start);
  nextVoxels.push_back(firstVoxel);

  std::list<TIndex> neighboursList;
  while( !nextVoxels.empty() )
  {
      TIndex currentVoxel = nextVoxels.front();

      nextVoxels.pop_front();
      blackList.push_back(currentVoxel);
      neighboursList.clear();

      // looking for neighbours
      for(int i=-1; i<=1; i++)
        for(int j=-1; j<=1; j++)
          for(int k=-1; k<=1; k++)
          {
            if(i==0 && j==0 && k==0)
              continue;

            TIndex neighbourIndex = currentVoxel;
            neighbourIndex[0] += i;
            neighbourIndex[1] += j;
            neighbourIndex[2] += k;

            if( skeletonImg->GetPixel(neighbourIndex) > 0 ) // neighbour is on skeleton
            {
              if( blackList.end() == std::find( blackList.begin(), blackList.end(), neighbourIndex) ) // not in blacklist
              {
                if( nextVoxels.end() == std::find( nextVoxels.begin(), nextVoxels.end(), neighbourIndex) ) // not in waiting list
                {
                  nextVoxels.push_back(neighbourIndex);
                  neighboursList.push_back(neighbourIndex);
                }
              }  
            }
                  
          }

      branch.voxels.push_back(currentVoxel);
      // we have neighbours, if we have 2 unmarked neighbours, then we break
      if(neighboursList.size() == 2)
      {
        branch.typeBif=2;
        branch.nextBifurcation = currentVoxel;
        branch.nextBifurcationNeighbours = neighboursList;
        return branch;
      }

      
      
  }
  branch.typeBif=0; // if we are here it means we came to the end peacefully
  return branch; 
}
// 



int main(int argc,char** argv)
 {

    std::string inputFileName(argv[1]);
    std::string gtFileName(argv[2]);
    
    bool isDicom = false;

    // read input

    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(inputFileName);
    reader->Update();

    auto img = vUtils::readImage<ImageType>(inputFileName,isDicom);

    // make skeleton
    
    auto thinningFilter = itk::BinaryThinningImageFilter3D<ImageType,ImageType>::New();
    thinningFilter->SetInput( img );
    
    std::cout<<"Thinning...";
    thinningFilter->Update();
    std::cout<<"done"<<std::endl;

    auto skeleton = thinningFilter->GetOutput();

    auto inverseFilter = itk::BinaryThresholdImageFilter<ImageType,ImageType>::New();
    inverseFilter->SetLowerThreshold(0);
    inverseFilter->SetUpperThreshold(0);
    inverseFilter->SetInsideValue(254);
    inverseFilter->SetOutsideValue(0);

    inverseFilter->SetInput( img );
    
    auto distanceTransform = itk::SignedMaurerDistanceMapImageFilter<ImageType,FloatImageType>::New();
    distanceTransform->SquaredDistanceOff();
    //distanceTransform->SquaredDistanceOn();
    distanceTransform->SetInput( inverseFilter->GetOutput() );
    distanceTransform->Update();

    // TODO : searching for all connected compononents

    // finding largest vessel radius on the skeleton
    auto distanceImage = distanceTransform->GetOutput();
    itk::ImageRegionIterator<ImageType> imageIterator( skeleton, skeleton->GetLargestPossibleRegion() );
    itk::ImageRegionIterator<FloatImageType> distanceImageIterator( distanceImage, distanceImage->GetLargestPossibleRegion() );

    distanceImageIterator.GoToBegin();
    imageIterator.GoToBegin();
    ImageType::IndexType bestIndex;
    FloatImageType::PixelType bestDistance = 0;

    // looping to find the best part to start on the skeleton (biggest vessel radius)
    while (!imageIterator.IsAtEnd())
    {
      if(imageIterator.Value() > 0)
      {
        if(distanceImageIterator.Value() > bestDistance)
        {
          bestIndex = distanceImageIterator.GetIndex();
          bestDistance = distanceImageIterator.Value();
        }
      }
      ++imageIterator;
      ++distanceImageIterator;
    }
    
    // first case 1 voxel, if center -> two neighbours
    // One, get branches on both sides
    // two label them both the same

    auto skelLabels = LabelImageType::New();
    skelLabels->SetOrigin( skeleton->GetOrigin() );
    skelLabels->SetRegions( skeleton->GetLargestPossibleRegion() );
    skelLabels->SetSpacing( skeleton->GetSpacing() );
    skelLabels->Allocate();
    skelLabels->FillBuffer(0);

    int label = 1;
    int skelLabel;
    // TODO careful to cycles....
    std::deque< BranchStart<ImageType::IndexType> > nextBranch;
    std::list<LabelImageType::IndexType> labeledVoxels;

    // looking for the two neighbours
    // looking for neighbours
    skelLabels->SetPixel(bestIndex,label);

    std::list<ImageType::IndexType> n;
    for(int i=-1; i<=1; i++)
      for(int j=-1; j<=1; j++)
        for(int k=-1; k<=1; k++)
        {
          if(i==0 && j==0 && k==0)
            continue;

          ImageType::IndexType neighbourIndex = bestIndex;
          neighbourIndex[0] += i;
          neighbourIndex[1] += j;
          neighbourIndex[2] += k;

          if( skeleton->GetPixel(neighbourIndex) > 0 )
          {
            BranchStart<ImageType::IndexType> b;
            b.start = bestIndex;
            b.firstVoxel = neighbourIndex;
            b.label = label;
            nextBranch.push_back(b);
          }
    
        }

      for(auto &b :nextBranch)
      {
        std::cout<<b<<" ";
      }


    while( !nextBranch.empty() )
    {
      auto nextBranchStart = nextBranch.front();
      nextBranch.pop_front();

      label=nextBranchStart.label;
    
      // retrieving the branch
      std::cout<<nextBranchStart<<std::endl; 
      auto newBranch = getBranch<ImageType::IndexType>(nextBranchStart.start,nextBranchStart.firstVoxel,skeleton);

      //std::cout<<newBranch<<std::endl;
      // labeling the current branch

        skelLabel =1;
      if( label > 1)
        skelLabel = 2;
      if( label > 1)
        skelLabel = 2;
      if( label > 1)
        skelLabel = 2;

        for(auto &v : newBranch.voxels)
        {
            skelLabels->SetPixel(v,label);
        }

      if(newBranch.typeBif == 2)
      {
        for(auto &n : newBranch.nextBifurcationNeighbours)
        {
          if(skelLabels->GetPixel(n) == 0)
          {
            BranchStart<ImageType::IndexType> b;
            b.start = newBranch.voxels.back(); // last element inserted, should be a bifurcation
            b.firstVoxel = n;
            b.label = label+1;
            if( nextBranch.end() == std::find(nextBranch.begin(),nextBranch.end(),b) )
              nextBranch.push_back( b );
          }
        }
      }
      std::cout<<nextBranch.size()<<std::endl;

      if(nextBranch.size()>100)
      { 
        std::cout<<"exit error ! "<<std::endl;
        exit(0);
      }
    }

  using OutputWriterType = itk::ImageFileWriter<ImageType>;
  auto writer = OutputWriterType::New();

  writer->SetFileName("skel.nii");
  writer->SetInput( skeleton );
  writer->Update();

  using LabelWriterType = itk::ImageFileWriter<LabelImageType>;
  auto labelWriter = LabelWriterType  ::New();

  labelWriter->SetFileName("labeledSkel.nii");
  labelWriter->SetInput( skelLabels );
  labelWriter->Update();

   return 0;
 }