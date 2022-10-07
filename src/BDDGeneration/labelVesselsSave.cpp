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


int addNeighbours(int label,
                    bool &noNeighbourBif,
                    ImageType::IndexType centralIndex, 
                    ImageType::Pointer skeleton, 
                    ImageType::Pointer bifurcations,
                    std::deque<VoxelLabelPair> &nextVoxels,
                    std::set<ImageType::IndexType> &blackList
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
    

    int newLabel = label;

    if( neighbours.size() == 2 )
    {
      if(newLabel > 1)
        newLabel--;
    }

    for(auto &voxel : neighbours)
    {
      auto pair = std::pair<int,ImageType::IndexType>(newLabel,voxel);
      // we don't want duplicates so we look in the waiting list container if voxels is already there
      if( std::find(nextVoxels.begin(),nextVoxels.end(),pair) == nextVoxels.end() )
        nextVoxels.push_front( pair );
    }
    return neighbours.size();
}

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

    // creating bifurcation image

    auto bifurcations = ImageType::New();
    bifurcations->SetOrigin( skeleton->GetOrigin());
    bifurcations->SetRegions( skeleton->GetLargestPossibleRegion() );
    bifurcations->SetSpacing( skeleton->GetSpacing() );
    bifurcations->Allocate();
    bifurcations->FillBuffer(0);

    // creating label image

    auto skelLabels = LabelImageType::New();
    skelLabels->SetOrigin( skeleton->GetOrigin() );
    skelLabels->SetRegions( skeleton->GetLargestPossibleRegion() );
    skelLabels->SetSpacing( skeleton->GetSpacing() );
    skelLabels->Allocate();
    skelLabels->FillBuffer(0);

    // creating inner skeleton 

    auto innerSkel = LabelImageType::New();
    innerSkel->SetOrigin( skeleton->GetOrigin());
    innerSkel->SetRegions( skeleton->GetLargestPossibleRegion() );
    innerSkel->SetSpacing( skeleton->GetSpacing() );
    innerSkel->Allocate();
    innerSkel->FillBuffer(0);

    // looking for first skeleton voxel
    // we choose the largest point on skeleton
    
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

    // skleton list
    std::list<ImageType::IndexType> innerSkelVoxels;
    std::list<ImageType::IndexType> innerSkelCandidates;

    // creating element
    // creating index and next label pair

    std::deque<VoxelLabelPair> nextVoxels;
    std::set<ImageType::IndexType> blackList;

    nextVoxels.push_back( std::pair<int,ImageType::IndexType>(labelNumber,bestIndex) );

    std::cout<<bestDistance<<" "<<bestIndex<<std::endl;
    
    // making depth first search
    ImageType::IndexType centralIndex;
    short nbNeighbours = 0;
    int nbBifurcations = 0;
    int nbBifurcationsAmbiguous = 0;
    int nbExtremity=0;

    bool noNeighbourBif = true;

    int label;
    while( !nextVoxels.empty() )
    {
      /*************************/
      /* Logic for propagation */

      centralIndex = nextVoxels.front().second;
      label = nextVoxels.front().first;

      nextVoxels.pop_front();
      blackList.insert(centralIndex);


      nbNeighbours = addNeighbours(label,noNeighbourBif,centralIndex,skeleton,bifurcations,nextVoxels,blackList);
      std::cout<<"toto:"<<nbNeighbours<<std::endl;

      /*************************************************/
      /* Logic of what do we do with the current voxel */

      skelLabels->SetPixel(centralIndex,label);

      innerSkelCandidates.push_back(centralIndex);
      
      if( nbNeighbours == 0) // extremity
      {
        innerSkelCandidates.clear();
        nbExtremity++;

        // clearing last branch and labeling it to extremity
        for( auto &voxel: innerSkelCandidates)
          skelLabels->SetPixel(centralIndex,1); // one is always extremity label except if number of labels is greater than number of vessels 
      }

      if(nbNeighbours == 2 ) // bifurcations
      {
          if(noNeighbourBif)//
          {
            std::cout<<"bifurcation ! "<<std::endl;
            bifurcations->SetPixel(centralIndex,254);
            nbBifurcations++;


            innerSkelCandidates.splice(innerSkelVoxels.begin(),
                                            innerSkelVoxels,
                                            innerSkelCandidates.begin(),
                                            innerSkelCandidates.end());

            // we clean up the temporary vector since they already are selected
            innerSkelCandidates.clear();
          }
      }
      if(nbNeighbours > 2) // lump
      {
        nbBifurcationsAmbiguous++;
      }
      

      noNeighbourBif = true;

      // updating skelinnerSkelCandidateseton labels
      std::cout<<centralIndex<<"   "<<label<<std::endl;
    }

    std::cout<<"Nb bifurcations:"<<nbBifurcations<<std::endl;
    std::cout<<"Nb labels:"<<labelNumber-label<<std::endl;
    std::cout<<"Nb bifurcations ambiguous:"<<nbBifurcationsAmbiguous<<std::endl;
    std::cout<<"Nb extremity:"<<nbExtremity<<std::endl;
    
    // printing inner skeleton into volume
    // Bad implementation but no choice for now
    // skeleton contains inner skeleton and skelLabels contains full skeleton
    for(auto &voxel : innerSkelVoxels)
    {
      innerSkel->SetPixel(voxel, skelLabels->GetPixel(voxel) );
    }

    // propagate labels back to vessels

    using StructuringElementType = itk::FlatStructuringElement<3>;
    StructuringElementType::RadiusType radius;
    radius.Fill(1);
    StructuringElementType structuringElement = StructuringElementType::Ball(radius);

    using GrayscaleDilateImageFilterType = itk::GrayscaleDilateImageFilter<LabelImageType, LabelImageType, StructuringElementType>;
    GrayscaleDilateImageFilterType::Pointer dilateFilter = GrayscaleDilateImageFilterType::New();

    using MaskFilterType = itk::MaskImageFilter<LabelImageType, ImageType>;
    MaskFilterType::Pointer maskFilter = MaskFilterType::New();

    auto labeledVessels = innerSkel;
    int vesselsLargestRadius = bestDistance+1;
    for(int i=0;i<vesselsLargestRadius;i++)
    {
      std::cout<<i<<std::endl;
      dilateFilter->SetInput(labeledVessels);
      dilateFilter->SetKernel(structuringElement);
      dilateFilter->Update();

      maskFilter->SetInput( dilateFilter->GetOutput() );
      maskFilter->SetMaskImage(img);
      maskFilter->Update();
      labeledVessels = maskFilter->GetOutput();
      
    }
    
    // save volumes
    
    using OutputWriterType = itk::ImageFileWriter<ImageType>;
    auto writer = OutputWriterType::New();

    writer->SetFileName("skel.nii");
    writer->SetInput( skeleton );
    writer->Update();

    writer->SetFileName("bifurcations.nii");
    writer->SetInput( bifurcations );
    writer->Update();

    using LabelWriterType = itk::ImageFileWriter<LabelImageType>;
    auto labelWriter = LabelWriterType  ::New();

    labelWriter->SetFileName("labeledSkel.nii");
    labelWriter->SetInput( skelLabels );
    labelWriter->Update();

    labelWriter->SetFileName("labeledInnerSkel.nii");
    labelWriter->SetInput( innerSkel );
    labelWriter->Update();

    labelWriter->SetFileName("labeledVessels.nii");
    labelWriter->SetInput( labeledVessels );
    labelWriter->Update();

    return 0;
}