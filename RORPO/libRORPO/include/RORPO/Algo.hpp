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

#ifndef ALGO_INCLUDED
#define ALGO_INCLUDED

#include <stdlib.h>
#include <iostream>
#include <string>

#include "Image/Image.hpp"


// Return the difference between images image1 and image2
template<typename T>
const Image3D<T> diff(const Image3D<T> &image1, const Image3D<T> &image2){

    Image3D<T> result(image1.dimX(), image1.dimY(), image1.dimZ());

    if (image1.size() != image2.size()){
		std::cout<<"Error Diff : Size of images are not the same."<<std::endl;
	}
	else {
        auto it1 = image1.get_data().begin();
        auto it2 = image2.get_data().begin();
        auto it3 = result.get_data().begin();
        for( ; it1 != image1.get_data().end() ; ++it1, ++it2, ++it3 )
            *it3 = *it1 - *it2;
    }
	return result;
}



// Min between image1 and image2. Result is stored in image1.
template<typename T>
int min_crush(Image3D<T> &image1, const Image3D<T> &image2)
{
    if (image1.size() != image2.size()){
        std::cout<<"Error in Algo.hpp (min_crush l 55): "
                   <<"Size of images is not the same."<<std::endl;
	}
	else {
        auto it1 = image1.get_data().begin();
        auto it2 = image2.get_data().begin();
        for( ; it1 != image1.get_data().end() ; ++it1, ++it2 )
            *it1 = std::min( *it1, *it2 );
	}
    return 0;
}


// Max between image1 and image2. Result is stored in image1.
template<typename T>
int max_crush(Image3D<T> &image1, const Image3D<T> &image2)
{
    if (image1.size() != image2.size()){
        std::cout<<"Error in Algo.hpp (max_crush l 76): "
                   <<"Size of images is not the same."<<std::endl;
	}
	else {
        auto it1 = image1.get_data().begin();
        auto it2 = image2.get_data().begin();
        for( ; it1 != image1.get_data().end() ; ++it1, ++it2 )
            *it1 = std::max( *it1, *it2 );
	}
    return 0;
}

// Apply the mask image mask to image image
template<typename T1, typename T2>
void mask_image(Image3D<T1> &image, const Image3D<T2> &mask){
    if (image.size() != mask.size()){
    std::cout<<"Error in Algo.hpp (mask_image l 96): "
               <<"Size of image and mask is not the same."<<std::endl;
	}
	else {
        auto it1 = mask.get_data().begin();
        auto it2 = image.get_data().begin();
        for( ; it1 != mask.get_data().end() ; ++it1, ++it2 )
            if ( *it1 == 0 )
                *it2 = 0;
	}
}

template<typename Iterator>
float computeSTD(Iterator begin, Iterator end)
{
    float mean = 0;
    int k = 0;
    for (auto i = begin; i != end; i++) {
        mean += i->first;
        k++;
    }
    mean /= k + 1;

    float std = 0;
    for (auto i = begin; i != end; i++) {
        std += pow(i->first - mean, 2);
    }
    std /= k + 1;
    std = sqrt(std);

    return std;
}

#endif // ALGO_INCLUDED
