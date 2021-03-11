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

#ifndef SORTING_HPP_INCLUDED
#define SORTING_HPP_INCLUDED

#include <algorithm>
#include "Image/Image.hpp"

template<typename PixelType>
static inline void sort7_sorting_network_simple_swap(PixelType **d, std::array<uint8_t, 7>& indices) {

#define SWAP(x, y) { if (*d[y] < *d[x]) { auto maxi = *d[x]; *d[x] = *d[y]; *d[y] = maxi; maxi=indices[x]; indices[x]=indices[y]; indices[y]=maxi;}}

    SWAP(1, 2);
    SWAP(3, 4);
    SWAP(5, 6);
    SWAP(0, 2);
    SWAP(4, 6);
    SWAP(3, 5);
    SWAP(2, 6);
    SWAP(1, 5);
    SWAP(0, 4);
    SWAP(2, 5);
    SWAP(0, 3);
    SWAP(2, 4);
    SWAP(1, 3);
    SWAP(0, 1);
    SWAP(2, 3);
    SWAP(4, 5);

#undef SWAP
}


template<typename PixelType>
static void sorting(Image3D<PixelType> &image1, Image3D<PixelType> &I2,
                    Image3D<PixelType> &I3, Image3D<PixelType> &I4,
                    Image3D<PixelType> &I5, Image3D<PixelType> &I6,
                    Image3D<PixelType> &I7, int N,
                    std::vector<std::array<uint8_t, 7>>& indices) {
    PixelType *d[7];
    d[0] = image1.get_pointer();
    d[1] = I2.get_pointer();
    d[2] = I3.get_pointer();
    d[3] = I4.get_pointer();
    d[4] = I5.get_pointer();
    d[5] = I6.get_pointer();
    d[6] = I7.get_pointer();


    for (int i = 0; i < N; i++) {
        indices[i] = {0, 1, 2, 3, 4, 5, 6};
        sort7_sorting_network_simple_swap(d, indices[i]);
        for (int j = 0; j < 7; j++)
            d[j]++;
    }
}

#ifdef _TEST_SORT_
int main(int argc, char **argv) {
   for (int i=0; i<10; i++){
        PixelType d[7], *b[7];
        for (int j=0;j<7; j++) {d[j]=rand()%256;b[j]=d+j;}
        sort7_sorting_network_simple_swap(b);
        for (int j=0; j<7; j++) std::cout<< d[j] << "--" ;
        std::cout << std::endl;
    }
    int i;
    std::cin >> i;
}
#endif // _TEST_SORT_

template<typename PixelType>
bool my_sorting_function(const PixelType *i, const PixelType *j)
// Input: i, j : two variables containing memory adress pointing to
// PixelType variables.

// Return : True if the variable pointed by i is smaller than the
// variabled pointed by j
{
    return (*i < *j);
}

template<typename PixelType, typename IndexType>
std::vector<IndexType> sort_image_value(PixelType *image, int size)
//  Return pixels index of image sorted according to intensity
{
    std::vector<IndexType> index_image(size);
    std::vector<PixelType *> index_pointer_adress(size);
    IndexType it;
    typename std::vector<PixelType>::iterator it1;
    typename std::vector<PixelType *>::iterator it2;
    typename std::vector<IndexType>::iterator it3;

    // Fill index_pointer_adress with memory adress of variables in image
    for (it = 0, it2 = index_pointer_adress.begin(); it != size; ++it, ++it2) {
        *it2 = &image[it];
    }

    // Sorting adresses according to intensity
    std::sort(index_pointer_adress.begin(), index_pointer_adress.end(),
              my_sorting_function<PixelType>);

    // Conversion from adresses to index of image I
    for (it3 = index_image.begin(), it = 0; it != size; ++it, ++it3) {
        *it3 = static_cast<IndexType>(index_pointer_adress[it] - &image[0]);
    }
    return index_image;
}

#endif // SORTING_HPP_INCLUDED
