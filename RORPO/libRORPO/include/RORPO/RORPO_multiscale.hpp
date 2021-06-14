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

#ifndef RORPO_MULTISCALE_INCLUDED
#define RORPO_MULTISCALE_INCLUDED

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "RORPO/pink/rect3dmm.hpp"
#include "RORPO/RORPO.hpp"
#include "RORPO/Algo.hpp"


template<typename PixelType, typename MaskType>
Image3D<PixelType> RORPO_multiscale(const Image3D<PixelType> &I,
                                    const std::vector<int>& S_list,
                                    int nb_core,
                                    int dilationSize,
                                    int debug_flag,
                                    Image3D<MaskType> &Mask)
{

    // ################## Computation of RORPO for each scale ##################

    Image3D<PixelType> Multiscale(I.dimX(), I.dimY(), I.dimZ(),I.spacingX(),I.spacingY(),I.spacingZ(),I.originX(),I.originY(),I.originZ());

	std::vector<int>::const_iterator it;

	for (it=S_list.begin();it!=S_list.end();++it)
	{
        Image3D<PixelType> One_Scale =
                RORPO<PixelType, MaskType>(I, *it, nb_core,dilationSize, Mask);

        // Max of scales
	    max_crush(Multiscale, One_Scale);
	}

    // ----------------- Dynamic Enhancement ---------------
	// Find Max value of output_buffer
	int max_value_RORPO = Multiscale.max_value();
	int max_value_I = I.max_value();

    // Contrast Enhancement
	for ( auto& val : Multiscale.get_data() )
        val = (PixelType)((val / (float)max_value_RORPO ) * max_value_I);

	min_crush(Multiscale, I);

    if (!Mask.empty()) // A mask image is given
	{
		// Application of the non dilated mask to output
        mask_image<PixelType, MaskType>(Multiscale, Mask);
	}
	return Multiscale;
}

#endif // RORPO_MULTISCALE_INCLUDED
