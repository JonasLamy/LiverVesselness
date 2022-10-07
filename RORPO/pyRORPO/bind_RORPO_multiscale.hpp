#pragma once
#include "Image/Image.hpp"
#include "Image/Image_IO_ITK.hpp"

#include "pyRORPO.hpp"

#include "RORPO/RORPO_multiscale.hpp"

#include <optional>

#define RORPO_MULTISCALE_BINDING(x) \
    m.def("RORPO_multiscale", &RORPO_multiscale_binding<x>, "RORPO multiscale", \
        py::arg("image") , \
        py::arg("scaleMin"), \
        py::arg("factor"), \
        py::arg("nbScale"), \
        py::arg("spacing") = py::none(), \
        py::arg("origin") = py::none(), \
        py::arg("nbCores") = 1, \
        py::arg("dilationSize") = 2 , \
        py::arg("verbose") = false, \
        py::arg("mask") = py::none() \
    ); \

namespace pyRORPO
{

    template<typename PixelType>
    py::array_t<PixelType> RORPO_multiscale_binding(py::array_t<PixelType> imageArray,
                    float scaleMin,
                    float factor,
                    int nbScales,
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

        // -------------------------- Scales computation ---------------------------

        std::vector<int> scaleList(nbScales);
        scaleList[0] = scaleMin;

        for (int i = 1; i < nbScales; ++i)
            scaleList[i] = int(scaleMin * pow(factor, i));

        if (verbose){
            std::cout<<"Scales : ";
            std::cout<<scaleList[0];
            for (int i = 1; i < nbScales; ++i)
                std::cout<<','<<scaleList[i];
        }

        // ---------------------------- Load image data ----------------------------

        Image3D<PixelType> image = pyarrayToImage3D<PixelType>(imageArray, spacing, origin);

        if (verbose){
            std::cout << "dimensions: [" << image.dimX() << ", " << image.dimY() << ", " << image.dimZ() << "]" << std::endl;
            std::cout << "spacing: [" << image.spacingX() << ", " << image.spacingY() << ", " << image.spacingZ() << "]" << std::endl;
        }

        // ------------------ Compute input image intensity range ------------------

        std::pair<PixelType,PixelType> minmax = image.min_max_value();

        if (verbose){
            std::cout<< "Image intensity range: "<< (int)minmax.first << ", "
                    << (int)minmax.second << std::endl;
            std::cout<<std::endl;
        }

        // -------------------------- mask Image -----------------------------------

        Image3D<PixelType> mask;

        if (maskArray)
            mask = pyarrayToImage3D<PixelType>(*maskArray, spacing, origin);

        // ---------------------- Run RORPO_multiscale -----------------------------

        Image3D<PixelType> output = RORPO_multiscale<PixelType, PixelType>(image, scaleList, nbCores, dilationSize, verbose, mask);

        return image3DToPyarray<PixelType>(output);
    }

} // namespace pyRORPO