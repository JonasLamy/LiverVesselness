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
#ifndef RORPO_INCLUDED
#define RORPO_INCLUDED

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <omp.h>
#include <cstdlib>
#include <cmath>

#include "RORPO/sorting.hpp"
#include "RORPO/Algo.hpp"
#include "RORPO/Geodilation.hpp"
#include "RORPO/RPO.hpp"


template<typename T, typename MaskType>
Image3D<T> RORPO(const Image3D<T> &image, int L, int nbCores, int dilationSize, Image3D<MaskType> &mask, std::shared_ptr<std::vector<int>> directions = nullptr) {

    // ############################# RPO  ######################################

    // the 7 RPO images with a 2-pixel border
    Image3D<T> RPO1(image.dimX() + 4, image.dimY() + 4, image.dimZ() + 4, 2);
    Image3D<T> RPO2(image.dimX() + 4, image.dimY() + 4, image.dimZ() + 4, 2);
    Image3D<T> RPO3(image.dimX() + 4, image.dimY() + 4, image.dimZ() + 4, 2);
    Image3D<T> RPO4(image.dimX() + 4, image.dimY() + 4, image.dimZ() + 4, 2);
    Image3D<T> RPO5(image.dimX() + 4, image.dimY() + 4, image.dimZ() + 4, 2);
    Image3D<T> RPO6(image.dimX() + 4, image.dimY() + 4, image.dimZ() + 4, 2);
    Image3D<T> RPO7(image.dimX() + 4, image.dimY() + 4, image.dimZ() + 4, 2);

    auto orientationsRPO = RPO(image, L, RPO1, RPO2, RPO3, RPO4, RPO5, RPO6, RPO7, nbCores, dilationSize, mask);

    // ################### Limit Orientations Treatment #######################

    // ------------------------- Computation of Imin --------------------------

    // ---- Imin limit case 4 orientations ----

    Image3D<T> Imin4(image.dimX(), image.dimY(), image.dimZ());

    //6 combinations for pattern 1
    //c1: horizontal + vertical + diag1 + diag4
    Image3D<T> Imin4_1 = RPO1.copy_image();
    min_crush(Imin4_1, RPO2);
    min_crush(Imin4_1, RPO4);
    min_crush(Imin4_1, RPO7);
    max_crush(Imin4, Imin4_1);
    Imin4_1.clear_image();

    //c2: horizontal + vertical + diag2 + diag3
    Image3D<T> Imin4_2 = RPO1.copy_image();
    min_crush(Imin4_2, RPO2);
    min_crush(Imin4_2, RPO5);
    min_crush(Imin4_2, RPO6);
    max_crush(Imin4, Imin4_2);
    Imin4_2.clear_image();

    //c3: horizontal + profondeur + diag2+ diag4
    Image3D<T> Imin4_3 = RPO1.copy_image();
    min_crush(Imin4_3, RPO3);
    min_crush(Imin4_3, RPO5);
    min_crush(Imin4_3, RPO7);
    max_crush(Imin4, Imin4_3);
    Imin4_3.clear_image();

    //c4: horizontal + profondeur + diag1+ diag3
    Image3D<T> Imin4_4 = RPO1.copy_image();
    min_crush(Imin4_4, RPO3);
    min_crush(Imin4_4, RPO4);
    min_crush(Imin4_4, RPO6);
    max_crush(Imin4, Imin4_4);
    Imin4_4.clear_image();

    //c5: vertical + profondeur + diag3+ diag4
    Image3D<T> Imin4_5 = RPO2.copy_image();
    min_crush(Imin4_5, RPO3);
    min_crush(Imin4_5, RPO6);
    min_crush(Imin4_5, RPO7);
    max_crush(Imin4, Imin4_5);
    Imin4_5.clear_image();

    //c6: vertical + profondeur + diag1+ diag2
    Image3D<T> Imin4_6 = RPO2.copy_image();
    min_crush(Imin4_6, RPO3);
    min_crush(Imin4_6, RPO4);
    min_crush(Imin4_6, RPO5);
    max_crush(Imin4, Imin4_6);
    Imin4_6.clear_image();

    //4 combinations for pattern 2
    //c7: horizontal + vertical + profondeur + diag1
    Image3D<T> Imin4_7 = RPO1.copy_image();
    min_crush(Imin4_7, RPO2);
    min_crush(Imin4_7, RPO3);
    min_crush(Imin4_7, RPO4);
    max_crush(Imin4, Imin4_7);
    Imin4_7.clear_image();

    //c8: horizontal + vertical + profondeur + diag2
    Image3D<T> Imin4_8 = RPO1.copy_image();
    min_crush(Imin4_8, RPO2);
    min_crush(Imin4_8, RPO3);
    min_crush(Imin4_8, RPO5);
    max_crush(Imin4, Imin4_8);
    Imin4_8.clear_image();

    //c9: horizontal + vertical + profondeur + diag3
    Image3D<T> Imin4_9 = RPO1.copy_image();
    min_crush(Imin4_9, RPO2);
    min_crush(Imin4_9, RPO3);
    min_crush(Imin4_9, RPO6);
    max_crush(Imin4, Imin4_9);
    Imin4_9.clear_image();

    //c10: horizontal + vertical + profondeur + diag4
    Image3D<T> Imin4_10 = RPO1.copy_image();
    min_crush(Imin4_10, RPO2);
    min_crush(Imin4_10, RPO3);
    min_crush(Imin4_10, RPO7);
    max_crush(Imin4, Imin4_10);
    Imin4_10.clear_image();

    // ---- Imin limit case 5 orientations ----
    Image3D<T> Imin5 = RPO4.copy_image();
    min_crush(Imin5, RPO5);
    min_crush(Imin5, RPO6);
    min_crush(Imin5, RPO7);


    // ############### Sorting RPO orientations ################################

    Image3D<T> RPOt1 = RPO1.copy_image();
    Image3D<T> RPOt2 = RPO2.copy_image();
    Image3D<T> RPOt3 = RPO3.copy_image();
    Image3D<T> RPOt4 = RPO4.copy_image();
    Image3D<T> RPOt5 = RPO5.copy_image();
    Image3D<T> RPOt6 = RPO6.copy_image();
    Image3D<T> RPOt7 = RPO7.copy_image();

    // Clear Images which are non useful anymore
    RPO1.clear_image();
    RPO2.clear_image();
    RPO3.clear_image();
    RPO4.clear_image();
    RPO5.clear_image();
    RPO6.clear_image();
    RPO7.clear_image();

    // ######################## Compute directions ############################

    // ------------------------ Pointwise Rank Filter -------------------------

    if (directions) {
        for (size_t i = 0; i < orientationsRPO[0].size(); i++) {

            // Pointwise Rank Filter --------------------------------
            std::vector<std::pair<T, int>> pixel{
                std::make_pair(RPOt1(i), 0),
                std::make_pair(RPOt2(i), 1),
                std::make_pair(RPOt3(i), 2),
                std::make_pair(RPOt4(i), 3),
                std::make_pair(RPOt5(i), 4),
                std::make_pair(RPOt6(i), 5),
                std::make_pair(RPOt7(i), 6)
            };

            std::sort(pixel.begin(), pixel.end(),
                [&](const auto& a, const auto& b) {
                    return a.first > b.first;
                });

            float stdMin = 99999999;
            int interestNb = 0; 
            
            // Compute Std and find orientations of interest --------
            for (size_t k = 0; k < 3; k++) {
                float stdP = computeSTD(pixel.begin(), pixel.begin() + k + 1);
                float stdO = computeSTD(pixel.begin() + k + 1, pixel.end());

                float stdSum = stdP + stdO;

                if (stdSum < stdMin) {
                    stdMin = stdSum;
                    interestNb = k;
                }
            }

            std::array<std::vector<int>, 7> orientations = {
                    std::vector<int>{1, 0, 0},//1
                    std::vector<int>{0, 1, 0},//2
                    std::vector<int>{0, 0, 1},//3
                    std::vector<int>{1, 1, 1},//4
                    std::vector<int>{1, 1, -1},//5
                    std::vector<int>{-1, 1, 1},//6
                    std::vector<int>{-1, 1, -1},//7
            };

            // Combine orientations of interest to find direction ---
            std::vector<int> direction = orientations[pixel[0].second];
            for (size_t j = 1; j <= interestNb; j++) {
                std::transform(direction.begin(), direction.end(), orientations[pixel[j].second].begin(),
                    direction.begin(), std::plus<int>());
            }

            std::copy(direction.begin(), direction.end(), directions->begin() + i * 3);
        }
    } // directions

    // Pointwise rank filter
    std::vector<std::array<uint8_t, 7>> o_indices(RPOt1.size()); //orientation indices
    sorting(RPOt1, RPOt2, RPOt3, RPOt4, RPOt5, RPOt6, RPOt7, RPOt1.size(), o_indices);

    // Clear Images which are non useful anymore
    RPOt1.clear_image();
    RPOt5.clear_image();
    RPOt6.clear_image();

    // Compute RORPO without limit orientations
    Image3D<T> RORPO_res = diff(RPOt7, RPOt4);
    RPOt7.clear_image();


    // ----------------------- Computation of Imin2 ----------------------------
    //geodesic reconstruction of RPO6 in RPO4
    Image3D<T> RPO6_geo = geodilation(RPOt2, RPOt4, 18, -1);
    RPOt2.clear_image();

    //geodesic reconstruction of RPO5 in RPO4
    Image3D<T> RPO5_geo = geodilation(RPOt3, RPOt4, 18, -1);;
    RPOt3.clear_image();
    RPOt4.clear_image();

    // ---- Imin2 limit case 4 orientations ----
    Image3D<T> Imin2_4 = Imin4.copy_image();
    min_crush(Imin2_4, RPO5_geo);
    RPO5_geo.clear_image();


    // ---- Imin2 limit case 5 orientations ----
    Image3D<T> Imin2_5 = Imin5.copy_image();
    min_crush(Imin2_5, RPO6_geo);

    RPO6_geo.clear_image();


    // --------------------------- Final Result --------------------------------

    Image3D<T> Diff_Imin4 = diff(Imin4, Imin2_4);
    Image3D<T> Diff_Imin5 = diff(Imin5, Imin2_5);

    Imin4.clear_image();
    Imin2_4.clear_image();
    Imin5.clear_image();
    Imin2_5.clear_image();

    max_crush(RORPO_res, Diff_Imin4);
    max_crush(RORPO_res, Diff_Imin5);

    return RORPO_res;

}

#endif // RORPO_INCLUDED
