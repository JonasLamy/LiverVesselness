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

#ifndef Images_IO_ITK_INCLUDED
#define Images_IO_ITK_INCLUDED

#include <string>
#include <vector>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImportImageFilter.h>
#include <itkImageSeriesReader.h>
#include <itkGDCMImageIO.h>
#include <itkGDCMSeriesFileNames.h>
#include <itkMetaDataDictionary.h>

#include "Image.hpp"
#include <typeinfo>


// ############################# MHA Image ##############################

struct Image3DMetadata {
    itk::IOComponentEnum pixelType;
    std::string pixelTypeString;
    uint nbDimensions;
};


std::optional<Image3DMetadata> Read_Itk_Metadata(const std::string &image_path) {
    itk::ImageIOBase::Pointer imageIO = itk::ImageIOFactory::CreateImageIO(image_path.c_str(),
                                                                           itk::ImageIOFactory::ReadMode);
    if (imageIO == nullptr) {
        std::cerr << "Error: file not found." << std::endl;
        return std::nullopt;
    }
    imageIO->SetFileName(image_path.c_str());
    imageIO->ReadImageInformation();

    return std::optional<Image3DMetadata>(
            {imageIO->GetComponentType(), imageIO->GetComponentTypeAsString(imageIO->GetComponentType()),
             imageIO->GetNumberOfDimensions()});
}

template<typename PixelType>
Image3D<PixelType> Read_Itk_Image(const std::string& image_path)
{
	typedef itk::Image<PixelType, 3> ITKImageType;

	typedef itk::ImageFileReader<ITKImageType> ReaderType;
	typename ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName(image_path);
	reader->Update();

	typename ITKImageType::Pointer itkImage = reader->GetOutput();

	const typename ITKImageType::SizeType& itkSize = itkImage->GetLargestPossibleRegion().GetSize();
	auto spacing = itkImage->GetSpacing();
	auto origin = itkImage->GetOrigin();
	Image3D<PixelType> image(itkSize[0], itkSize[1], itkSize[2],spacing[0],spacing[1],spacing[2],origin[0],origin[1],origin[2]);
    image.add_data_from_pointer(itkImage->GetBufferPointer());

    return image;
}

template<typename PixelType>
Image3D<PixelType> Read_Itk_Image_Series(const std::string& image_path)
{
	typedef itk::Image<PixelType, 3> ITKImageType;

	// Définition des type de l'image
	typedef itk::ImageSeriesReader<ITKImageType> ReaderType;
	typename ReaderType::Pointer reader = ReaderType::New();

	// Définition du dicom
	typedef itk::GDCMImageIO ImageIOType;
	ImageIOType::Pointer dicomIO = ImageIOType::New();
	reader->SetImageIO(dicomIO);

	// Mise en place de la lecture en serie
	typedef itk::GDCMSeriesFileNames NamesGeneratorType;
	NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();
	nameGenerator->SetDirectory(image_path);
	reader->SetFileNames(nameGenerator->GetInputFileNames());

	try
	{
		reader->Update();
	}
	catch( itk::ExceptionObject& ex )
	{
		std::cout << ex.GetDescription();
		return Image3D<PixelType>();
	}

	const typename ITKImageType::Pointer& itkImage = reader->GetOutput();
	const typename ITKImageType::SizeType& itkSize = itkImage->GetLargestPossibleRegion().GetSize();
	auto spacing = itkImage->GetSpacing();
	auto origin = itkImage->GetOrigin();
	Image3D<PixelType> image(itkSize[0], itkSize[1], itkSize[2],spacing[0],spacing[1],spacing[2],origin[0],origin[1],origin[2]);


    image.add_data_from_pointer(itkImage->GetBufferPointer());

    return image;
}


template<typename PixelType>
void Write_Itk_Image( Image3D<PixelType>& image, const std::string& image_path )
{
	typedef itk::Image<PixelType, 3> ITKImageType;

	// Convert image to ITK image
    typedef typename itk::ImportImageFilter<PixelType, 3> ImportImageFilterType;
    typename ImportImageFilterType::Pointer importFilter = ImportImageFilterType::New();

    typename ImportImageFilterType::IndexType start;
    start.Fill(0);
	typename ImportImageFilterType::SizeType size = { image.dimX(), image.dimY(), image.dimZ() };
	typename ImportImageFilterType::RegionType region;
	region.SetIndex(start);
	region.SetSize(size);
	typename ImportImageFilterType::SpacingType origin;
	origin[0] = image.originX();
	origin[1] = image.originY();
	origin[2] = image.originZ();
	typename ImportImageFilterType::SpacingType spacing; 
	spacing[0] = image.spacingX();
	spacing[1] = image.spacingY();
	spacing[2] = image.spacingZ();

	importFilter->SetRegion(region);
	importFilter->SetOrigin(origin);
	importFilter->SetSpacing(spacing);

	const bool importImageFilterWillOwnTheBuffer = false;
	importFilter->SetImportPointer( image.get_pointer(), image.size(), importImageFilterWillOwnTheBuffer );

	// Write ITK image
	typedef itk::ImageFileWriter< ITKImageType  > WriterType;
	typename WriterType::Pointer writer = WriterType::New();
	writer->SetFileName( image_path );
	writer->SetInput( importFilter->GetOutput() );
	writer->Update();
}

#endif // Images_IO_ITK_INCLUDED
