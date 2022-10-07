#include <iostream>

#include "bench_evaluation.h"
#include "itkImageFileReader.h"
#include <list>

#include "utils.h"
#include "itkEllipseSpatialObject.h"
#include "itkSpatialObjectToImageFilter.h"
#include "itkImageRegionConstIterator.h"

void testFailed(const std::string &message)
{
    std::cout<<message<<std::endl;
}

void testMCC()
{
    using PixelType = short;
    using ImageType = itk::Image<PixelType,3>;
    using GTImageType = itk::Image<PixelType,3>;
    using MaskImageType = itk::Image<PixelType,3>;

    Eval<ImageType,GTImageType,MaskImageType> eval1(0.0f,0.0f,10,0,0,0); // true positives 1, rest 0
    Eval<ImageType,GTImageType,MaskImageType> eval2(0.0f,0.0f,0,10,0,0); // true negatives 1, rest 0
    Eval<ImageType,GTImageType,MaskImageType> eval3(0.0f,0.0f,10,10,0,0); // true positive and true negatives 1, rest 0

    Eval<ImageType,GTImageType,MaskImageType> eval4(0.0f,0.0f,0,0,0,1); // false negatives 1, rest 0
    Eval<ImageType,GTImageType,MaskImageType> eval5(0.0f,0.0f,0,0,1,0); // false positives 1, rest 0
    Eval<ImageType,GTImageType,MaskImageType> eval6(0.0f,0.0f,0,0,1,1); // false positives and false negatives 1, rest 0
    Eval<ImageType,GTImageType,MaskImageType> eval7(0.0f,0.0f,20,30,5,5); // unbalanced good classification
    Eval<ImageType,GTImageType,MaskImageType> eval8(0.0f,0.0f,5,5,20,30); // unbalanced bad classification
    Eval<ImageType,GTImageType,MaskImageType> eval9(0.0f,0.0f,512,469,35,70); // testing versus manually calculated MCC
    

    // no classification error
    std::cout<<"TP only :"<<eval1.matthewsCorrelation()<<std::endl;
    if( eval1.matthewsCorrelation() != 1 )
    {
        testFailed("test 1, metric uncorrect");
    }
    // no classification error
    std::cout<<"TN only :"<<eval2.matthewsCorrelation()<<std::endl;
    if( eval2.matthewsCorrelation() != 1 )
    {
        testFailed("test 2, metric uncorrect");
    }
    // no classification error
    std::cout<<"TP and TN only :"<<eval3.matthewsCorrelation()<<std::endl;
    if( eval3.matthewsCorrelation() != 1 )
    {
        testFailed("test 3, metric uncorrect");
    }

    // classification errors
    std::cout<<"FN only :"<<eval4.matthewsCorrelation()<<std::endl;
    if( eval4.matthewsCorrelation() != -1 )
    {
        testFailed("test 4, metric uncorrect");
    }
    
    // classification errors
    std::cout<<"FP only :"<< eval5.matthewsCorrelation()<<std::endl;
    if( eval5.matthewsCorrelation() != -1 )
    {
        testFailed("test 5, metric uncorrect");
    }
    
    // classification errors
    std::cout<<"FN and FP only :"<< eval6.matthewsCorrelation()<<std::endl;
    if( eval6.matthewsCorrelation() != -1 )
    {
        testFailed("test 6, metric uncorrect");
    }

    // classification unbalance
    std::cout<<"Positive unbalance :"<<eval7.matthewsCorrelation()<<std::endl;
    if( eval7.matthewsCorrelation() <= 0 )
    {
        testFailed("test 7, metric uncorrect");
    }
    // classification unbalance

    std::cout<<"Negative unbalance :"<<eval8.matthewsCorrelation()<<std::endl;
    if( eval8.matthewsCorrelation() >= 0 )
    {
        testFailed("test 8, metric uncorrect");
    }

    // manual number
    std::cout<<"against manual number (prec=0.0000001) TP=512, TN=469, FP=35, FN=70 (MCC=0.80820) :"<<eval9.matthewsCorrelation()<<std::endl;
    if( (eval9.matthewsCorrelation() - 0.808209922491334) >= 0.0000001 )
    {
        std::cout<<(eval9.matthewsCorrelation() - 0.808209922491334)<<std::endl;
        testFailed("test9, uncorrect :" + std::to_string( eval9.matthewsCorrelation() ));
    }

}

void testDice()
{
    using PixelType = short;
    using ImageType = itk::Image<PixelType,3>;
    using GTImageType = itk::Image<PixelType,3>;
    using MaskImageType = itk::Image<PixelType,3>;

    Eval<ImageType,GTImageType,MaskImageType> eval1(0.0f,0.0f,512,469,35,70); // testing versus manually calculated MCC

     // manual number 
    std::cout<<"against manual number (prec=0.0000001) TP=512, TN=469, FP=35, FN=70 (Dice=0.9069973427) :"<<eval1.dice()<<std::endl;
    if( (eval1.dice() - 0.906997342781222) >= 0.0000001 )
    {
        std::cout<<(eval1.dice() - 0.906997342781222)<<std::endl;
        testFailed("test1, uncorrect :" + std::to_string( eval1.dice() ));
    }
}

void testMetrics()
{
    using PixelType = short;
    using ImageType = itk::Image<PixelType,3>;
    using GTImageType = itk::Image<PixelType,3>;
    using MaskImageType = itk::Image<PixelType,3>;

    Eval<ImageType,GTImageType,MaskImageType> eval1(0.0f,0.0f,512,469,35,70); // testing versus manually calculated MCC


    std::cout<<"against manual number (prec=0.0000001) TP=512, TN=469, FP=35, FN=70 (Accuracy=0.9033149) :"<<eval1.accuracy()<<std::endl;
    if( (eval1.accuracy() - 0.903314917127072) >= 0.0000001 )
    {
        std::cout<<(eval1.accuracy() - 0.903314917127072)<<std::endl;
        testFailed("test1, uncorrect :" + std::to_string( eval1.accuracy() ));
    }

    std::cout<<"against manual number (prec=0.0000001) TP=512, TN=469, FP=35, FN=70 (Precision=0.93601462) :"<<eval1.precision()<<std::endl;
    if( (eval1.precision() - 0.93601462522) >= 0.0000001 )
    {
        std::cout<<(eval1.precision() - 0.936014625228519)<<std::endl;
        testFailed("test1, uncorrect :" + std::to_string( eval1.precision() ));
    }
    
    std::cout<<"against manual number (prec=0.0000001) TP=512, TN=469, FP=35, FN=70 (sensitivity=0.8797250859) :"<<eval1.sensitivity()<<std::endl;
    if( (eval1.sensitivity() - 0.8797250859) >= 0.0000001 )
    {
        std::cout<<(eval1.sensitivity() - 0.8797250859)<<std::endl;
        testFailed("test1, uncorrect :" + std::to_string( eval1.sensitivity() ));
    }

    std::cout<<"against manual number (prec=0.0000001) TP=512, TN=469, FP=35, FN=70 (Specificity=0.9305555) :"<<eval1.specificity()<<std::endl;
    if( (eval1.specificity() - 0.930555555 ) >= 0.0000001 )
    {
        std::cout<<(eval1.specificity() - 0.93055555)<<std::endl;
        testFailed("test1, uncorrect :" + std::to_string( eval1.specificity() ));
    }
}

void testEllipses()
{
    using PixelType = unsigned char;
    using ImageType = itk::Image<PixelType,3>;
    using FloatImageType = itk::Image<float,3>;

    ImageType::RegionType::SizeType size;
    
    using EllipseType = itk::EllipseSpatialObject<3>;
    using SpacialObjectToImageFilterType = itk::SpatialObjectToImageFilter<EllipseType,ImageType>;
    
    // define ellipse
    auto ellipse = EllipseType::New();
    EllipseType::ArrayType radiusArray;
    int radius = static_cast<int>( 10 );
    radiusArray[0] = radius;
    radiusArray[1] = radius;
    radiusArray[2] = radius;
    
    std::cout<<radius<<std::endl;

    ImageType::RegionType::SizeType imSize;

    imSize[0] = radius*2 +1 ;
    imSize[1] = radius*2 +1 ;
    imSize[2] = radius*2 +1 ;

    ImageType::SpacingType spacing;
    spacing[0] = 1;
    spacing[1] = 1;
    spacing[2] = 1;

    ImageType::PointType origin;
    origin[0] = 0;//12.5;
    origin[1] = 0;//12.5;
    origin[2] = 0;//12.5;

    auto ellipseToImageFilter = SpacialObjectToImageFilterType::New();
    ellipseToImageFilter->SetSize( imSize );
    ellipseToImageFilter->SetSpacing( spacing );
    ellipseToImageFilter->SetOrigin( origin );

    ellipse->SetRadiusInObjectSpace(radiusArray);
    
    // move the ellipse
    auto transform = EllipseType::TransformType::New();
    transform->SetIdentity();
    EllipseType::TransformType::OutputVectorType translation;
    ImageType::IndexType index;
    //index = it.GetIndex();

    //std::cout<<"index:"<<index<<" radius: "<<radius<<std::endl;

    translation[0] = radius;
    translation[1] = radius;
    translation[2] = radius;
    transform->Translate(translation);

    ellipse->SetObjectToParentTransform(transform);
    
    ellipseToImageFilter->SetInput(ellipse);
    ellipse->SetDefaultInsideValue(255);
    ellipse->SetDefaultOutsideValue(0);
    ellipseToImageFilter->SetUseObjectValue(true);
    ellipseToImageFilter->SetOutsideValue(0);
    ellipseToImageFilter->Update();

    auto imgElle = ellipseToImageFilter->GetOutput();
    itk::ImageRegionIterator<ImageType> itBall(imgElle,imgElle->GetLargestPossibleRegion());
    itBall.GoToBegin();

    // go through an image
    while( !itBall.IsAtEnd() )
    {
        itBall.Value();
        ++itBall;

        
    }


    using OutputWriterType = itk::ImageFileWriter<ImageType>;
    auto writer = OutputWriterType::New();

    writer->SetFileName("testEllipse.nii");
    writer->SetInput( ellipseToImageFilter->GetOutput() );
    try
    {
        writer->Update();
    }
    catch (itk::ExceptionObject & excp)
    {
        std::cerr << excp << std::endl;
        return ;
    }
}

int main(int argc,char** argv)
{
    std::cout<<"-- Test metrics --"<<std::endl;
    std::cout<<"-MCC-"<<std::endl;
    testMCC();
    std::cout<<"-Dice-"<<std::endl;
    testDice();
    std::cout<<"-Acc,Sens,Prec,Spe-"<<std::endl;
    testMetrics();
    std::cout<<"---"<<std::endl;
    testEllipses();
    std::cout<<"test finished"<<std::endl;
    return 0;
}