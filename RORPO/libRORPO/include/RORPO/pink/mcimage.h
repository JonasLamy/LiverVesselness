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

/** Pink

 \brief This file holds the basic image allocation functions.

 \ingroup development

 \file   mcimage.h

 \author Michel Couprie, 2009

*/


#ifndef MCIMAGE__H__
#define MCIMAGE__H__

#ifdef __cplusplus
extern "C" {
#endif


// ********************************************************************************************************
// ********************************************************************************************************
// ********************************************************************************************************
//                                    PLATFORM DEPENDENT CONFIGURATION
// ********************************************************************************************************
// ********************************************************************************************************
// ********************************************************************************************************

#ifdef UNIXIO
#  define __pink__inline inline
#  define __pink__export export
#  define __pink__import
#  include <stdint.h>
#else /* NOT UNIXIO */
#  define __pink__inline
#  define __pink__export __declspec(dllexport)
#  define __pink__import __declspec(dllimport)
  typedef unsigned char u_int8_t;
  typedef unsigned int  u_int32_t;
  typedef unsigned char uint8_t;
  typedef int           int32_t;
  typedef unsigned int  uint32_t;
  typedef long long     int64_t_bis;
  typedef unsigned long long uint64_t_bis;
  typedef unsigned long long u_int64_t_bis;
#endif /* UNIXIO */


// attention : les index doivent être signés (pour les parcours rétro : for(i = N-1; i >=0; i--))
#ifdef MC_64_BITS
typedef int64_t_bis index_t;
#else
typedef int32_t index_t;
#endif
#define HUGE_IMAGE_SIZE INT32_MAX

// ********************************************************************************************************
// ********************************************************************************************************
// ********************************************************************************************************
//                            end of  PLATFORM DEPENDENT CONFIGURATION
// ********************************************************************************************************
// ********************************************************************************************************
// ********************************************************************************************************

/* ============== */
/* prototypes for mcimage.c    */
/* ============== */

/**
\brief Allocates an image object with the given size and type

\param name Not used
\param rs x-size
\param cs y-size
\param ds z-size
\param t t-size
\return The pointer to the image.
*/
extern struct xvimage *allocimage(char * name, index_t rs, index_t cs, index_t ds, int32_t t);

  extern struct xvimage *allocmultimage(char * name, index_t rs, index_t cs, index_t ds, index_t ts, index_t nb, int32_t t);

/**
\brief fills the image with zeros
description Sets every pixel of the image to binary zero.

\param f the input image
\return no return value
*/
extern void razimage(struct xvimage *f);
extern struct xvimage *allocheader(char * name, index_t rs, index_t cs, index_t d, int32_t t);
extern int32_t showheader(char * name);

/**
\brief Frees an image object

\param image The pointer to the image
*/
extern void freeimage(struct xvimage *image);
extern struct xvimage *copyimage(struct xvimage *f);
extern int32_t copy2image(struct xvimage *dest, struct xvimage *source);
extern int32_t equalimages(struct xvimage *im1, struct xvimage *im2);
extern void list2image(struct xvimage * image, double *P, index_t n);
extern double * image2list(struct xvimage * image, index_t *n);

/**
\brief Writes an image to disk.

\param image The pointer to the image
\param filename The file to write the image at.
*/
extern void writeimage(
  struct xvimage * image,
  const char *filename
);

extern void writese(
  struct xvimage * image,
  const char *filename,
  index_t x, index_t y, index_t z
);

extern void writelongimage(
  struct xvimage * image,
  const char *filename
);

extern void writerawimage(
  struct xvimage * image,
  const char *filename
);

extern void writeascimage(
  struct xvimage * image,
  const char *filename
);

extern void printimage(
  struct xvimage * image
);

extern void writergbimage(
  struct xvimage * redimage,
  struct xvimage * greenimage,
  struct xvimage * blueimage,
  const char *filename
);

extern void writergbascimage(
  struct xvimage * redimage,
  struct xvimage * greenimage,
  struct xvimage * blueimage,
  const char *filename
);

/**
\brief Reads an image from a file

\param filename The name of the image file.
\return A Pointer to a newly allocated image.
*/
extern struct xvimage * readimage(
  const char *filename
);

extern struct xvimage * readheader(
  const char *filename
);

extern struct xvimage * readse(const char *filename, index_t *x, index_t *y, index_t*z);

extern struct xvimage * readlongimage(
  const char *filename
);

extern int32_t readrgbimage(
  const char *filename,
  struct xvimage ** r,
  struct xvimage ** g,
  struct xvimage ** b
);

extern int32_t readbmp(
  const char *filename,
  struct xvimage ** r,
  struct xvimage ** g,
  struct xvimage ** b
);

extern void writebmp(
  struct xvimage * redimage,
  struct xvimage * greenimage,
  struct xvimage * blueimage,
  const char *filename
);

extern int32_t readrgb(
  const char *filename,
  struct xvimage ** r,
  struct xvimage ** g,
  struct xvimage ** b
);

extern int32_t convertgen(struct xvimage **f1, struct xvimage **f2);
extern int32_t convertlong(struct xvimage **f1);
extern int32_t convertfloat(struct xvimage **f1);

extern void writelist2(const char *filename, int32_t *x, int32_t *y, int32_t npoints);
extern void writelist3(const char *filename, int32_t *x, int32_t *y, int32_t *z, int32_t npoints);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCIMAGE__H__ */
