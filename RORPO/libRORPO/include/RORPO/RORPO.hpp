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

#include "RORPO/sorting.hpp"
#include "RORPO/Algo.hpp"
#include "RORPO/Geodilation.hpp"
#include "RORPO/RPO.hpp"


template<typename T, typename MaskType>
Image3D<T> RORPO(const Image3D<T> &image, int L, int nbCores,int dilationSize,
                 Image3D<MaskType> &mask)
{

    // ############################# RPO  ######################################

	// the 7 RPO images with a 2-pixel border
    Image3D<T> RPO1(image.dimX() + 4, image.dimY() + 4, image.dimZ() + 4, 2);
    Image3D<T> RPO2(image.dimX() + 4, image.dimY() + 4, image.dimZ() + 4, 2);
    Image3D<T> RPO3(image.dimX() + 4, image.dimY() + 4, image.dimZ() + 4, 2);
    Image3D<T> RPO4(image.dimX() + 4, image.dimY() + 4, image.dimZ() + 4, 2);
    Image3D<T> RPO5(image.dimX() + 4, image.dimY() + 4, image.dimZ() + 4, 2);
    Image3D<T> RPO6(image.dimX() + 4, image.dimY() + 4, image.dimZ() + 4, 2);
    Image3D<T> RPO7(image.dimX() + 4, image.dimY() + 4, image.dimZ() + 4, 2);

    RPO(image, L, RPO1, RPO2, RPO3, RPO4, RPO5,RPO6, RPO7, nbCores,dilationSize, mask);


     // ################### Limit Orientations Treatment #######################

     // ------------------------- Computation of Imin --------------------------

	 // ---- Imin limit case 4 orientations ----

     Image3D<T> Imin4(image.dimX(), image.dimY(), image.dimZ());

	 //horizontal + vertical + diag1 + diag4
     Image3D<T> Imin4_1 = RPO1.copy_image();
	min_crush(Imin4_1, RPO2);
	min_crush(Imin4_1, RPO4);
	min_crush(Imin4_1, RPO7);

	max_crush(Imin4, Imin4_1);
	 Imin4_1.clear_image();

     //horizontal + vertical + diag2 + diag3
     Image3D<T> Imin4_2 = RPO1.copy_image();
	min_crush(Imin4_2, RPO2);
	min_crush(Imin4_2, RPO5);
	min_crush(Imin4_2, RPO6);

	 max_crush(Imin4, Imin4_2);
	 Imin4_2.clear_image();


     //horizontal + profondeur + diag2+ diag4
     Image3D<T> Imin4_3 = RPO1.copy_image();
	min_crush(Imin4_3, RPO3);
	min_crush(Imin4_3, RPO5);
	min_crush(Imin4_3, RPO7);

	max_crush(Imin4, Imin4_3);
	 Imin4_3.clear_image();


     //horizontal + profondeur + diag1+ diag3
     Image3D<T> Imin4_4 = RPO1.copy_image();
	min_crush(Imin4_4, RPO3);
	min_crush(Imin4_4, RPO4);
	min_crush(Imin4_4, RPO6);

	max_crush(Imin4, Imin4_4);
	 Imin4_4.clear_image();

     //vertical + profondeur + diag1+ diag2
     Image3D<T> Imin4_5 = RPO2.copy_image();
	min_crush(Imin4_5, RPO3);
	min_crush(Imin4_5, RPO4);
	min_crush(Imin4_5, RPO5);

	max_crush(Imin4, Imin4_5);
	 Imin4_5.clear_image();

     //vertical + profondeur + diag3+ diag4
     Image3D<T> Imin4_6 = RPO2.copy_image();
	min_crush(Imin4_6, RPO3);
	min_crush(Imin4_6, RPO6);
	min_crush(Imin4_6, RPO7);

	max_crush(Imin4, Imin4_6);
	 Imin4_6.clear_image();


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

    sorting(RPOt1, RPOt2, RPOt3, RPOt4, RPOt5, RPOt6, RPOt7, RPOt1.size());

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
