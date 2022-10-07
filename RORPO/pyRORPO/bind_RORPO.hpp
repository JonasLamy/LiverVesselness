#pragma once

#include "Image/Image.hpp"
#include "Image/Image_IO_ITK.hpp"

#include "pyRORPO.hpp"

#include "RORPO/RORPO.hpp"

#include <memory>
#include <optional>

#define RORPO_BINDING(x) \
    m.def("RORPO", &RORPO_binding<x>, "rorpo usage", \
        py::arg("image"), \
        py::arg("scale"), \
        py::arg("spacing") = py::none(), \
        py::arg("origin") = py::none(), \
        py::arg("nbCores") = 1, \
        py::arg("dilationSize") = 2, \
        py::arg("verbose") = false, \
        py::arg("mask") = py::none(), \
        py::arg("directions") = false \
    ); \

namespace pyRORPO
{
    template<typename PixelType>
    py::array_t<PixelType> RORPO_binding(py::array_t<PixelType> imageArray,
                    int scale,
                    std::optional<std::vector<float>> spacingOpt,
                    std::optional<std::vector<double>> originOpt,
                    int nbCores = 1,
                    int dilationSize = 2,
                    int verbose = false,
                    std::optional<py::array_t<PixelType>> maskArray = py::none(),
                    bool directions = false)
    {
        std::vector<int> window(3);
        window[2] = 0;

        std::vector<float> spacing = spacingOpt ? *spacingOpt : std::vector<float>{1.0, 1.0, 1.0};
        std::vector<double> origin = originOpt ? *originOpt : std::vector<double>{1.0, 1.0, 1.0};

        // ---------------------------- Load image data ----------------------------

        Image3D<PixelType> image = pyarrayToImage3D<PixelType>(imageArray, spacing, origin);
        std::cout << "after arr2img: " << image.get_data()[0] << " : " << image.get_data()[1] <<  " : " <<image(2) <<  " : " <<image(3) <<  " : " <<image(4) << " : " << image(5) << std::endl;

        // -------------------------- mask Image -----------------------------------

        Image3D<PixelType> mask;

        if (maskArray)
            mask = pyarrayToImage3D<PixelType>(*maskArray, spacing, origin);

        // ------ Directions setup ----------

        std::shared_ptr<std::vector<int>> directionsResult = nullptr;

        if (directions) {
            directionsResult = std::make_shared<std::vector<int>>(std::vector<int>(image.size() * 3, 0));
        }

        // ---------------------------- Run RORPO ----------------------------------

        Image3D<PixelType> output = RORPO<PixelType, PixelType>(
            image,
            scale,
            nbCores,
            dilationSize,
            //verbose,
            mask,
            directionsResult
        );

        // ---------------------------- Return results ------------------------------

        if (directions) {
            std::vector<PixelType> directionsImgType(directionsResult->begin(), directionsResult->end());

            auto nbDim = output.dimY();
            nbDim = 3;

            py::array_t<PixelType> result = py::array_t<PixelType>({output.dimZ(), output.dimY(), output.dimX(), nbDim});

            py::buffer_info buf3 = result.request();
            PixelType* ptr = (PixelType*) buf3.ptr;

            memcpy(ptr, directionsImgType.data(), directionsImgType.size() * sizeof(PixelType));

            result.resize({output.dimZ(), output.dimY(), output.dimX(), nbDim});

            return result;
        }

        return image3DToPyarray<PixelType>(output);
    }

} // namespace pyRORPO