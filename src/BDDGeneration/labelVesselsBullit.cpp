#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkBinaryThresholdImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkMaskImageFilter.h"
#include "itkSignedMaurerDistanceMapImageFilter.h"
#include "itkDanielssonDistanceMapImageFilter.h"

#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIteratorWithIndex.h"

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

using VoronoiPixelType = PixelType;
using VoronoiImageType = itk::Image<VoronoiPixelType, 3>;


int getNeighbours(  
                    ImageType::IndexType centralIndex, 
                    std::vector<ImageType::IndexType> & neighbours,
                    ImageType::Pointer skeleton,
                    ImageType::RegionType::SizeType imgSize
                    )
{
    ImageType::IndexType neighbourIndex;
     
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
          if(neighbourIndex[2]<0 || neighbourIndex[2]>= imgSize[2] ){continue;}

          if( skeleton->GetPixel( neighbourIndex ) > 0 )
          { 
            neighbours.push_back(neighbourIndex); 
          }
        }

    return neighbours.size();
}

// TODO : template the function
std::vector<LabelImageType::IndexType> getNeighbours(LabelImageType::IndexType centralIndex, 
                LabelImageType::Pointer skeleton,
                ImageType::RegionType::SizeType imgSize )
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

          if(neighbourIndex[0]<0 || neighbourIndex[0]>= imgSize[0] ){continue;}
          if(neighbourIndex[1]<0 || neighbourIndex[1]>= imgSize[1] ){continue;}
          if(neighbourIndex[2]<0 || neighbourIndex[2]>= imgSize[2] ){continue;}

          if( skeleton->GetPixel( neighbourIndex ) > 0 )
          { 
            neighbours.push_back(neighbourIndex); 
          }
        }
    return neighbours;
}


int main(int argc,char** argv)
{


    /* what do we need ??
    1) Inputs
      - input file (binaryVessels)
      - input skeleton (centerline of vessels)
      - input mask file (binary liver)
      - output bifurcation (balls on all bifurcations)
    */

  std::string inputFileName(argv[1]);
  std::string inputSkelName(argv[2]);
  std::string maskFileName(argv[3]);
  std::string gtFileName(argv[4]);
  std::string voronoiSavePath(argv[5]);
  
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
  filterCC->SetInput(skeleton);
  filterCC->Update();
  int nbCC = filterCC->GetObjectCount();
  auto vesselsCC = filterCC->GetOutput();

  std::cout<<"number of connected components :"<<nbCC<<std::endl;
  
  
  std::cout<<"1"<<std::endl;

  
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
  auto resultImage = LabelImageType::New();
  resultImage->SetRegions( binaryVessels->GetLargestPossibleRegion() );
  auto binaryVesselsImageSize = binaryVessels->GetLargestPossibleRegion().GetSize();
  std::cout<<"image size:"<<binaryVesselsImageSize<<std::endl;

  resultImage->SetSpacing(binaryVessels->GetSpacing() );
  resultImage->SetOrigin( binaryVessels->GetOrigin() );
  resultImage->Allocate();
  resultImage->FillBuffer(0);
  std::cout<<"result image spacing \n"<<resultImage->GetSpacing() <<std::endl;
  // creating ellipses

  std::set<ImageType::IndexType> bifurcationsList;
  std::list<std::list<ImageType::IndexType>> skelBranchExtremityList;
  std::list<std::list<ImageType::IndexType>> skelBranchCoreList;

  int label;
  static int uniqueLabel = 1;
  for(int i=0; i<nbCC; i++)
  {
    std::cout<<"best starting point : "<<startingDistance[i]<<" "<<startingIndex[i]<<std::endl;

    // creating element
    // creating index and next label pair

    std::deque<VoxelLabelPair> nextVoxels;
    std::set<ImageType::IndexType> blackList;

    std::vector<ImageType::IndexType> neighbours;
    ImageType::IndexType centralIndex;
    short nbNeighbours = 0;
    int nbBifurcations = 0;
    int nbBifurcationsAmbiguous = 0;
    int nbExtremity=0;
    int CCLength = 1;

    bool noNeighbourBif = true;

    nextVoxels.push_back( std::pair<int,ImageType::IndexType>(uniqueLabel++,startingIndex[i]) );

    while( !nextVoxels.empty() )
    {
      /*************************/
      /* Logic for propagation */

      centralIndex = nextVoxels.front().second;
      label = nextVoxels.front().first;


      //std::cout<<centralIndex<<std::endl;

      nextVoxels.pop_front();
      blackList.insert(centralIndex);
      CCLength++;

      //std::cout<<"neigh"<<std::endl;
      neighbours.clear();
      nbNeighbours = getNeighbours(centralIndex,neighbours,skeleton,binaryVesselsImageSize);
      //std::cout<<"bours"<<std::endl;
      /*************************************************/
      /* Logic of what do we do with the current voxel */

      //std::cout<<"set pixel"<<std::endl;
      resultImage->SetPixel(centralIndex,label);
      //std::cout<<"set pixel ok"<<std::endl;

      if( nbNeighbours == 0) // extremity
      {
        nbExtremity++; 

      }

      if(nbNeighbours == 3 ) // bifurcations
      {
          if(noNeighbourBif)//
          {
            //bifurcations->SetPixel(centralIndex,150);
            nbBifurcations++;

            // add index to bifurcation 
            bifurcationsList.insert(centralIndex);

          }
      }
      if(nbNeighbours > 3) // lump
      {
        nbBifurcationsAmbiguous++;
        //bifurcations->SetPixel(centralIndex,70);
        // add index to bifurcation 
        bifurcationsList.insert(centralIndex);
      }


      for(auto &voxel : neighbours)
      {
        // we don't want duplicates so we look in the waiting list container if voxels is already there
        if( std::find(blackList.begin(),blackList.end(),voxel) == blackList.end() )
        {
            bool notInNextVoxels = true;
            for(auto &item : nextVoxels)
            {
              if(item.second == voxel )
              {
                notInNextVoxels = false;
              }
            }

            if( notInNextVoxels)
            { 
              if(nbNeighbours >= 3)
              {
                auto pair = std::pair<int,ImageType::IndexType>(++uniqueLabel,voxel);
                nextVoxels.push_front( pair );
              }  
              else
              {
                auto pair = std::pair<int,ImageType::IndexType>(label,voxel);
                nextVoxels.push_front( pair ); 
              }

            }
        }
      }

    }

    std::cout<<"CC index:"<<i<<std::endl;
    std::cout<<"Nb bifurcations:"<<nbBifurcations<<std::endl;
    std::cout<<"Nb bifurcations ambiguous:"<<nbBifurcationsAmbiguous<<std::endl;
    std::cout<<"Nb extremity:"<<nbExtremity<<std::endl; 
    std::cout<<"CC length:"<<CCLength<<std::endl;
    std::cout<<std::endl; 

    label++; 
  }

  std::cout<<"labeling finished"<<std::endl;
  using OutputWriterType = itk::ImageFileWriter<LabelImageType>;
  auto writer = OutputWriterType::New();

  writer->SetFileName("lskel_debug.nii.gz");
  writer->SetInput( resultImage );
  writer->Update();


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

std::cout<<"makeLabels"<<std::endl;
std::cout<<"number of labels:"<< listIndexPerLabel.size() <<std::endl;

std::vector<int> labelToBeRemoved;

  for(auto &p : voxelsPerLabel)
  {
      if(p.second == 1)
      {
          int l = p.first;
          auto i = listIndexPerLabel[l][0];
          // get neighbours
          auto neighbours = getNeighbours( i, resultImage, binaryVesselsImageSize );
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

  std::cout<<"Before ----"<<std::endl;
  for(auto &p : voxelsPerLabel)
  {
      std::cout<<"label:"<<p.first<<"  nbVoxels:"<<p.second<<std::endl;
  }

  // delete labels with only 1 member, these are usually voxels in messy bifurcations
  for(auto &l : labelToBeRemoved)
  {
      voxelsPerLabel.erase(l);
      listIndexPerLabel.erase(l);
  }

  std::cout<<"After ----"<<std::endl;
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
  

  std::cout<<"5"<<std::endl;
  using OutputLabelWriterType = itk::ImageFileWriter<LabelImageType>;
  auto resWriter = OutputLabelWriterType::New();

  resWriter->SetFileName(gtFileName);
  resWriter->SetInput( resultImage );
  try
  {
      resWriter->Update();
  }
  catch (itk::ExceptionObject & excp)
  {
      std::cerr << excp << std::endl;
      return EXIT_FAILURE;
  }


  // creating voronoi partition

    using VoronoiFilterType = itk::DanielssonDistanceMapImageFilter<LabelImageType,
                                                           FloatImageType,
                                                           VoronoiImageType>;

    auto voronoiFilter = VoronoiFilterType::New();
    voronoiFilter->SetInput( resultImage );
    voronoiFilter->Update();
    auto voronoiMap = voronoiFilter->GetVoronoiMap();

    // mask voronoi map with vessels

    auto maskLabelFilter = itk::MaskImageFilter<VoronoiImageType,ImageType,VoronoiImageType>::New();
    maskLabelFilter->SetInput( voronoiMap );
    maskLabelFilter->SetMaskImage( binaryVessels );
    maskLabelFilter->SetOutsideValue(0);
    maskLabelFilter->SetMaskingValue(0);
    maskLabelFilter->Update();

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
    itk::ImageRegionIterator<VoronoiImageType> voronoiIterator( maskLabelFilter->GetOutput(), maskLabelFilter->GetOutput()->GetLargestPossibleRegion() );
    voronoiIterator.GoToBegin();
    int vesselLabel;
    while( !voronoiIterator.IsAtEnd() )
    {
        vesselLabel = voronoiIterator.Value();
        if( vesselLabel > 0)
        {
          if( vesselsSize[vesselLabel] < 1)
            voronoiIterator.Set(1);
          else
            voronoiIterator.Set(vesselsSize[vesselLabel]);
        }
        
        ++voronoiIterator;
    }

    using VoronoiWriterType = itk::ImageFileWriter< VoronoiImageType >;
    auto voroWriter = VoronoiWriterType::New();

    voroWriter->SetFileName(voronoiSavePath);
    voroWriter->SetInput( maskLabelFilter->GetOutput() );
    voroWriter->Update();

  return 0;
}