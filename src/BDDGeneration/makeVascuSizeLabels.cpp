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
#include "itkDanielssonDistanceMapImageFilter.h"

#include "itkEllipseSpatialObject.h"
#include "itkSpatialObjectToImageFilter.h"

#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIteratorWithIndex.h"

#include "itkMaximumImageFilter.h"

#include "utils.h"

#include <string>

using PixelType = unsigned char;
using ImageType = itk::Image<PixelType,3>;

using PixelLabelType = int;
using LabelImageType = itk::Image<PixelLabelType,3>;

using PixelDistanceType = float;
using FloatImageType = itk::Image<PixelDistanceType,3>;

using VoxelLabelPair = std::pair<int,ImageType::IndexType>;

using VoronoiPixelType = PixelType;
using VoronoiImageType = itk::Image<VoronoiPixelType, 3>;

// really dirty thing, but no time for now
std::vector<ImageType::IndexType> getNeighbours(ImageType::IndexType centralIndex, 
                ImageType::Pointer skeleton )
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
            neighbours.push_back(neighbourIndex); 
          }
        }
    return neighbours;
}

std::vector<LabelImageType::IndexType> getNeighbours(LabelImageType::IndexType centralIndex, 
                LabelImageType::Pointer skeleton )
{
    LabelImageType::IndexType neighbourIndex;
    std::vector<LabelImageType::IndexType> neighbours;
     
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
            neighbours.push_back(neighbourIndex); 
          }
        }
    return neighbours;
}


static int static_uniqueLabel = 1;
int addNeighbours(  int label,
                    bool &noNeighbourBif,
                    ImageType::IndexType centralIndex, 
                    ImageType::Pointer skeleton, 
                    std::deque<VoxelLabelPair> &nextVoxels,
                    std::set<ImageType::IndexType> &blackList,
                    std::set<ImageType::IndexType> &bifurcationsList
                    )
{
    ImageType::IndexType neighbourIndex;

    std::vector<ImageType::IndexType> neighbours;
    std::cout<<"central:"<<centralIndex<<std::endl;
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

          std::cout<<neighbourIndex<<" "<< int( skeleton->GetPixel( neighbourIndex ) ) <<std::endl;

          if(neighbourIndex[0]<0 || neighbourIndex[0]>= 128 ){continue;}
          if(neighbourIndex[1]<0 || neighbourIndex[1]>= 128 ){continue;}
          if(neighbourIndex[2]<0 || neighbourIndex[2]>= 128 ){continue;}

          if( skeleton->GetPixel( neighbourIndex ) > 0 )
          { 

            if(label < 3)
            {
                std::cout<<"is neighbour:"<<neighbourIndex<<std::endl;
            }

            // checking if neighbour is not marked
            if( blackList.end() == blackList.find(neighbourIndex) )
            {
                neighbours.push_back(neighbourIndex); 
                if(label<3)
                {
                    std::cout<<"added neighbour:"<<neighbourIndex<<std::endl;
                }
            }
            else{
                if(label<3)
                {
                    std::cout<<"already in blacklist"<<neighbourIndex<<std::endl;
                }
            }
          }
          else{
              std::cout<<"no neighbour at:"<<neighbourIndex<<std::endl;
          }

        }
    
    if(label < 3)
    {
        std::cout<<"--------"<<std::endl;
        for(auto &b : blackList){std::cout<<"blacklist:"<<b<<std::endl;}
        std::cout<<"label: "<<label<<std::endl;
        std::cout<<"static label :"<<static_uniqueLabel<<std::endl;
        std::cout<<"central pixel : "<<centralIndex<<std::endl;
        std::cout<<"neighbours :"<<std::endl;
        for(auto &n : neighbours){std::cout<<n;}
        std::cout<<std::endl;
    }
    
    if(neighbours.size() >= 2)
    {
        for(auto &voxel : neighbours)
        {
            auto pair = std::pair<int,ImageType::IndexType>(++static_uniqueLabel,voxel);
            // we don't want duplicates so we look in the waiting list container if voxels is already there
            if( std::find(nextVoxels.begin(),nextVoxels.end(),pair) == nextVoxels.end() )
            {
                nextVoxels.push_front( pair );
            }
        }
    }
    else
    {
        for(auto &voxel : neighbours)
        {
            auto pair = std::pair<int,ImageType::IndexType>(label,voxel);
            // we don't want duplicates so we look in the waiting list container if voxels is already there
            if( std::find(nextVoxels.begin(),nextVoxels.end(),pair) == nextVoxels.end() )
                nextVoxels.push_front( pair );
        }
    }

    return neighbours.size();
}

int main(int argc,char** argv)
{

    std::string inputFileName(argv[1]);
    std::string gtFileName(argv[2]);
    std::string skelFileName(argv[3]);
    std::string voronoiSavePath(argv[4]);
    
    bool isDicom = false;
    
    std::string skeletonFileName;
    bool saveSkeleton = false;
    if(argc >= 6)
    {
        saveSkeleton = true;
        skeletonFileName = argv[5];
    }    

    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(inputFileName);
    reader->Update();

    auto binaryVessels = vUtils::readImage<ImageType>(inputFileName,isDicom);

    auto thinningFilter = itk::BinaryThinningImageFilter3D<ImageType,ImageType>::New();
    thinningFilter->SetInput(binaryVessels);
    thinningFilter->Update();

    auto skeleton = thinningFilter->GetOutput();

    using WriterType = itk::ImageFileWriter<ImageType>;
    if(saveSkeleton)
    {
        WriterType::Pointer thinningWriter = WriterType::New();

        thinningWriter->SetFileName(skeletonFileName);
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
    }

    // 1) inverse image
    // 2) distance transform

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

    distanceImageIterator.GoToBegin();
    imageIterator.GoToBegin();

    ImageType::IndexType bestIndex;
    FloatImageType::PixelType bestDistance = 0;

    std::cout<<"starting skeleton traversal"<<std::endl;
    std::cout<<"finding best starting point "<<std::endl;

    // looping to find the best part to start on the skeleton (biggest vessel radius)
    int startingDistance = 0;
    ImageType::IndexType startingIndex;
    while (!imageIterator.IsAtEnd())
    {
      if(imageIterator.Value() > 0)
      {
        if(distanceImageIterator.Value() > startingDistance)
        {
            startingIndex = distanceImageIterator.GetIndex();
            startingDistance = distanceImageIterator.Value();
        }
      }
      ++imageIterator;
      ++distanceImageIterator;
    }

    // creating image
    LabelImageType::Pointer resultImage = LabelImageType::New();
    resultImage->SetRegions( binaryVessels->GetLargestPossibleRegion() );
    resultImage->SetOrigin( binaryVessels->GetOrigin() );
    resultImage->SetSpacing(binaryVessels->GetSpacing() );
    resultImage->Allocate();
    resultImage->FillBuffer(0);
    
    // creating element
    // creating index and next label pair

    std::deque<VoxelLabelPair> nextVoxels;
    std::set<ImageType::IndexType> blackList;
    std::set<ImageType::IndexType> bifurcationsList;

    std::list<ImageType::IndexType> skelBranch;
    
    // making depth first search
    ImageType::IndexType centralIndex;
    short nbNeighbours = 0;
    int nbBifurcations = 0;
    int nbBifurcationsAmbiguous = 0;
    int nbExtremity=0;

    bool noNeighbourBif = true;
    int label;
    
    // Starting point is often in the middle of a branch
    // manually setting labels for this special case
    
    resultImage->SetPixel(startingIndex,static_uniqueLabel);
    blackList.insert(startingIndex);
    //nextVoxels.push_back( std::pair<int,ImageType::IndexType>( static_uniqueLabel, startingIndex) );

    std::cout<<"starting index:"<<startingIndex<<std::endl;

    auto neighbours = getNeighbours(startingIndex, skeleton);
    for(auto &n : neighbours)
    {
        nextVoxels.push_back( std::pair<int,ImageType::IndexType>( static_uniqueLabel, n) );
    }     
    
    for(auto &b : blackList){std::cout<<"blacklist:"<<b<<std::endl;}
    for(auto &n : nextVoxels){std::cout<<"nextVoxels:"<<n.second<<std::endl;}

    while( !nextVoxels.empty() )
    {
        /*************************/
        /* Logic for propagation */
        std::cout<<"***"<<std::endl;
        for(auto &n : nextVoxels){std::cout<<"nextVoxels:"<<n.second<<std::endl;}
        centralIndex = nextVoxels.front().second;
        label = nextVoxels.front().first;

        nextVoxels.pop_front();
        blackList.insert(centralIndex);

        nbNeighbours = addNeighbours(label,noNeighbourBif,centralIndex,skeleton,nextVoxels,blackList,bifurcationsList);

    /*************************************************/
    /* Logic of what do we do with the current voxel */

        resultImage->SetPixel(centralIndex,label);
    }

    using OutputWriterType = itk::ImageFileWriter<LabelImageType>;
    auto writer = OutputWriterType::New();

    writer->SetFileName(skelFileName);
    writer->SetInput( resultImage );
    writer->Update();

    // 1) Nettoyer les labels du squelette
    // 2) Creer une map pour compter le nombre de voxels par labels
    // 3) Creet une map pour lister les voxels par label
    // 4) Pour chaques label ayant un nombre de voxels == 1
    // 5) prendre le label d'un voisin (au hasard)
    // 6) Utiliser les cellules de voronoi de chaque ensemble pour labeliser les vaisseaux entiers
    // 7) Reparcourir le squelette pour relabeliser les vaisseaux en fonction de la hierarchie voulue

    std::map< int, std::vector<LabelImageType::IndexType> > listIndexPerLabel;
    std::map<int, int> voxelsPerLabel;

    itk::ImageRegionIteratorWithIndex<LabelImageType> labelImageIterator( resultImage, resultImage->GetLargestPossibleRegion() );
    labelImageIterator.GoToBegin();

    while( !labelImageIterator.IsAtEnd() )
    {
        if( labelImageIterator.Value() > 0 )
        {
            auto it = voxelsPerLabel.find( labelImageIterator.Value() );
            if( it == voxelsPerLabel.end() )
            { 
                voxelsPerLabel[ labelImageIterator.Value() ] = 1;
                std::vector<LabelImageType::IndexType> v(1, labelImageIterator.GetIndex() );
                listIndexPerLabel[ labelImageIterator.Value() ] = v ;
            }
            else
            {
                voxelsPerLabel[ labelImageIterator.Value() ] += 1;
                listIndexPerLabel[ labelImageIterator.Value() ].push_back( labelImageIterator.GetIndex() ) ;
            }
            
        }
        ++labelImageIterator;
    }

    std::vector<int> labelToBeRemoved;

    for(auto &p : voxelsPerLabel)
    {
        if(p.second == 1)
        {
            int l = p.first;
            auto i = listIndexPerLabel[l][0];
            // get neighbours
            auto neighbours = getNeighbours( i, resultImage );
            // set new label from the closest neighbour
            int newLabel;

            for(auto &n : neighbours)
            {  
                int tempLabel = resultImage->GetPixel(neighbours[0]);
                if( voxelsPerLabel[ tempLabel ] > 1 ) 
                {
                    newLabel = tempLabel;
                    break;
                } 
            }
            voxelsPerLabel[ newLabel ] += 1;
            listIndexPerLabel[ newLabel ].push_back( i ) ;
            
            labelToBeRemoved.push_back(l);
        }
    }

    // delete labels with only 1 member, these are usually voxels in messy bifurcations
    for(auto &l : labelToBeRemoved)
    {
        voxelsPerLabel.erase(l);
        listIndexPerLabel.erase(l);
    }

    std::cout<<"----"<<std::endl;
    for(auto &p : voxelsPerLabel)
    {
        std::cout<<"label:"<<p.first<<"  nbVoxels:"<<p.second<<std::endl;
    }

    std::cout<<"----"<<std::endl;
    // re order the labels so that they are consecutive
    label = 1;
    for(auto &p : listIndexPerLabel)
    {
        for(auto &index : p.second){ resultImage->SetPixel( index,label); }
        label++;
    }
    
    writer->SetFileName(gtFileName);
    writer->SetInput( resultImage );
    writer->Update();

    // creating voronoi partition

    using VoronoiFilterType = itk::DanielssonDistanceMapImageFilter<LabelImageType,
                                                           FloatImageType,
                                                           VoronoiImageType>;

    auto voronoiFilter = VoronoiFilterType::New();
    voronoiFilter->SetInput( resultImage );
    voronoiFilter->Update();
    auto voronoiMap = voronoiFilter->GetVoronoiMap();

    // mask voronoi map with vessels

    auto maskFilter = itk::MaskImageFilter<VoronoiImageType,ImageType,VoronoiImageType>::New();
    maskFilter->SetInput( voronoiMap );
    maskFilter->SetMaskImage( binaryVessels );
    maskFilter->SetOutsideValue(0);
    maskFilter->SetMaskingValue(0);
    maskFilter->Update();

    // changing labels to the max size of each vessels

    std::map<int,float> vesselsSize;

    labelImageIterator.GoToBegin();
    while( !labelImageIterator.IsAtEnd() )
    {
        if( labelImageIterator.Value() == 0)
        {
            ++labelImageIterator;
            continue;
        }   

        auto itVS = vesselsSize.find( labelImageIterator.Value() );
        if( itVS == vesselsSize.end()  )
        {
            vesselsSize[labelImageIterator.Value()] = (int)distanceImage->GetPixel( labelImageIterator.GetIndex() );
        }
        else
        {
            if( vesselsSize[labelImageIterator.Value()] < (int)distanceImage->GetPixel( labelImageIterator.GetIndex() ) )
            {
                vesselsSize[labelImageIterator.Value()] = (int)distanceImage->GetPixel( labelImageIterator.GetIndex() );
            }
        }

        ++labelImageIterator;
    }

    for(auto &p : vesselsSize)
    {
        std::cout<<"label:"<<p.first<<"  vessels Size: "<<p.second<<std::endl;
    }

    //propagating labels to vessels
    itk::ImageRegionIterator<VoronoiImageType> voronoiIterator( maskFilter->GetOutput(), maskFilter->GetOutput()->GetLargestPossibleRegion() );
    voronoiIterator.GoToBegin();
    int vesselLabel;
    while( !voronoiIterator.IsAtEnd() )
    {
        vesselLabel = voronoiIterator.Value();
        voronoiIterator.Set(vesselsSize[vesselLabel]);
        ++voronoiIterator;
    }

    using VoronoiWriterType = itk::ImageFileWriter< VoronoiImageType >;
    auto voroWriter = VoronoiWriterType::New();

    voroWriter->SetFileName(voronoiSavePath);
    voroWriter->SetInput( maskFilter->GetOutput() );
    voroWriter->Update();
    
    
    return 0;
}