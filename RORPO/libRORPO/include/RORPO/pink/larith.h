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

#ifndef LARITH__H__
#define LARITH__H__

#include "mccodimage.h"

#ifdef __cplusplus
extern "C" {
#endif
extern int32_t ladd(
        struct xvimage *image1,
        struct xvimage *image2
);

extern int32_t laddconst(
        struct xvimage *image1,
        int32_t constante
);

extern int32_t laddconst2(
        struct xvimage *image1,
        double constante
);

extern index_t larea(
  struct xvimage * image
);

extern index_t largmax(struct xvimage * image1);
extern index_t largmin(struct xvimage * image1);

extern int32_t laverage(
  struct xvimage * image1,
  struct xvimage * image2,
  double alpha
);

extern int32_t ldiff(
	struct xvimage * image1,
        struct xvimage * image2
);

extern int32_t ldivide(
	struct xvimage * image1,
        struct xvimage * image2
);

extern int32_t lequal(
	struct xvimage * image1,
        struct xvimage * image2
);

extern int32_t lgammacor(
	struct xvimage * image,
	double gamma
);

extern int32_t linf(
        struct xvimage *image1,
        struct xvimage *image2
);

extern int32_t linverse(
        struct xvimage *image
);

extern int32_t linvert(
        struct xvimage *image
);

extern int32_t lmask(
  struct xvimage * image,
  struct xvimage * mask
);

extern int32_t lmax(
        struct xvimage *image1,
        struct xvimage *image2
);

extern int32_t lmin(
        struct xvimage *image1,
        struct xvimage *image2
);

extern double lmax1(struct xvimage * image1);
extern double lmin1(struct xvimage * image1);
extern int32_t lneg(struct xvimage * image);

extern int32_t lmult(
        struct xvimage *image1,
        struct xvimage *image2
);

extern int32_t lnormalize(
	struct xvimage * image, 
	float nmin, 
	float nmax
);

extern int32_t lnull(
	struct xvimage * image1
);

extern int32_t lscale(
        struct xvimage * image,
        double scale
);

extern int32_t lpow(
	struct xvimage * image,
	double p
);

extern int32_t lexp(
	struct xvimage * image
);

extern int32_t llog(
	struct xvimage * image
);

extern int32_t lsub(
        struct xvimage * image1,
        struct xvimage * image2
);

extern int32_t lsup(
        struct xvimage * image1,
        struct xvimage * image2
);

extern int32_t lvolume(
  struct xvimage * image, 
  double *vol
);

extern int32_t lxor(
        struct xvimage *image1,
        struct xvimage *image2
);

extern int32_t lmodulus(struct xvimage * image, struct xvimage * result);
extern int32_t lreal(struct xvimage * image, struct xvimage * result);
extern int32_t limaginary(struct xvimage * image, struct xvimage * result);

#ifdef __cplusplus
}
#endif

#endif /* LARITH__H__ */
