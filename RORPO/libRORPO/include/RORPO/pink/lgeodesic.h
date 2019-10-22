/*
Copyright ESIEE (2009) 

m.couprie@esiee.fr

This software is an image processing library whose purpose is to be
used primarily for research and teaching.

This software is governed by the CeCILL  license under French law and
abiding by the rules of distribution of free software. You can  use, 
modify and/ or redistribute the software under the terms of the CeCILL
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
knowledge of the CeCILL license and that you accept its terms.
*/

#ifndef LGEODESIC__H__
#define LGEODESIC__H__

#include "mccodimage.h"

#ifdef __cplusplus
extern "C" {
#endif
extern int32_t lgeodilat(struct xvimage *g, struct xvimage *f, int32_t connex, int32_t niter) ;
extern int32_t lgeodilat2d(struct xvimage *g, struct xvimage *f, int32_t connex, int32_t niter) ;
extern int32_t lgeodilat2d_short(struct xvimage *g, struct xvimage *f, int32_t connex, int32_t niter) ;
extern int32_t lgeodilat2d_long(struct xvimage *g, struct xvimage *f, int32_t connex, int32_t niter) ;
extern int32_t lgeodilat3d(struct xvimage *g, struct xvimage *f, int32_t connex, int32_t niter) ;
extern int32_t lgeodilat3d_short(struct xvimage *g, struct xvimage *f, int32_t connex, int32_t niter) ;
extern int32_t lgeodilat3d_long(struct xvimage *g, struct xvimage *f, int32_t connex, int32_t niter) ;

extern int32_t lreconsdilat(
        struct xvimage *g,
        struct xvimage *f,
        int32_t connex) ;

extern int32_t lgeoeros(
        struct xvimage *g,
        struct xvimage *f,
        int32_t connex,
        int32_t niter) ;

extern int32_t ldeletecomp(
        struct xvimage *f,
        int32_t connex,
        int32_t x, int32_t y, int32_t z) ;

extern int32_t lselectcomp(
        struct xvimage *f,
        int32_t connex,
        int32_t x, 
        int32_t y, 
        int32_t z) ;

extern int32_t lreconsdilat3d(
        struct xvimage *g,
        struct xvimage *f,
        int32_t connex) ;

extern int32_t lgeoeros3d(
        struct xvimage *g,
        struct xvimage *f,
        int32_t connex,
        int32_t niter) ;

extern int32_t lreconseros3d(
        struct xvimage *g,
        struct xvimage *f,
        int32_t connex) ;

extern int32_t lamont(
        struct xvimage *m,
        struct xvimage *f,
        int32_t connex,
        int32_t strict) ;
#ifdef __cplusplus
}
#endif

#endif /* LGEODESIC__H__ */

