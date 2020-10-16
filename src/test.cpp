#include <iostream>

#include "bench_evaluation.h"

#include "itkImageFileReader.h"
#include <list>

void testFailed(const std::string &message)
{
    std::cout<<message<<std::endl;
}

void test1()
{
    using PixelType = short;
    using ImageType = itk::Image<PixelType,3>;
    using GTImageType = itk::Image<PixelType,3>;
    using MaskImageType = itk::Image<PixelType,3>;

    Eval<ImageType,GTImageType,MaskImageType> eval1(10,0,0,0); // true positives 1, rest 0
    Eval<ImageType,GTImageType,MaskImageType> eval2(0,10,0,0); // true negatives 1, rest 0
    Eval<ImageType,GTImageType,MaskImageType> eval3(10,10,0,0); // true positive and true negatives 1, rest 0

    Eval<ImageType,GTImageType,MaskImageType> eval4(0,0,0,1); // false negatives 1, rest 0
    Eval<ImageType,GTImageType,MaskImageType> eval5(0,0,1,0); // false positives 1, rest 0
    Eval<ImageType,GTImageType,MaskImageType> eval6(0,0,1,1); // false positives and false negatives 1, rest 0
    Eval<ImageType,GTImageType,MaskImageType> eval7(20,30,5,5); // unbalanced good classification
    Eval<ImageType,GTImageType,MaskImageType> eval8(5,5,20,30); // unbalanced bad classification

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
    if( eval6.matthewsCorrelation() != 0 )
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
    
}


int main(int argc,char** argv)
{
    test1();
    std::cout<<"test finished"<<std::endl;
    return 0;
}