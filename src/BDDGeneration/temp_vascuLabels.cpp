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


#include "utils.h"

#include <iostream>
#include <string>
#include <set>
#include <queue>
#include <algorithm>
#include <vector>
#include <list>

using PixelType = unsigned char;
using ImageType = itk::Image<PixelType,3>;

using PixelLabelType = int;
using LabelImageType = itk::Image<PixelLabelType,3>;

using PixelDistanceType = float;
using FloatImageType = itk::Image<PixelDistanceType,3>;


struct Index
{ 
    Index(){x=0;y=0;z=0;}
    Index(const ImageType::IndexType &index){x=index[0];y=index[1];z=index[2];}
    Index(int x, int y, int z){this->x = x; this->y = y; this->z = z;}
    bool operator <( const Index &b) const{ return std::tie(x,y,z) < std::tie(b.x,b.y,b.z) ; }
    bool operator ==( const Index &b) const{ if(x == b.x and y == b.y and b.z == z){return true;}else{return false;} }
    int x;
    int y;
    int z;

    private:
        Index& operator=(Index&& other){ return *this;}
};

std::ostream & operator<<(std::ostream & os,const Index &index)
{
    os<<"["<<index.x<<","<<index.y<<","<<index.z<<"]"<<std::endl;
    return os;
}

struct VoxelLabelPair{
    VoxelLabelPair(int label,Index index){ m_label=label; m_index=index;}
    int m_label=0;
    Index m_index;
    bool operator <( const VoxelLabelPair &b) const{ return m_label < b.m_label; }
    VoxelLabelPair& operator =(const VoxelLabelPair &vp){m_label = vp.m_label; m_index = vp.m_index; return *this;}
};

std::ostream & operator<<(std::ostream & os,const VoxelLabelPair &vlp)
{
    os<<"["<<vlp.m_label<<std::endl<<vlp.m_index<<std::endl;
    return os;
}


using VoxelsToVisitListType = std::priority_queue<VoxelLabelPair,std::vector<VoxelLabelPair>>;

using BlackListType = std::vector<Index>;
using NeighboursListType = std::list<Index>;
using IndexListType = NeighboursListType;

NeighboursListType getNeighbours(const Index &startingPoint, ImageType::Pointer skeleton)
{
    NeighboursListType neighbours;
    ImageType::IndexType neighbourIndex;

    for(int i=-1; i<=1; i++)
      for(int j=-1; j<=1; j++)
        for(int k=-1; k<=1; k++)
        {
          if(i==0 && j==0 && k==0)
            continue;

          neighbourIndex[0] = startingPoint.x + i;
          neighbourIndex[1] = startingPoint.y + j;
          neighbourIndex[2] = startingPoint.z + k;

          if( skeleton->GetPixel( neighbourIndex ) > 0 )
          {
            neighbours.push_back( Index(neighbourIndex[0],neighbourIndex[1],neighbourIndex[2]) ); // failing and I don't know why
          }
        }    
    return neighbours;
}

Index findStartingPoint(ImageType::Pointer skeleton,FloatImageType::Pointer distanceImage)
{
    itk::ImageRegionIterator<ImageType> imageIterator( skeleton, skeleton->GetLargestPossibleRegion() );
    itk::ImageRegionIterator<FloatImageType> distanceImageIterator( distanceImage, distanceImage->GetLargestPossibleRegion() );
    
    distanceImageIterator.GoToBegin();
    imageIterator.GoToBegin();

    ImageType::IndexType bestIndex;
    FloatImageType::PixelType bestDistance = 0;

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

    return Index(bestIndex);
}


int main(int argc, char** argv)
{

    std::string binaryVesselsPath(argv[1]);
    std::string skeletonSavePath(argv[2]);
    std::string dTransformPath(argv[3]);
    std::string labelSavePath(argv[4]);

    std::cout<<binaryVesselsPath<<std::endl;
    std::cout<<skeletonSavePath<<std::endl;
    std::cout<<dTransformPath<<std::endl;
    std::cout<<labelSavePath<<std::endl;
    
    auto binaryVessels = vUtils::readImage<ImageType>(binaryVesselsPath,false);

    //
    // Computing skeleton
    // 

    auto thinningFilter = itk::BinaryThinningImageFilter3D<ImageType,ImageType>::New();
    thinningFilter->SetInput(binaryVessels);
    thinningFilter->Update();

    auto skeleton = thinningFilter->GetOutput();

    using WriterType = itk::ImageFileWriter<ImageType>;
    
    WriterType::Pointer thinningWriter = WriterType::New();

    thinningWriter->SetFileName(skeletonSavePath);
    thinningWriter->SetInput(skeleton);
    try
    {
        thinningWriter->Update();
    }
    catch (itk::ExceptionObject & excp)
    {
        std::cerr << excp << std::endl;
        return EXIT_FAILURE;
    }

    //
    // Computing distance transform of the inside of the vessels
    // 

    // inverting the binary vessels for distance transform
    auto inverseFilter = itk::BinaryThresholdImageFilter<ImageType,ImageType>::New();
    inverseFilter->SetLowerThreshold(0);
    inverseFilter->SetUpperThreshold(0);
    inverseFilter->SetInsideValue(254);
    inverseFilter->SetOutsideValue(0);

    inverseFilter->SetInput( binaryVessels );
    inverseFilter->Update();

    // computing distance transform
    auto distanceTransform = itk::SignedMaurerDistanceMapImageFilter<ImageType,FloatImageType>::New();
    distanceTransform->SquaredDistanceOff();
    distanceTransform->SetInput( inverseFilter->GetOutput() );
    distanceTransform->Update();

    auto distanceImage = distanceTransform->GetOutput();

    auto maskFilter = itk::MaskImageFilter<FloatImageType,ImageType,FloatImageType>::New();
    maskFilter->SetInput( distanceImage );
    maskFilter->SetMaskImage( binaryVessels );
    maskFilter->SetOutsideValue(0);
    maskFilter->SetMaskingValue(0);
    maskFilter->Update();

    using FloatWriterType = itk::ImageFileWriter<FloatImageType>;
    
    FloatWriterType::Pointer distanceWriter = FloatWriterType::New();

    distanceWriter->SetFileName( dTransformPath  );
    distanceWriter->SetInput( maskFilter->GetOutput() );
    try
    {
        distanceWriter->Update();
    }
    catch (itk::ExceptionObject & excp)
    {
        std::cerr << excp << std::endl;
        return EXIT_FAILURE;
    }


    auto branchLabels = LabelImageType::New();
    branchLabels->SetOrigin( branchLabels->GetOrigin());
    branchLabels->SetRegions( branchLabels->GetLargestPossibleRegion() );
    branchLabels->SetSpacing( branchLabels->GetSpacing() );
    branchLabels->Allocate();
    branchLabels->FillBuffer(0);

    // Get starting point 

    auto startingIndex = findStartingPoint(skeleton, maskFilter->GetOutput() );
    std::cout<<"starting index:"<<startingIndex<<std::endl;

    VoxelsToVisitListType voxelsToVisit;
    voxelsToVisit.push( VoxelLabelPair(1,startingIndex) );
    
    BlackListType visitedVoxels;

    ImageType::IndexType currentIndex;

    int i = 0;
    while( !voxelsToVisit.empty() )
    {
        auto currentLabelAndVoxel = voxelsToVisit.top();
        voxelsToVisit.pop();

        std::cout<<"adding label to visited voxels:"<<currentLabelAndVoxel.m_index<<std::endl;
        std::cout<<"visited list elements:"<<visitedVoxels.size()<<std::endl;
        visitedVoxels.push_back( currentLabelAndVoxel.m_index );
        std::cout<<"this is it"<<std::endl;

        ImageType::IndexType index;
        index[0] = currentLabelAndVoxel.m_index.x;
        index[1] = currentLabelAndVoxel.m_index.y;
        index[2] = currentLabelAndVoxel.m_index.z;
        branchLabels->SetPixel(index,currentLabelAndVoxel.m_label);


        IndexListType neighbours = getNeighbours( currentLabelAndVoxel.m_index, skeleton);
        for(auto &n : neighbours)
        {   
            bool inVisited = false;
            for(auto &v :visitedVoxels)
            {
                if(n == v){inVisited = true;}
                break;
            }
            
            if( !inVisited )
            {
                if( neighbours.size() > 1 )
                {
                    VoxelLabelPair vp(currentLabelAndVoxel.m_label+1,n);
                    std::cout<<"pushing this:"<<vp;
                    voxelsToVisit.push( vp );
                }
                else
                {
                    VoxelLabelPair vp(currentLabelAndVoxel.m_label,n);
                    std::cout<<"pushing this:"<<vp;
                    voxelsToVisit.push( vp );
                }
            }
        }
    }

    std::cout<<"skeleton labelization done"<<std::endl;
    /*
    using WriterLabelType = itk::ImageFileWriter<LabelImageType>;
    
    WriterLabelType::Pointer labelWriter = WriterLabelType::New();

    labelWriter->SetFileName(labelSavePath);
    labelWriter->SetInput(branchLabels);
    try
    {
        labelWriter->Update();
    }
    catch (itk::ExceptionObject & excp)
    {
        std::cerr << excp << std::endl;
        return EXIT_FAILURE;
    }
    */


    return 0;
}