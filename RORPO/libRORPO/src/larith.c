/*
Copyright ESIEE (2009)

m.couprie@esiee.fr

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
/*
  operations arithmetiques :
    ladd
    laddconst
    larea
    largmax
    largmin
    laverage
    linverse
    ldiff
    ldivide
    lequal
    lexp
    lgamma
    linf
    llog
    lmask
    lmax
    lmax1
    lmin
    lmin1
    lmult
    lneg
    lnormalize
    lnull
    lpow
    lscale
    lsub
    lsup
    lvolume
    lxor
    lmodulus
*/
/* Michel Couprie - juillet 1996 */
/* Camille Couprie - octobre 2002 (xor) */
/* Michel Couprie - d�cembre 2010 (modulus) */
/* Michel Couprie - f�vrier 2011 (gamma) */
/* Michel Couprie - juillet 2012 (argmin, argmax) */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <assert.h>
#include <RORPO/pink/mcutil.h>
#include <RORPO/pink/mcimage.h>
#include <RORPO/pink/mccodimage.h>
#include <RORPO/pink/larith.h>


#define EPSILON 1e-6

/* ==================================== */
int32_t ladd(
  struct xvimage * image1,
  struct xvimage * image2)
/* somme de 2 images */
/* ==================================== */
#undef F_NAME
#define F_NAME "ladd"
{
  index_t i;
  uint8_t *pt1, *pt2;
  int32_t *PT1, *PT2;
  float *FPT1, *FPT2;
  index_t N = rowsize(image1) * colsize(image1) * depth(image1) * tsize(image1) * nbands(image1);

  COMPARE_SIZE(image1, image2);

  if ((datatype(image1) == VFF_TYP_1_BYTE) && (datatype(image2) == VFF_TYP_1_BYTE))
  {
    pt1 = UCHARDATA(image1); pt2 = UCHARDATA(image2);
    for (i = 0; i < N; i++, pt1++, pt2++)
      *pt1 = (uint8_t)mcmin(NDG_MAX,((int32_t)*pt1+(int32_t)*pt2));
  }
  else if ((datatype(image1) == VFF_TYP_4_BYTE) && (datatype(image2) == VFF_TYP_4_BYTE))
  {
    PT1 = SLONGDATA(image1); PT2 = SLONGDATA(image2);
    for (i = 0; i < N; i++, PT1++, PT2++)
      *PT1 = *PT1 + *PT2;
  }
  else if ((datatype(image1) == VFF_TYP_FLOAT) && (datatype(image2) == VFF_TYP_FLOAT))
  {
    FPT1 = FLOATDATA(image1); FPT2 = FLOATDATA(image2);
    for (i = 0; i < N; i++, FPT1++, FPT2++)
      *FPT1 = *FPT1 + *FPT2;
  }
  else if ((datatype(image1) == VFF_TYP_COMPLEX) && (datatype(image2) == VFF_TYP_COMPLEX))
  {
    FPT1 = FLOATDATA(image1); FPT2 = FLOATDATA(image2);
    for (i = 0; i < N+N; i++, FPT1++, FPT2++)
      *FPT1 = *FPT1 + *FPT2;
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* ladd() */

/* ==================================== */
int32_t laddconst(struct xvimage * image1, int32_t constante)
/* ajoute une constante a une image  - seuil si depassement */
/* ==================================== */
#undef F_NAME
#define F_NAME "laddconst"
{
  index_t i;
  uint8_t *pt1;
  uint16_t *spt1;
  int32_t *lpt1;
  index_t N = rowsize(image1) * colsize(image1) * depth(image1) * tsize(image1) * nbands(image1);

  ACCEPTED_TYPES3(image1, VFF_TYP_1_BYTE, VFF_TYP_2_BYTE, VFF_TYP_4_BYTE);

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */
  if (datatype(image1) == VFF_TYP_1_BYTE)
  {
    pt1 = UCHARDATA(image1);
    for (i = 0; i < N; i++, pt1++)
      *pt1 = (uint8_t)mcmin(NDG_MAX, mcmax(NDG_MIN, (int32_t)(*pt1) + constante));
  }
  else if (datatype(image1) == VFF_TYP_2_BYTE)
  {
    spt1 = USHORTDATA(image1);
    for (i = 0; i < N; i++, spt1++)
      *spt1 = (uint16_t)mcmin(USHRT_MAX,mcmax(0,(uint16_t)(*spt1)+constante));
  }
  else if (datatype(image1) == VFF_TYP_4_BYTE)
  {
    lpt1 = SLONGDATA(image1);
    for (i = 0; i < N; i++, lpt1++)
      *lpt1 = (int32_t)mcmin(INT32_MAX,mcmax(INT32_MIN,(int32_t)(*lpt1)+constante));
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }
  return 1;
} /* laddconst() */

/* ==================================== */
int32_t laddconst2(struct xvimage * image1, double constante)
/* ajoute une constante a une image  - seuil si depassement */
/* ==================================== */
#undef F_NAME
#define F_NAME "laddconst"
{
  index_t i;
  float *FPT1;
  index_t N = rowsize(image1) * colsize(image1) * depth(image1) * tsize(image1) * nbands(image1);

  ACCEPTED_TYPES1(image1, VFF_TYP_FLOAT);

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */

  if (datatype(image1) == VFF_TYP_FLOAT)
  {
    FPT1 = FLOATDATA(image1);
    for (i = 0; i < N; i++, FPT1++)
      *FPT1 = *FPT1 + (float)constante;
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }
  return 1;
} /* laddconst2() */

/* ==================================== */
index_t larea(
  struct xvimage * image
// LuM
//,  int32_t *area
// end of LuM
)
/* retourne le nombre d'�l�ments non nuls */
/* ==================================== */
#undef F_NAME
#define F_NAME "larea"
{
  index_t b, a = 0;
  uint8_t *pt;
  int32_t *PT;
  float *FPT;
  index_t i, nb = nbands(image), N = rowsize(image) * colsize(image) * depth(image) * tsize(image) * nb;

  if (nb == 1)
  {
    if (datatype(image) == VFF_TYP_1_BYTE)
    {
      for (pt = UCHARDATA(image), i = 0; i < N; i++, pt++) if (*pt) a++;
    }
    else if (datatype(image) == VFF_TYP_4_BYTE)
    {
      for (PT = SLONGDATA(image), i = 0; i < N; i++, PT++) if (*PT) a++;
    }
    else if (datatype(image) == VFF_TYP_FLOAT)
    {
      for (FPT = FLOATDATA(image), i = 0; i < N; i++, FPT++) if (*FPT != 0.0) a++;
    }
    else
    {
      fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
      return(0);
    }
  }
  else
  {
    if (datatype(image) == VFF_TYP_1_BYTE)
    {
      pt = UCHARDATA(image);
      for (i = 0; i < N; i++)
      {
	for (b = 0; b < nb; b++) if (pt[b*N + i]) break;
	if (b < nb) a++;
      }
    }
    else if (datatype(image) == VFF_TYP_4_BYTE)
    {
      PT = SLONGDATA(image);
      for (i = 0; i < N; i++)
      {
	for (b = 0; b < nb; b++) if (PT[b*N + i]) break;
	if (b < nb) a++;
      }
    }
    else if (datatype(image) == VFF_TYP_FLOAT)
    {
      FPT = FLOATDATA(image);
      for (i = 0; i < N; i++)
      {
	for (b = 0; b < nb; b++) if (FPT[b*N + i] != 0.0) break;
	if (b < nb) a++;
      }
    }
    else
    {
      fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
      return(0);
    }
  }

  //*area = a;
  return a;
} /* larea() */

/* ==================================== */
int32_t laverage(
  struct xvimage * image1,
  struct xvimage * image2,
  double alpha)
/* moyenne ponderee de 2 images */
/* ==================================== */
#undef F_NAME
#define F_NAME "laverage"
{
  index_t i;
  uint8_t *pt1, *pt2;
  int32_t *PT1, *PT2;
  float *FPT1, *FPT2;
  index_t N = rowsize(image1) * colsize(image1) * depth(image1) * tsize(image1) * nbands(image1);

  COMPARE_SIZE(image1, image2);

  if ((datatype(image1) == VFF_TYP_1_BYTE) && (datatype(image2) == VFF_TYP_1_BYTE))
  {
    pt1 = UCHARDATA(image1); pt2 = UCHARDATA(image2);
    for (i = 0; i < N; i++, pt1++, pt2++)
      *pt1 = (uint8_t)((alpha * *pt1) + ((1.0 - alpha) * *pt2));
  }
  else if ((datatype(image1) == VFF_TYP_4_BYTE) && (datatype(image2) == VFF_TYP_4_BYTE))
  {
    PT1 = SLONGDATA(image1); PT2 = SLONGDATA(image2);
    for (i = 0; i < N; i++, PT1++, PT2++)
      *PT1 = (int32_t)((alpha * *PT1) + ((1.0 - alpha) * *PT2));
  }
  else if ((datatype(image1) == VFF_TYP_FLOAT) && (datatype(image2) == VFF_TYP_FLOAT))
  {
    FPT1 = FLOATDATA(image1); FPT2 = FLOATDATA(image2);
    for (i = 0; i < N; i++, FPT1++, FPT2++)
      *FPT1 = (float)((alpha * *FPT1) + ((1.0 - alpha) * *FPT2));
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* laverage() */

/* ==================================== */
int32_t ldiff(
  struct xvimage * image1,
  struct xvimage * image2)
/* difference en valeur absolue de 2 images */
/* ==================================== */
#undef F_NAME
#define F_NAME "ldiff"
{
  index_t i;
  uint8_t *pt1, *pt2;
  int32_t *PT1, *PT2;
  float *FPT1, *FPT2;
  index_t N = rowsize(image1) * colsize(image1) * depth(image1) * tsize(image1) * nbands(image1);

  COMPARE_SIZE(image1, image2);

  if ((datatype(image1) == VFF_TYP_1_BYTE) && (datatype(image2) == VFF_TYP_1_BYTE))
  {
    pt1 = UCHARDATA(image1); pt2 = UCHARDATA(image2);
    for (i = 0; i < N; i++, pt1++, pt2++)
      *pt1 = (uint8_t)mcabs((int32_t)*pt1-(int32_t)*pt2);
  }
  else if ((datatype(image1) == VFF_TYP_4_BYTE) && (datatype(image2) == VFF_TYP_4_BYTE))
  {
    PT1 = SLONGDATA(image1); PT2 = SLONGDATA(image2);
    for (i = 0; i < N; i++, PT1++, PT2++)
      *PT1 = mcabs(*PT1-*PT2);
  }
  else if ((datatype(image1) == VFF_TYP_FLOAT) && (datatype(image2) == VFF_TYP_FLOAT))
  {
    FPT1 = FLOATDATA(image1); FPT2 = FLOATDATA(image2);
    for (i = 0; i < N; i++, FPT1++, FPT2++)
      *FPT1 = (float)fabs((float)(*FPT1)-(float)(*FPT2));
  }
  else if ((datatype(image1) == VFF_TYP_COMPLEX) && (datatype(image2) == VFF_TYP_COMPLEX))
  {
    FPT1 = FLOATDATA(image1); FPT2 = FLOATDATA(image2);
    for (i = 0; i < N+N; i++, FPT1++, FPT2++)
      *FPT1 = (float)fabs((float)(*FPT1)-(float)(*FPT2));
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* ldiff() */

/* ==================================== */
int32_t ldivide(
  struct xvimage * image1,
  struct xvimage * image2)
/* quotient (pixel par pixel) de 2 images */
/* ==================================== */
#undef F_NAME
#define F_NAME "ldivide"
{
  index_t i;
  uint8_t *pt1, *pt2;
  int32_t *PT1, *PT2;
  float *FPT1, *FPT2;
  index_t N = rowsize(image1) * colsize(image1) * depth(image1) * tsize(image1) * nbands(image1);

  COMPARE_SIZE(image1, image2);

  if ((datatype(image1) == VFF_TYP_1_BYTE) && (datatype(image2) == VFF_TYP_1_BYTE))
  {
    pt1 = UCHARDATA(image1); pt2 = UCHARDATA(image2);
    for (i = 0; i < N; i++, pt1++, pt2++)
      if (*pt2 != 0) *pt1 = *pt1 / *pt2; else *pt1 = 0;
  }
  else if ((datatype(image1) == VFF_TYP_4_BYTE) && (datatype(image2) == VFF_TYP_4_BYTE))
  {
    PT1 = SLONGDATA(image1); PT2 = SLONGDATA(image2);
    for (i = 0; i < N; i++, PT1++, PT2++)
      if (*PT2 != 0) *PT1 = *PT1 / *PT2; else *PT1 = 0;
  }
  else if ((datatype(image1) == VFF_TYP_FLOAT) && (datatype(image2) == VFF_TYP_FLOAT))
  {
    FPT1 = FLOATDATA(image1); FPT2 = FLOATDATA(image2);
    for (i = 0; i < N; i++, FPT1++, FPT2++)
      if (*FPT2 != 0.0) *FPT1 = *FPT1 / *FPT2; else *FPT1 = 0.0;
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* ldivide() */

/* ==================================== */
int32_t lequal(
  struct xvimage * image1,
  struct xvimage * image2)
/* test d'egalite de 2 images */
/* ==================================== */
#undef F_NAME
#define F_NAME "lequal"
{
  index_t i;
  uint8_t *pt1, *pt2;
  int32_t *PT1, *PT2;
  float *FPT1, *FPT2;
  index_t N = rowsize(image1) * colsize(image1) * depth(image1) * tsize(image1) * nbands(image1);

  COMPARE_SIZE(image1, image2);

  if ((datatype(image1) == VFF_TYP_1_BYTE) && (datatype(image2) == VFF_TYP_1_BYTE))
  {
    pt1 = UCHARDATA(image1); pt2 = UCHARDATA(image2);
    for (i = 0; i < N; i++, pt1++, pt2++)
      if (*pt1 != *pt2) return 0;
  }
  else if ((datatype(image1) == VFF_TYP_4_BYTE) && (datatype(image2) == VFF_TYP_4_BYTE))
  {
    PT1 = SLONGDATA(image1); PT2 = SLONGDATA(image2);
    for (i = 0; i < N; i++, PT1++, PT2++)
      if (*PT1 != *PT2) return 0;
  }
  else if ((datatype(image1) == VFF_TYP_FLOAT) && (datatype(image2) == VFF_TYP_FLOAT))
  {
    FPT1 = FLOATDATA(image1); FPT2 = FLOATDATA(image2);
    for (i = 0; i < N; i++, FPT1++, FPT2++)
      if (*FPT1 != *FPT2) return 0;
  }
  else if ((datatype(image1) == VFF_TYP_COMPLEX) && (datatype(image2) == VFF_TYP_COMPLEX))
  {
    FPT1 = FLOATDATA(image1); FPT2 = FLOATDATA(image2);
    for (i = 0; i < N+N; i++, FPT1++, FPT2++)
      if (*FPT1 != *FPT2) return 0;
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* lequal() */

/* ==================================== */
int32_t lgammacor(
  struct xvimage * image,
  double gamma)
/*
   correction gamma :
   - normalisation entre 0 et 1
   - elevation � la puissance gamma
   - inverse la normalisation
*/
/* ==================================== */
#undef F_NAME
#define F_NAME "lgammacor"
{
  struct xvimage * flimage;
  float * Fl;
  index_t x, N = rowsize(image) * colsize(image) * depth(image) * tsize(image) * nbands(image);

  flimage = allocmultimage(NULL, rowsize(image), colsize(image), depth(image), tsize(image), nbands(image), VFF_TYP_FLOAT);
  assert(flimage != NULL);
  Fl = FLOATDATA(flimage);

  if (datatype(image) == VFF_TYP_1_BYTE)
  {
    uint8_t *Im;
    uint8_t ndgmin, ndgmax;
    Im = UCHARDATA(image);
    ndgmin = ndgmax = Im[0];
    for (x = 0; x < N; x++)
    {
      if (Im[x] < ndgmin) ndgmin = Im[x];
      else if (Im[x] > ndgmax) ndgmax = Im[x];
    }
    ndgmax = ndgmax - ndgmin;
    if (ndgmax == 0) ndgmax = 1;

    for (x = 0; x < N; x++)
      Fl[x] = (float)(Im[x] - ndgmin) / (float)ndgmax;

    for (x = 0; x < N; x++)
      Fl[x] = (float)pow((double)(Fl[x]), gamma);

    for (x = 0; x < N; x++)
      Im[x] = (uint8_t)(Fl[x]*(ndgmax-ndgmin)) + ndgmin;

  }
  else if (datatype(image) == VFF_TYP_4_BYTE)
  {
    int32_t *Im;
    int32_t ndgmin, ndgmax;
    Im = SLONGDATA(image);
    ndgmin = ndgmax = Im[0];
    for (x = 0; x < N; x++)
    {
      if (Im[x] < ndgmin) ndgmin = Im[x];
      else if (Im[x] > ndgmax) ndgmax = Im[x];
    }
    ndgmax = ndgmax - ndgmin;
    if (ndgmax == 0) ndgmax = 1;

    for (x = 0; x < N; x++)
      Fl[x] = (float)(Im[x] - ndgmin) / (float)ndgmax;

    for (x = 0; x < N; x++)
      Fl[x] = (float)pow((double)(Fl[x]), gamma);

    for (x = 0; x < N; x++)
      Im[x] = (int32_t)(Fl[x]*(ndgmax-ndgmin)) + ndgmin;
  }
  else if (datatype(image) == VFF_TYP_FLOAT)
  {
    float *Im;
    float ndgmin, ndgmax;
    Im = FLOATDATA(image);
    ndgmin = ndgmax = Im[0];
    for (x = 0; x < N; x++)
    {
      if (Im[x] < ndgmin) ndgmin = Im[x];
      else if (Im[x] > ndgmax) ndgmax = Im[x];
    }
    ndgmax = ndgmax - ndgmin;
    if (ndgmax < EPSILON) ndgmax = 1.0;

    for (x = 0; x < N; x++)
      Fl[x] = (Im[x] - ndgmin) / ndgmax;

    for (x = 0; x < N; x++)
      Fl[x] = (float)pow((double)(Fl[x]), gamma);

    for (x = 0; x < N; x++)
      Im[x] = (Fl[x]*(ndgmax-ndgmin)) + ndgmin;
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  freeimage(flimage);

  return 1;
} /* lgammacor() */

/* ==================================== */
int32_t linf(
  struct xvimage * image1,
  struct xvimage * image2)
/* pr�dicat inf pixelwise */
/* ==================================== */
#undef F_NAME
#define F_NAME "linf"
{
  index_t i;
  index_t N = rowsize(image1) * colsize(image1) * depth(image1) * tsize(image1) * nbands(image1);

  COMPARE_SIZE(image1, image2);

  if ((datatype(image1) == VFF_TYP_1_BYTE) && (datatype(image2) == VFF_TYP_1_BYTE))
  {
    uint8_t *pt1, *pt2;
    pt1 = UCHARDATA(image1); pt2 = UCHARDATA(image2);
    for (i = 0; i < N; i++, pt1++, pt2++)
      if (*pt1 <= *pt2) *pt1 = NDG_MAX; else *pt1 = NDG_MIN;
  }
  else if ((datatype(image1) == VFF_TYP_4_BYTE) && (datatype(image2) == VFF_TYP_4_BYTE))
  {
    int32_t *pt1, *pt2;
    pt1 = SLONGDATA(image1); pt2 = SLONGDATA(image2);
    for (i = 0; i < N; i++, pt1++, pt2++)
      if (*pt1 <= *pt2) *pt1 = NDG_MAX; else *pt1 = NDG_MIN;
  }
  else if ((datatype(image1) == VFF_TYP_FLOAT) && (datatype(image2) == VFF_TYP_FLOAT))
  {
    float *pt1, *pt2;
    pt1 = FLOATDATA(image1); pt2 = FLOATDATA(image2);
    for (i = 0; i < N; i++, pt1++, pt2++)
      if (*pt1 <= *pt2) *pt1 = NDG_MAX; else *pt1 = NDG_MIN;
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* linf() */

/* ==================================== */
int32_t lsup(
  struct xvimage * image1,
  struct xvimage * image2)
/* pr�dicat sup pixelwise */
/* ==================================== */
#undef F_NAME
#define F_NAME "lsup"
{
  index_t i;
  index_t N = rowsize(image1) * colsize(image1) * depth(image1) * tsize(image1) * nbands(image1);

  COMPARE_SIZE(image1, image2);

  if ((datatype(image1) == VFF_TYP_1_BYTE) && (datatype(image2) == VFF_TYP_1_BYTE))
  {
    uint8_t *pt1, *pt2;
    pt1 = UCHARDATA(image1); pt2 = UCHARDATA(image2);
    for (i = 0; i < N; i++, pt1++, pt2++)
      if (*pt1 >= *pt2) *pt1 = NDG_MAX; else *pt1 = NDG_MIN;
  }
  else if ((datatype(image1) == VFF_TYP_4_BYTE) && (datatype(image2) == VFF_TYP_4_BYTE))
  {
    int32_t *pt1, *pt2;
    pt1 = SLONGDATA(image1); pt2 = SLONGDATA(image2);
    for (i = 0; i < N; i++, pt1++, pt2++)
      if (*pt1 >= *pt2) *pt1 = NDG_MAX; else *pt1 = NDG_MIN;
  }
  else if ((datatype(image1) == VFF_TYP_FLOAT) && (datatype(image2) == VFF_TYP_FLOAT))
  {
    float *pt1, *pt2;
    pt1 = FLOATDATA(image1); pt2 = FLOATDATA(image2);
    for (i = 0; i < N; i++, pt1++, pt2++)
      if (*pt1 >= *pt2) *pt1 = NDG_MAX; else *pt1 = NDG_MIN;
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* lsup() */

/* ==================================== */
int32_t linverse(
  struct xvimage * image)
/* obsolete - use linvert - left for compatibility */
/* ==================================== */
{
  return linvert(image);
} /* linverse() */

/* ==================================== */
int32_t linvert(
  struct xvimage * image)
/* invert an image */
/* ==================================== */
#undef F_NAME
#define F_NAME "linvert"
{
  index_t i, N = rowsize(image) * colsize(image) * depth(image) * tsize(image) * nbands(image);

  if (datatype(image) == VFF_TYP_1_BYTE)
  {
    uint8_t *pt;
    for (pt = UCHARDATA(image), i = 0; i < N; i++, pt++)
      *pt = NDG_MAX - *pt;
  }

  else if (datatype(image) == VFF_TYP_2_BYTE)
  {
    int16_t *pt, vmax;
    vmax = 0;
    for (pt = SSHORTDATA(image), i = 0; i < N; i++, pt++)
      if (*pt > vmax) vmax = *pt;
    for (pt = SSHORTDATA(image), i = 0; i < N; i++, pt++)
      *pt = vmax - *pt;
  }

  else if (datatype(image) == VFF_TYP_4_BYTE)
  {
    int32_t *pt, vmax;
    vmax = 0;
    for (pt = SLONGDATA(image), i = 0; i < N; i++, pt++)
      if (*pt > vmax) vmax = *pt;
    for (pt = SLONGDATA(image), i = 0; i < N; i++, pt++)
      *pt = vmax - *pt;
  }

  else if (datatype(image) == VFF_TYP_FLOAT)
  {
    float *pt, vmax;
    vmax = 0;
    for (pt = FLOATDATA(image), i = 0; i < N; i++, pt++)
      if (*pt > vmax) vmax = *pt;
    for (pt = FLOATDATA(image), i = 0; i < N; i++, pt++)
      *pt = vmax - *pt;
  }
  else
  {
    fprintf(stderr, "%s: bad image type\n", F_NAME);
    return 0;
  }

  return 1;
} /* linvert() */

/* ==================================== */
int32_t lmask(
  struct xvimage * image,
  struct xvimage * mask)
/* applique un masque binaire */
/* ==================================== */
#undef F_NAME
#define F_NAME "lmask"
{
  index_t i;
  index_t rs, cs, ds, N;
  uint8_t *pt2;

  rs = rowsize(image);
  cs = colsize(image);
  ds = depth(image);
  N = rs * cs * ds;

  if (nbands(image) > 1)
  {
    fprintf(stderr, "%s: multiband images not allowed\n", F_NAME);
    return(0);
  }

  if (tsize(image) > 1)
  {
    fprintf(stderr, "%s: time sequences not allowed\n", F_NAME);
    return(0);
  }

  COMPARE_SIZE(image, mask);
  ACCEPTED_TYPES1(mask, VFF_TYP_1_BYTE);
  pt2 = UCHARDATA(mask);

  if (datatype(image) == VFF_TYP_1_BYTE)
  {
    uint8_t *pt1;
    pt1 = UCHARDATA(image);
    for (i = 0; i < N; i++, pt1++, pt2++)
      if (*pt2 == 0) *pt1 = 0;
  }
  else if (datatype(image) == VFF_TYP_4_BYTE)
  {
    int32_t *PT1;
    PT1 = SLONGDATA(image);
    for (i = 0; i < N; i++, PT1++, pt2++)
      if (*pt2 == 0) *PT1 = 0;
  }
  else if (datatype(image) == VFF_TYP_FLOAT)
  {
    float *FPT1;
    FPT1 = FLOATDATA(image);
    for (i = 0; i < N; i++, FPT1++, pt2++)
      if (*pt2 == 0) *FPT1 = 0;
  }
  else if (datatype(image) == VFF_TYP_COMPLEX)
  {
    fcomplex *CPT1;
    CPT1 = COMPLEXDATA(image);
    for (i = 0; i < N; i++, CPT1++, pt2++)
      if (*pt2 == 0) (*CPT1).re = (*CPT1).im = 0;
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* lmask() */

/* ==================================== */
int32_t lmax(
  struct xvimage * image1,
  struct xvimage * image2)
/* max de 2 images */
/* ==================================== */
#undef F_NAME
#define F_NAME "lmax"
{
  index_t i;
  uint8_t *pt1, *pt2;
  uint16_t *spt1, *spt2;
  int32_t *PT1, *PT2;
  float *FPT1, *FPT2;
  index_t N = rowsize(image1) * colsize(image1) * depth(image1) * tsize(image1) * nbands(image1);

  COMPARE_SIZE(image1, image2);

  if ((datatype(image1) == VFF_TYP_1_BYTE) && (datatype(image2) == VFF_TYP_1_BYTE))
  {
    pt1 = UCHARDATA(image1); pt2 = UCHARDATA(image2);
    for (i = 0; i < N; i++, pt1++, pt2++)
      *pt1 = mcmax(*pt1, *pt2);
  }
  else if ((datatype(image1) == VFF_TYP_2_BYTE) && (datatype(image2) == VFF_TYP_2_BYTE))
  {
    spt1 = USHORTDATA(image1); spt2 = USHORTDATA(image2);
    for (i = 0; i < N; i++, spt1++, spt2++)
      *spt1 = mcmax(*spt1, *spt2);
  }

  else if ((datatype(image1) == VFF_TYP_4_BYTE) && (datatype(image2) == VFF_TYP_4_BYTE))
  {
    PT1 = SLONGDATA(image1); PT2 = SLONGDATA(image2);
    for (i = 0; i < N; i++, PT1++, PT2++)
      *PT1 = mcmax(*PT1, *PT2);
  }
  else if ((datatype(image1) == VFF_TYP_FLOAT) && (datatype(image2) == VFF_TYP_FLOAT))
  {
    FPT1 = FLOATDATA(image1); FPT2 = FLOATDATA(image2);
    for (i = 0; i < N; i++, FPT1++, FPT2++)
      *FPT1 = mcmax(*FPT1, *FPT2);
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* lmax() */

/* ==================================== */
double lmax1(struct xvimage * image1)
/* maximum value in an image */
/* ==================================== */
#undef F_NAME
#define F_NAME "lmax1"
{
  index_t i, N = rowsize(image1) * colsize(image1) * depth(image1) * tsize(image1) * nbands(image1);
  double maxval;

  if (datatype(image1) == VFF_TYP_1_BYTE)
  {
    uint8_t *F = UCHARDATA(image1);
    maxval = (double)F[0];
    for (i = 1; i < N; i++) if ((double)F[i] > maxval) maxval = (double)F[i];
  }
  else if (datatype(image1) == VFF_TYP_4_BYTE)
  {
    int32_t *F = SLONGDATA(image1);
    maxval = (double)F[0];
    for (i = 1; i < N; i++) if ((double)F[i] > maxval) maxval = (double)F[i];
  }
  else if (datatype(image1) == VFF_TYP_FLOAT)
  {
    float *F = FLOATDATA(image1);
    maxval = (double)F[0];
    for (i = 1; i < N; i++) if ((double)F[i] > maxval) maxval = (double)F[i];
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return(0);
  }

  return maxval;
} /* lmax1() */

/* ==================================== */
double lmin1(struct xvimage * image1)
/* minimum value in an image */
/* ==================================== */
#undef F_NAME
#define F_NAME "lmin1"
{
  index_t i, N = rowsize(image1) * colsize(image1) * depth(image1) * tsize(image1) * nbands(image1);
  double minval;

  if (datatype(image1) == VFF_TYP_1_BYTE)
  {
    uint8_t *F = UCHARDATA(image1);
    minval = (double)F[0];
    for (i = 1; i < N; i++) if ((double)F[i] < minval) minval = (double)F[i];
  }
  else if (datatype(image1) == VFF_TYP_2_BYTE)
  {
    int16_t *F = SSHORTDATA(image1);
    minval = (double)F[0];
    for (i = 1; i < N; i++) if ((double)F[i] < minval) minval = (double)F[i];
  }
  else if (datatype(image1) == VFF_TYP_4_BYTE)
  {
    int32_t *F = SLONGDATA(image1);
    minval = (double)F[0];
    for (i = 1; i < N; i++) if ((double)F[i] < minval) minval = (double)F[i];
  }
  else if (datatype(image1) == VFF_TYP_FLOAT)
  {
    float *F = FLOATDATA(image1);
    minval = (double)F[0];
    for (i = 1; i < N; i++) if ((double)F[i] < minval) minval = (double)F[i];
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return(0);
  }

  return minval;
} /* lmin1() */

/* ==================================== */
index_t largmin(struct xvimage * image1)
/* index of the minimum value in an image */
/* ==================================== */
#undef F_NAME
#define F_NAME "largmin"
{
  index_t i, arg, N = rowsize(image1) * colsize(image1) * depth(image1) * tsize(image1) * nbands(image1);
  double minval;

  arg = 0;
  if (datatype(image1) == VFF_TYP_1_BYTE)
  {
    uint8_t *F = UCHARDATA(image1);
    minval = (double)F[0];
    for (i = 1; i < N; i++) if ((double)F[i] < minval) { minval = (double)F[i]; arg = i; }
  }
  else if (datatype(image1) == VFF_TYP_4_BYTE)
  {
    int32_t *F = SLONGDATA(image1);
    minval = (double)F[0];
    for (i = 1; i < N; i++) if ((double)F[i] < minval) { minval = (double)F[i]; arg = i; }
  }
  else if (datatype(image1) == VFF_TYP_FLOAT)
  {
    float *F = FLOATDATA(image1);
    minval = (double)F[0];
    for (i = 1; i < N; i++) if ((double)F[i] < minval) { minval = (double)F[i]; arg = i; }
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return(0);
  }

  return arg;
} /* largmin() */

/* ==================================== */
index_t largmax(struct xvimage * image1)
/* index of the maximum value in an image */
/* ==================================== */
#undef F_NAME
#define F_NAME "largmax"
{
  index_t i, arg, N = rowsize(image1) * colsize(image1) * depth(image1) * tsize(image1) * nbands(image1);
  double maxval;

  arg = 0;
  if (datatype(image1) == VFF_TYP_1_BYTE)
  {
    uint8_t *F = UCHARDATA(image1);
    maxval = (double)F[0];
    for (i = 1; i < N; i++) if ((double)F[i] > maxval) { maxval = (double)F[i]; arg = i; }
  }
  else if (datatype(image1) == VFF_TYP_4_BYTE)
  {
    int32_t *F = SLONGDATA(image1);
    maxval = (double)F[0];
    for (i = 1; i < N; i++) if ((double)F[i] > maxval) { maxval = (double)F[i]; arg = i; }
  }
  else if (datatype(image1) == VFF_TYP_FLOAT)
  {
    float *F = FLOATDATA(image1);
    maxval = (double)F[0];
    for (i = 1; i < N; i++) if ((double)F[i] > maxval) { maxval = (double)F[i]; arg = i; }
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return(0);
  }

  return arg;
} /* largmax() */

/* ==================================== */
int32_t lmin(
  struct xvimage * image1,
  struct xvimage * image2)
/* min de 2 images */
/* ==================================== */
#undef F_NAME
#define F_NAME "lmin"
{
  index_t i;
  uint8_t *pt1, *pt2;
  int32_t *PT1, *PT2;
  uint16_t *SPT1, *SPT2;
  float *FPT1, *FPT2;
  index_t N = rowsize(image1) * colsize(image1) * depth(image1) * tsize(image1) * nbands(image1);
  COMPARE_SIZE(image1, image2);
  if ((datatype(image1) == VFF_TYP_1_BYTE) && (datatype(image2) == VFF_TYP_1_BYTE))
  {
    pt1 = UCHARDATA(image1); pt2 = UCHARDATA(image2);
    for (i = 0; i < N; i++, pt1++, pt2++)
      *pt1 = mcmin(*pt1, *pt2);

  }
  else if ((datatype(image1) == VFF_TYP_2_BYTE) && (datatype(image2) == VFF_TYP_2_BYTE))
  {
    SPT1 = USHORTDATA(image1); SPT2 = USHORTDATA(image2);
    for (i = 0; i < N; i++, SPT1++, SPT2++)
      *SPT1 = mcmin(*SPT1, *SPT2);
  }
  else if ((datatype(image1) == VFF_TYP_4_BYTE) && (datatype(image2) == VFF_TYP_4_BYTE))
  {
    PT1 = SLONGDATA(image1); PT2 = SLONGDATA(image2);
    for (i = 0; i < N; i++, PT1++, PT2++)
      *PT1 = mcmin(*PT1, *PT2);
  }
  else if ((datatype(image1) == VFF_TYP_FLOAT) && (datatype(image2) == VFF_TYP_FLOAT))
  {
    FPT1 = FLOATDATA(image1); FPT2 = FLOATDATA(image2);
    for (i = 0; i < N; i++, FPT1++, FPT2++)
      *FPT1 = mcmin(*FPT1, *FPT2);
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* lmin() */

/* ==================================== */
int32_t lmult(
  struct xvimage * image1,
  struct xvimage * image2)
/* produit de 2 images */
/* ==================================== */
#undef F_NAME
#define F_NAME "lmult"
{
  index_t i, b;
  index_t rs, cs, ds, nb1, nb2, N;
  struct xvimage * tmp;

  rs = rowsize(image1);
  cs = colsize(image1);
  ds = depth(image1);
  N = rs * cs * ds;
  nb1 = nbands(image1);
  nb2 = nbands(image2);

  if ((tsize(image1) != 1) || (tsize(image1) != 1))
  {
    fprintf(stderr, "%s: time sequences not allowed\n", F_NAME);
    return(0);
  }

  if ((nb1 > 1) && (nb2 > 1))
  {
    fprintf(stderr, "%s: only one image may have several bands\n", F_NAME);
    return(0);
  }

  if (nb2 > 1) // the multiband image (if any) must be image1
  {
    tmp = image2;
    image2 = image1;
    image1 = tmp;
  }

  COMPARE_SIZE(image1, image2);

  if ((datatype(image1) == VFF_TYP_1_BYTE) && (datatype(image2) == VFF_TYP_1_BYTE))
  {
    uint8_t *pt1, *pt2;
    pt1 = UCHARDATA(image1);
    for (b = 0; b < nb1; b++)
      for (pt2 = UCHARDATA(image2), i = 0; i < N; i++, pt1++, pt2++)
	*pt1 = (uint8_t)mcmin(NDG_MAX, (int32_t)*pt1 * (int32_t)*pt2);
  }
  else if ((datatype(image1) == VFF_TYP_4_BYTE) && (datatype(image2) == VFF_TYP_4_BYTE))
  {
    int32_t *PT1, *PT2;
    PT1 = SLONGDATA(image1);
    for (b = 0; b < nb1; b++)
      for (PT2 = SLONGDATA(image2), i = 0; i < N; i++, PT1++, PT2++)
	*PT1 = *PT1 * *PT2;
  }
  else if ((datatype(image1) == VFF_TYP_FLOAT) && (datatype(image2) == VFF_TYP_FLOAT))
  {
    float *FPT1, *FPT2;
    FPT1 = FLOATDATA(image1);
    for (b = 0; b < nb1; b++)
      for (FPT2 = FLOATDATA(image2), i = 0; i < N; i++, FPT1++, FPT2++)
	*FPT1 = *FPT1 * *FPT2;
  }
  else if ((datatype(image1) == VFF_TYP_COMPLEX) && (datatype(image2) == VFF_TYP_COMPLEX))
  {
    fcomplex *CPT1, *CPT2;
    CPT1 = COMPLEXDATA(image1);
    for (b = 0; b < nb1; b++)
      for (CPT2 = COMPLEXDATA(image2), i = 0; i < N; i++, CPT1++, CPT2++)
      {
	(*CPT1).re = ((*CPT1).re * (*CPT2).re) - ((*CPT1).im * (*CPT2).im);
	(*CPT1).im = ((*CPT1).re * (*CPT2).im) + ((*CPT1).im * (*CPT2).re);
      }
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* lmult() */

/* ==================================== */
int32_t lneg(
  struct xvimage * image)
/* negation d' une image de booleens */
/* ==================================== */
#undef F_NAME
#define F_NAME "lneg"
{
  index_t i;
  uint8_t *pt;
  index_t N = rowsize(image) * colsize(image) * depth(image) * tsize(image) * nbands(image);

  if (datatype(image) == VFF_TYP_1_BYTE)
  {
    for (pt = UCHARDATA(image), i = 0; i < N; i++, pt++)
      if (*pt) *pt = 0; else *pt = NDG_MAX;
  }
  else
  {
    fprintf(stderr, "%s: bad image type\n", F_NAME);
    return 0;
  }

  return 1;
} /* lneg() */

/* ==================================== */
int32_t lnormalize(struct xvimage * image, float nmin, float nmax)
/* ==================================== */
#undef F_NAME
#define F_NAME "lnormalize"
{
  index_t x;
  index_t N = rowsize(image) * colsize(image) * depth(image) * tsize(image) * nbands(image);

  if (nmin > nmax)
  {
    fprintf(stderr, "%s: bad output range\n", F_NAME);
    return 0;
  }

  if (datatype(image) == VFF_TYP_1_BYTE)
  {
    uint8_t *Im;
    uint8_t ndgmin, ndgmax;
    uint8_t Nmin = arrondi(nmin);
    uint8_t Nmax = arrondi(nmax);
    Im = UCHARDATA(image);
    ndgmin = ndgmax = Im[0];
    for (x = 0; x < N; x++)
    {
      if (Im[x] < ndgmin) ndgmin = Im[x];
      else if (Im[x] > ndgmax) ndgmax = Im[x];
    }
    ndgmax = ndgmax - ndgmin;
    if (ndgmax == 0) ndgmax = 1;
    for (x = 0; x < N; x++)
      Im[x] = Nmin + ((Im[x] - ndgmin) * (Nmax-Nmin)) / ndgmax;
  }
  else if (datatype(image) == VFF_TYP_4_BYTE)
  {
    int32_t *Im;
    int32_t ndgmin, ndgmax;
    int32_t Nmin = arrondi(nmin);
    int32_t Nmax = arrondi(nmax);
    Im = SLONGDATA(image);
    ndgmin = ndgmax = Im[0];
    for (x = 0; x < N; x++)
    {
      if (Im[x] < ndgmin) ndgmin = Im[x];
      else if (Im[x] > ndgmax) ndgmax = Im[x];
    }
    ndgmax = ndgmax - ndgmin;
    if (ndgmax == 0) ndgmax = 1;
    for (x = 0; x < N; x++)
      Im[x] = Nmin + ((Im[x] - ndgmin) * (Nmax-Nmin)) / ndgmax;
  }
  else if (datatype(image) == VFF_TYP_FLOAT)
  {
    float *Im;
    float ndgmin, ndgmax;
    Im = FLOATDATA(image);
    ndgmin = ndgmax = Im[0];
    for (x = 0; x < N; x++)
    {
      if (Im[x] < ndgmin) ndgmin = Im[x];
      else if (Im[x] > ndgmax) ndgmax = Im[x];
    }
    ndgmax = ndgmax - ndgmin;
    if (ndgmax < EPSILON) ndgmax = 1.0;
    for (x = 0; x < N; x++)
      Im[x] = nmin + ((Im[x] - ndgmin) * (nmax-nmin)) / ndgmax;
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} // lnormalize()

/* ==================================== */
int32_t lnull(struct xvimage * image1)
/* test de nullite d'une image */
/* ==================================== */
#undef F_NAME
#define F_NAME "lnull"
{
  index_t i;
  uint8_t *pt1;
  int32_t *PT1;
  float *FPT1;
  index_t N = rowsize(image1) * colsize(image1) * depth(image1) * tsize(image1) * nbands(image1);

  if (datatype(image1) == VFF_TYP_1_BYTE)
  {
    pt1 = UCHARDATA(image1);
    for (i = 0; i < N; i++, pt1++)
      if (*pt1) return 0;
  }
  else if (datatype(image1) == VFF_TYP_4_BYTE)
  {
    PT1 = SLONGDATA(image1);
    for (i = 0; i < N; i++, PT1++)
      if (*PT1) return 0;
  }
  else if (datatype(image1) == VFF_TYP_FLOAT)
  {
    FPT1 = FLOATDATA(image1);
    for (i = 0; i < N; i++, FPT1++)
      if (*FPT1 != 0.0) return 0;
  }
  else if (datatype(image1) == VFF_TYP_COMPLEX)
  {
    FPT1 = FLOATDATA(image1);
    for (i = 0; i < N+N; i++, FPT1++)
      if (*FPT1 != 0.0) return 0;
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* lnull() */

/* ==================================== */
int32_t lscale(
  struct xvimage * image,
  double scale)
/* produit d' une image par un scalaire - seuil a NDG_MAX pour les uint8_t */
/* ==================================== */
#undef F_NAME
#define F_NAME "lscale"
{
  index_t i;
  uint8_t *pt;
  int32_t *PT;
  float *FPT;
  index_t N = rowsize(image) * colsize(image) * depth(image) * tsize(image) * nbands(image);

  /* ---------------------------------------------------------- */
  /* calculs du resultat */
  /* ---------------------------------------------------------- */

  if (datatype(image) == VFF_TYP_1_BYTE)
  {
    for (pt = UCHARDATA(image), i = 0; i < N; i++, pt++)
    {
      *pt = (uint8_t)mcmin(NDG_MAX, (int32_t)(*pt * scale));
    }
  }
  else if (datatype(image) == VFF_TYP_4_BYTE)
  {
    for (PT = SLONGDATA(image), i = 0; i < N; i++, PT++)
    {
      *PT = (int32_t)(*PT * scale);
    }
  }
  else if (datatype(image) == VFF_TYP_FLOAT)
  {
    for (FPT = FLOATDATA(image), i = 0; i < N; i++, FPT++)
    {
      *FPT = (float)(*FPT * scale);
    }
  }
  else if (datatype(image) == VFF_TYP_COMPLEX)
  {
    for (FPT = FLOATDATA(image), i = 0; i < N+N; i++, FPT++)
    {
      *FPT = (float)(*FPT * scale);
    }
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }
  return 1;
} /* lscale() */

/* ==================================== */
int32_t lpow(
  struct xvimage * image,
  double p)
/* elevation � la puissance p - seuil a NDG_MAX pour les uint8_t */
/* ==================================== */
#undef F_NAME
#define F_NAME "lpow"
{
  index_t i;
  uint8_t *pt;
  int32_t *PT;
  float *FPT;
  index_t N = rowsize(image) * colsize(image) * depth(image) * tsize(image) * nbands(image);

  /* ---------------------------------------------------------- */
  /* calcul du resultat */
  /* ---------------------------------------------------------- */

  if (datatype(image) == VFF_TYP_1_BYTE)
  {
    for (pt = UCHARDATA(image), i = 0; i < N; i++, pt++)
    {
      *pt = (uint8_t)mcmin(NDG_MAX,(pow((double)(*pt),p)));
    }
  }
  else if (datatype(image) == VFF_TYP_4_BYTE)
  {
    for (PT = SLONGDATA(image), i = 0; i < N; i++, PT++)
    {
      *PT = (int32_t)pow((double)(*PT),p);
    }
  }
  else if (datatype(image) == VFF_TYP_FLOAT)
  {
    for (FPT = FLOATDATA(image), i = 0; i < N; i++, FPT++)
    {
      *FPT = (float)pow((double)(*FPT),p);
    }
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }
  return 1;
} /* lpow() */

/* ==================================== */
int32_t lexp(struct xvimage * image)
/* exponentiation */
/* ==================================== */
#undef F_NAME
#define F_NAME "lexp"
{
  index_t i;
  float *FPT;
  index_t N = rowsize(image) * colsize(image) * depth(image) * tsize(image) * nbands(image);

  if (datatype(image) == VFF_TYP_FLOAT)
  {
    for (FPT = FLOATDATA(image), i = 0; i < N; i++, FPT++)
    {
      *FPT = (float)exp((double)(*FPT));
    }
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }
  return 1;
} /* lexp() */

/* ==================================== */
int32_t llog(struct xvimage * image)
/* logarithme */
/* ==================================== */
#undef F_NAME
#define F_NAME "llog"
{
  index_t i;
  float *FPT;
  index_t N = rowsize(image) * colsize(image) * depth(image) * tsize(image) * nbands(image);

  if (datatype(image) == VFF_TYP_FLOAT)
  {
    for (FPT = FLOATDATA(image), i = 0; i < N; i++, FPT++)
    {
      *FPT = (float)log((double)(*FPT));
    }
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }
  return 1;
} /* llog() */

/* ==================================== */
int32_t lsub(
  struct xvimage * image1,
  struct xvimage * image2)
/* difference de 2 images */
/* ==================================== */
#undef F_NAME
#define F_NAME "lsub"
{
  index_t i;
  uint8_t *pt1, *pt2;
  uint16_t *pt3, *pt4;
  int32_t *PT1, *PT2;
  float *FPT1, *FPT2;
  index_t N = rowsize(image1) * colsize(image1) * depth(image1) * tsize(image1) * nbands(image1);

  COMPARE_SIZE(image1, image2);

  if ((datatype(image1) == VFF_TYP_1_BYTE) && (datatype(image2) == VFF_TYP_1_BYTE))
  {
    pt1 = UCHARDATA(image1); pt2 = UCHARDATA(image2);
    for (i = 0; i < N; i++, pt1++, pt2++)
      *pt1 = (uint8_t)mcmax(NDG_MIN, (int32_t)*pt1 - (int32_t)*pt2);
  }
  else if ((datatype(image1) == VFF_TYP_2_BYTE) && (datatype(image2) == VFF_TYP_2_BYTE))
  {
    pt3 = USHORTDATA(image1); pt4 = USHORTDATA(image2);
    for (i = 0; i < N; i++, pt3++, pt4++)
      *pt3 = (uint16_t)mcmax(NDG_MIN, (uint16_t)*pt3 - (uint16_t)*pt4);
  }
  else if ((datatype(image1) == VFF_TYP_4_BYTE) && (datatype(image2) == VFF_TYP_4_BYTE))
  {
    PT1 = SLONGDATA(image1); PT2 = SLONGDATA(image2);
    for (i = 0; i < N; i++, PT1++, PT2++)
      *PT1 = (int32_t)mcmax(NDG_MIN, (int32_t)*PT1 - (int32_t)*PT2);
  }
  else if ((datatype(image1) == VFF_TYP_FLOAT) && (datatype(image2) == VFF_TYP_FLOAT))
  {
    FPT1 = FLOATDATA(image1); FPT2 = FLOATDATA(image2);
    for (i = 0; i < N; i++, FPT1++, FPT2++)
      *FPT1 = *FPT1 - *FPT2;
  }
  else if ((datatype(image1) == VFF_TYP_COMPLEX) && (datatype(image2) == VFF_TYP_COMPLEX))
  {
    FPT1 = FLOATDATA(image1); FPT2 = FLOATDATA(image2);
    for (i = 0; i < N+N; i++, FPT1++, FPT2++)
      *FPT1 = *FPT1 - *FPT2;
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* lsub() */

/* ==================================== */
int32_t lvolume(
  struct xvimage * image,
  double *vol)
/* retourne la somme des valeurs de pixels */
/* ==================================== */
#undef F_NAME
#define F_NAME "lvolume"
{
  index_t i;
  uint8_t *pt;
  int32_t *PT;
  float *FPT;
  double fvolume = 0.0;
  index_t N = rowsize(image) * colsize(image) * depth(image) * tsize(image) * nbands(image);

  if (datatype(image) == VFF_TYP_1_BYTE)
    for (pt = UCHARDATA(image), i = 0; i < N; i++, pt++) fvolume += (double)*pt;
  else
  if (datatype(image) == VFF_TYP_4_BYTE)
    for (PT = SLONGDATA(image), i = 0; i < N; i++, PT++) fvolume += (double)*PT;
  else
  if (datatype(image) == VFF_TYP_FLOAT)
    for (FPT = FLOATDATA(image), i = 0; i < N; i++, FPT++) fvolume += (double)*FPT;
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }
  *vol = fvolume;
  return 1;
} /* lvolume() */

/* ==================================== */
int32_t lxor(
  struct xvimage * image1,
  struct xvimage * image2)
/* xor de 2 images */
/* ==================================== */
#undef F_NAME
#define F_NAME "lxor"
{
  index_t i;
  uint8_t *F1, *F2;
  index_t N = rowsize(image1) * colsize(image1) * depth(image1) * tsize(image1) * nbands(image1);

  F1 = UCHARDATA(image1);
  F2 = UCHARDATA(image2);

  COMPARE_SIZE(image1, image2);

  if ((datatype(image1) == VFF_TYP_1_BYTE) && (datatype(image2) == VFF_TYP_1_BYTE))
  {
    for (i = 0; i < N; i++)
      {
        if (((F1[i] == 0) && (F2[i] == 0)) || ((F1[i] != 0) && (F2[i] != 0)))
	  F1[i]=0;
        else
     	  F1[i]=255;
      }
  }
  else
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

  return 1;
} /* lxor() */

//===================================================
//===================================================
// COMPLEX TYPE IMAGES
//===================================================
//===================================================

/* ==================================== */
int32_t lmodulus(struct xvimage * image, struct xvimage * result)
/* ==================================== */
// modulus of a complex
// result image must have been allocated before calling
#undef F_NAME
#define F_NAME "lmodulus"
{
  index_t i;
  index_t N = rowsize(image) * colsize(image) * depth(image) * tsize(image) * nbands(image);
  fcomplex *F;
  float *R;
  double t1, t2;

  assert(result != NULL);
  COMPARE_SIZE(image, result);
  ACCEPTED_TYPES1(result, VFF_TYP_FLOAT);
  ACCEPTED_TYPES1(image, VFF_TYP_COMPLEX);

  F = COMPLEXDATA(image);
  R = FLOATDATA(result);
  for (i = 0; i < N; i++)
  {
    t1 = (double)F[i].re;
    t2 = (double)F[i].im;
    t1 = sqrt(t1*t1 + t2*t2);
    R[i] = (float)t1;
  }
  return 1;
} /* lmodulus() */

/* ==================================== */
int32_t lreal(struct xvimage * image, struct xvimage * result)
/* ==================================== */
// real part of a complex
// result image must have been allocated before calling
#undef F_NAME
#define F_NAME "lreal"
{
  index_t i;
  index_t N = rowsize(image) * colsize(image) * depth(image) * tsize(image) * nbands(image);
  fcomplex *F;
  float *R;

  assert(result != NULL);
  COMPARE_SIZE(image, result);
  ACCEPTED_TYPES1(result, VFF_TYP_FLOAT);
  ACCEPTED_TYPES1(image, VFF_TYP_COMPLEX);

  N = rowsize(image) * colsize(image) * depth(image);
  F = COMPLEXDATA(image);
  R = FLOATDATA(result);
  for (i = 0; i < N; i++) R[i] = F[i].re;
  return 1;
} /* lreal() */

/* ==================================== */
int32_t limaginary(struct xvimage * image, struct xvimage * result)
/* ==================================== */
// imaginary part of a complex
// result image must have been allocated before calling
#undef F_NAME
#define F_NAME "limaginary"
{
  index_t i;
  index_t N = rowsize(image) * colsize(image) * depth(image) * tsize(image) * nbands(image);
  fcomplex *F;
  float *R;

  assert(result != NULL);
  COMPARE_SIZE(image, result);
  ACCEPTED_TYPES1(result, VFF_TYP_FLOAT);
  ACCEPTED_TYPES1(image, VFF_TYP_COMPLEX);

  N = rowsize(image) * colsize(image) * depth(image);
  F = COMPLEXDATA(image);
  R = FLOATDATA(result);
  for (i = 0; i < N; i++) R[i] = F[i].im;
  return 1;
} /* limaginary() */
