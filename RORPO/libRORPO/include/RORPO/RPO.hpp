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

#ifndef RPO_INCLUDED
#define RPO_INCLUDED

#include <iostream>
#include <omp.h>
#include <vector>

#include "RORPO/sorting.hpp"
#include "RORPO/pink/rect3dmm.hpp"


#include "RORPO/Algo.hpp"
#include "Image/Image.hpp"
#include "RORPO/PO.hpp"

#define OMP


template<typename T, typename MaskType>
void Stuff_PO(Image3D<T> &dilatImageWithBorders,
              std::vector<long> &index_image,
              int L,
              std::vector<bool> &b,
              Image3D<MaskType> &Mask){


    // Sort the grey level intensity in a vector
    index_image = sort_image_value<T,long>(dilatImageWithBorders.get_pointer(),
                                           dilatImageWithBorders.size());


    int new_dimz = dilatImageWithBorders.dimZ();
    int new_dimy = dilatImageWithBorders.dimY();
    int new_dimx = dilatImageWithBorders.dimX();

    // z = 0
    for (int y = 0; y < dilatImageWithBorders.dimY() ; ++y){
        for (int x = 0 ; x < dilatImageWithBorders.dimX() ; ++x){
            b[y*new_dimx+x] = 0;
        }
    }

    //z = dimz-1
    for (int y = 0; y < dilatImageWithBorders.dimY() ; ++y){
        for (int x = 0 ; x < dilatImageWithBorders.dimX() ; ++x){
            b[(new_dimz-1)*new_dimx*new_dimy+y*new_dimx+x] = 0;
        }
    }

    //x = 0
    for (int z = 0 ; z < dilatImageWithBorders.dimZ() ; ++z){
        for (int y = 0 ; y < dilatImageWithBorders.dimY() ; ++y){
            b[z*new_dimx*new_dimy+y*new_dimx] = 0;
        }
    }

    //x = dimx-1
    for (int z = 0 ; z < dilatImageWithBorders.dimZ() ; ++z){
        for (int y = 0 ; y < dilatImageWithBorders.dimY() ; ++y){
            b[z*new_dimx*new_dimy+y*new_dimx+new_dimx-1] = 0;
        }
    }

    // y = 0
    for (int z = 0 ; z < dilatImageWithBorders.dimZ(); ++z){
        for (int x = 0 ; x < dilatImageWithBorders.dimX() ; ++x){
            b[z*new_dimy*new_dimx+x] = 0;
        }
    }

    // y = dimy-1
    for (int z = 0 ; z < dilatImageWithBorders.dimZ() ; ++z){
        for (int x = 0 ; x < dilatImageWithBorders.dimX() ; ++x){
            b[z*new_dimy*new_dimx+(new_dimy-1)*new_dimx+x] = 0;
        }
    }


    // ############################ Mask treatment #############################
    if (!Mask.empty())
    {
        Image3D<MaskType> Mask_dilat = Mask.add_border(2);

        int r_dilat= L/2;

        // Mask dynamic [0 1] ==> [0 255] for the dilation
        int ind=0;
        for(int z = 0; z < Mask_dilat.dimZ(); ++z) {
            for(int y = 0 ; y < Mask_dilat.dimY(); ++y) {
                for(int x = 0; x < Mask_dilat.dimX(); ++x) {
                    if (Mask_dilat(x,y,z) != 0){
                        Mask_dilat(x,y,z) = 255;
                        ind += 1;
                    }
                }
            }
        }

        // Dilation
        rect3dminmax(Mask_dilat.get_pointer(), Mask_dilat.dimX(),
                     Mask_dilat.dimY(), Mask_dilat.dimZ(),
                     r_dilat, r_dilat, r_dilat, false);

        ind = 0;
        for(int z = 0; z < Mask_dilat.dimZ(); ++z) {
            for(int y = 0 ; y < Mask_dilat.dimY(); ++y) {
                for(int x = 0; x < Mask_dilat.dimX(); ++x) {
                    if (Mask_dilat(x,y,z) == 0){
                        b[z*new_dimy*new_dimx+y*new_dimx+x] = 0;
                        ind += 1;
                    }
                }
            }
        }
    }
}




template<typename T, typename MaskType>
void RPO(const Image3D<T> &image, int L, Image3D<T> &RPO1,
         Image3D<T> &RPO2, Image3D<T> &RPO3, Image3D<T> &RPO4,
         Image3D<T> &RPO5,Image3D<T> &RPO6,Image3D<T> &RPO7,
         int nb_core,int dilationSize, Image3D<MaskType> &Mask){


// #################### Definition of the orientations #########################

	// orientation vector
	std::vector<int> orientation1(3);
	orientation1[0] = 1;
	orientation1[1] = 0;
	orientation1[2] = 0;
	std::vector<int> orientation2(3);
	orientation2[0] = 0;
	orientation2[1] = 1;
	orientation2[2] = 0;
	std::vector<int> orientation3(3);
	orientation3[0] = 0;
	orientation3[1] = 0;
	orientation3[2] = 1;
	std::vector<int> orientation4(3);
	orientation4[0] = 1;
	orientation4[1] = 1;
	orientation4[2] = 1;
	std::vector<int> orientation5(3);
	orientation5[0] = 1;
	orientation5[1] = 1;
	orientation5[2] = -1;
	std::vector<int> orientation6(3);
	orientation6[0] = -1;
	orientation6[1] = 1;
	orientation6[2] = 1;
	std::vector<int> orientation7(3);
	orientation7[0] = -1;
	orientation7[1] = 1;
	orientation7[2] = -1;

    // ################### Dilation + Add border on image ######################

	// Dilatation
    Image3D<T> imageDilat=image.copy_image();

    rect3dminmax(imageDilat.get_pointer(), imageDilat.dimX(), imageDilat.dimY(),
                 imageDilat.dimZ(), dilationSize, dilationSize, dilationSize, false);

    Image3D<T> dilatImageWithBorders=imageDilat.add_border(2);
    imageDilat.clear_image();


    RPO1.copy_image(dilatImageWithBorders);
    RPO2.copy_image(dilatImageWithBorders);
    RPO3.copy_image(dilatImageWithBorders);
    RPO4.copy_image(dilatImageWithBorders);
    RPO5.copy_image(dilatImageWithBorders);
    RPO6.copy_image(dilatImageWithBorders);
    RPO7.copy_image(dilatImageWithBorders);


    std::vector<long> index_image;
    std::vector<bool>b(dilatImageWithBorders.size(),1);

    Stuff_PO(dilatImageWithBorders, index_image, L, b, Mask);



    // ############################ COMPUTE PO #################################


	std::cout<<"------- RPO computation with scale " <<L<< "-------"<<std::endl;

    // Calling PO for each orientation
    omp_set_num_threads(nb_core);

    #ifdef OMP
    #pragma omp parallel shared(dilatImageWithBorders, index_image)
	{
		#pragma omp sections nowait
		{
			#pragma omp section
			{
			#endif //OMP
             PO_3D<T, MaskType>(dilatImageWithBorders,
                                L, index_image, orientation1, RPO1, b);
			 std::cout<<"orientation1 1 0 0 : passed"<<std::endl;
			#ifdef OMP
			}

			#pragma omp section
			{
				#endif //OMP

                PO_3D<T, MaskType>(dilatImageWithBorders,
                                   L, index_image, orientation2, RPO2, b);
			    std::cout<<"orientation2 0 1 0 : passed"<<std::endl;
			#ifdef OMP
			}
			#pragma omp section
			{
				#endif
                PO_3D<T, MaskType>(dilatImageWithBorders,
                                   L, index_image, orientation3, RPO3, b);
			    std::cout<<"orientation3 0 0 1 : passed"<<std::endl;

			#ifdef OMP
			}
			#pragma omp section
			{
			 	#endif
                PO_3D<T, MaskType>(dilatImageWithBorders,
                                   L, index_image, orientation4, RPO4, b);
			    std::cout<<"orientation4 1 1 1 : passed"<<std::endl;

			#ifdef OMP
			}
			#pragma omp section
			{
				#endif
                PO_3D<T, MaskType>(dilatImageWithBorders,
                                   L, index_image, orientation5, RPO5, b);
			    std::cout<<"orientation5 1 1 -1 : passed"<<std::endl;

			#ifdef OMP
			}
			#pragma omp section
			{
				#endif
                PO_3D<T, MaskType>(dilatImageWithBorders,
                                   L, index_image, orientation6, RPO6, b);
			    std::cout<<"orientation6 -1 1 1 : passed"<<std::endl;

			#ifdef OMP
			}
			#pragma omp section
			{
				#endif
                PO_3D<T, MaskType>(dilatImageWithBorders,
                                   L, index_image, orientation7, RPO7, b);
			    std::cout<<"orientation7 -1 1 -1 : passed"<<std::endl;

			#ifdef OMP
			}
		}
    }
    #endif

	 std::cout<<"RPO computation completed"<<std::endl;

    dilatImageWithBorders.clear_image();

    // Minimum between the computed RPO on the dilation and the initial image
    // + remove borders

    RPO1.remove_border(2);
    RPO2.remove_border(2);
    RPO3.remove_border(2);
    RPO4.remove_border(2);
    RPO5.remove_border(2);
    RPO6.remove_border(2);
    RPO7.remove_border(2);

    min_crush(RPO1, image);
    min_crush(RPO2, image);
    min_crush(RPO3, image);
    min_crush(RPO4, image);
    min_crush(RPO5, image);
    min_crush(RPO6, image);
    min_crush(RPO7, image);

}

#endif //RPO_INCLUDED
