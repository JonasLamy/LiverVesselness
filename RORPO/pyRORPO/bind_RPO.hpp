#pragma once

#include "Image/Image.hpp"
#include "Image/Image_IO_ITK.hpp"

#include "pyRORPO.hpp"

#include "RORPO/RPO.hpp"

#include<optional>

#define RPO_BINDING(x) \
    m.def("RPO", &RPO_binding<x>, "rpo usage", \
        py::arg("image"), \
        py::arg("scale"), \
        py::arg("spacing") = py::none(), \
        py::arg("origin") = py::none(), \
        py::arg("nbCores") = 1, \
        py::arg("dilationSize") = 3, \
        py::arg("verbose") = false, \
        py::arg("mask") = py::none() \
    ); \

namespace pyRORPO
{
    template<typename PixelType>
    py::array_t<PixelType> RPO_binding(py::array_t<PixelType> imageArray,
                    int scale,
                    std::optional<std::vector<float>> spacingOpt,
                    std::optional<std::vector<double>> originOpt,
                    int nbCores = 1,
                    int dilationSize = 2,
                    int verbose = false,
                    std::optional<py::array_t<PixelType>> maskArray = py::none())
    {
        std::vector<int> window(3);
        window[2] = 0;

        std::vector<float> spacing = spacingOpt ? *spacingOpt : std::vector<float>{1.0, 1.0, 1.0};
        std::vector<double> origin = originOpt ? *originOpt : std::vector<double>{1.0, 1.0, 1.0};

        // ---------------------------- Load image data ----------------------------

        Image3D<PixelType> image = pyarrayToImage3D<PixelType>(imageArray, spacing, origin);

        // -------------------------- mask Image -----------------------------------

        Image3D<PixelType> mask;

        if (maskArray)
            mask = pyarrayToImage3D<PixelType>(*maskArray, spacing, origin);

        // ############################# RPO  ######################################

        // the 7 RPO images with a 2-pixel border
        Image3D<PixelType> RPO1(image.dimX() + 4, image.dimY() + 4, image.dimZ() + 4, 2);
        Image3D<PixelType> RPO2(image.dimX() + 4, image.dimY() + 4, image.dimZ() + 4, 2);
        Image3D<PixelType> RPO3(image.dimX() + 4, image.dimY() + 4, image.dimZ() + 4, 2);
        Image3D<PixelType> RPO4(image.dimX() + 4, image.dimY() + 4, image.dimZ() + 4, 2);
        Image3D<PixelType> RPO5(image.dimX() + 4, image.dimY() + 4, image.dimZ() + 4, 2);
        Image3D<PixelType> RPO6(image.dimX() + 4, image.dimY() + 4, image.dimZ() + 4, 2);
        Image3D<PixelType> RPO7(image.dimX() + 4, image.dimY() + 4, image.dimZ() + 4, 2);

        RPO(image, scale, RPO1, RPO2, RPO3, RPO4, RPO5,RPO6, RPO7, nbCores, dilationSize, mask);

        // ----------------------- Image3D to pyarray ------------------------------

        auto nbDirections = RPO1.dimY();
        nbDirections = 7;

        py::array_t<PixelType> result = py::array_t<PixelType>({nbDirections, RPO1.dimZ(), RPO1.dimY(), RPO1.dimX()});
        
        py::buffer_info buf3 = result.request();

        PixelType* ptr = (PixelType*) buf3.ptr;

        size_t offset = 0;
        memcpy(ptr + offset, RPO1.get_pointer(), RPO1.size() * sizeof(PixelType));
        offset += RPO1.size();
        memcpy(ptr + offset, RPO2.get_pointer(), RPO2.size() * sizeof(PixelType));
        offset += RPO2.size();
        memcpy(ptr + offset, RPO3.get_pointer(), RPO3.size() * sizeof(PixelType));
        offset += RPO3.size();
        memcpy(ptr + offset, RPO4.get_pointer(), RPO4.size() * sizeof(PixelType));
        offset += RPO4.size();
        memcpy(ptr + offset, RPO5.get_pointer(), RPO5.size() * sizeof(PixelType));
        offset += RPO5.size();
        memcpy(ptr + offset, RPO6.get_pointer(), RPO6.size() * sizeof(PixelType));
        offset += RPO6.size();
        memcpy(ptr + offset, RPO7.get_pointer(), RPO7.size() * sizeof(PixelType));
        offset += RPO7.size();

        result.resize({nbDirections, RPO1.dimZ(), RPO1.dimY(), RPO1.dimX()});

        return result;
    }

} // namespace pyRORPO