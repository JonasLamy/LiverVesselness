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

#ifndef GEODILATION_INCLUDED
#define GEODILATION_INCLUDED

#include <stdlib.h>
#include <iostream>
#include <string>

#include "RORPO/pink/mcimage.h"
#include "RORPO/pink/mccodimage.h"
#include "RORPO/pink/lgeodesic.h"



template<typename T>
Image3D<T> geodilation(Image3D<T> &G, Image3D<T> &R, int connex, int niter)
{
    Image3D<T> geodilat(G.dimX(), G.dimY(), G.dimZ());

	// Pink Images
    struct xvimage* imageG;
    struct xvimage* imageR;
    struct xvimage* temp;
    int32_t typepixel;

	if (sizeof(T)==1)
   		typepixel = VFF_TYP_1_BYTE;
   	else if (sizeof(T)==2)
		typepixel = VFF_TYP_2_BYTE;
	else if (sizeof(T)==4)
		typepixel = VFF_TYP_4_BYTE;
	else
		std::cerr<<"Error in Geodilation : ImageType not known"<<std::endl;

    imageG=allocheader(NULL,G.dimX(),G.dimY(),G.dimZ(),typepixel);
    imageG->image_data= G.get_pointer();

    imageR=allocheader(NULL,G.dimX(),G.dimY(),G.dimZ(),typepixel);
    imageR->image_data= R.get_pointer();

    temp=copyimage(imageG);

    lgeodilat(temp,imageR,connex,niter);

    for (int z = 0; z<G.dimZ()  ; ++z){
		for (int y = 0; y<G.dimY() ; ++y){
			for (int x = 0; x<G.dimX(); ++x){
                    geodilat(x, y, z) = ((T *)(temp->image_data))[x
                            + y * G.dimX() + z * G.dimX() * G.dimY()];
			}
		}
	}

    free(imageR);
    free(imageG);
    free(temp);

   return geodilat;
}

#endif // GEODILATION_INCLUDED
