#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include "Image/Image.hpp"

namespace pyRORPO
{
    template<typename PixelType>
    inline Image3D<PixelType> pyarrayToImage3D(py::array_t<PixelType>& imageInput, std::vector<float>& spacing,
        std::vector<double>& origin)
    {
        auto bufImage = imageInput.request();

        auto image = Image3D<PixelType>(
            bufImage.shape[2], 
            bufImage.shape[1], 
            bufImage.shape[0], 
            spacing[0],
            spacing[1],
            spacing[2],
            origin[0],
            origin[1],
            origin[2]
        );

        image.add_data_from_pointer((PixelType*) bufImage.ptr);

        return image;
    }

    template<typename PixelType>
    inline py::array_t<PixelType> image3DToPyarray(Image3D<PixelType>& image)
    {
        py::array_t<PixelType> result = py::array_t<PixelType>({image.dimZ(), image.dimY(), image.dimX()});
        
        py::buffer_info buf3 = result.request();

        PixelType* ptr = (PixelType*) buf3.ptr;

        memcpy(ptr, image.get_pointer(), image.size() * sizeof(PixelType));

        result.resize({image.dimZ(), image.dimY(), image.dimX()});

        return result;
    }
}