/*
 * File:		rect3dmm.hpp
 *

 Hugues Talbot	 7 Dec 2010

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
 *
*/

/* rect3dminmax.cpp */

#ifndef RECT3DMM_HPP
#define RECT3DMM_HPP

#include <math.h>
#include <stdlib.h>

#include "liarp.h"
#include "genfmin.hpp"
#include "genfmax.hpp"

/**
 * \brief function to replace each voxel within a 3d image with
    max/min within a given 3d structuring element (rectangular)

    This function performs erosion or dilation using the three dimensional
    equivelant of a rectangular structuring element.  The original image is
    described both by the image itself (*in) and the dimensions (nx, ny and
    nz) while the structuring element is described by its width, breadth and
    depth (w, b and d).  The (*func)() argument is then used to specify
    either erosion of dilation (genfmin for erosion and genfmax for dilation).
    Note that this function does not create a new image but rather modifies the
    image supplied (*in).

	TESTS: Tested on a number of images

	Notes:   Terminology needs correcting (ie slice, rect3d etc)

 * \param *in:            input image
 * \param nx:             number of columns in input image
 * \param ny:             number of rows in input image
 * \param nz:             number of slices in input image
 * \param w:              width (x dimension) of SE
 * \param b:              breadth (y dimension) of SE
 * \param d:              depth (z dimension) of SE
 * \param (*func)():      min or max operation (genfmin/genfmax)

 * \return
 * \author Ian Sowden
 * <br> Based on existing rectminmax function.
 * \date 18/12/97
*/

template <typename Type>
void rect3dminmax(Type *in, int nx, int ny, int nz, int w, int b,
		int d, bool usemin )
{
  int        i, j;               /* indexing variables */
  int        maxdim;             /* maximum dimension */
  Type      *row, *col, *slice; /* row/col/slice pointers */
  Type      *h, *g;             /* fowards and backwards arrays (for func) */
  long      *p;                 /* row/col/slice offset array */

  /* Calculate max dimension */
  maxdim = nx<ny?ny:nx;
  maxdim = maxdim<nz?nz:maxdim;

  /* Allocate memory for max/min and offset buffers */
  /* calloc() args swapped */
  g = (Type *)calloc(maxdim, sizeof(Type));
  h = (Type *)calloc(maxdim, sizeof(Type));
  p = (long *)calloc(maxdim, sizeof(long));


  /* set row, col and slice buffers to start of image buffer */
  row = col = slice = in;

  /* if width of SE > 1 then perform max/min on each row */
  if (w > 1) {
    /* set row element offsets */
    for (p[0]=0, i=1; i<nx; ++i)
      p[i] = 1 + p[i-1];

    /* gen max/min for each row (within each slice) */
    if (usemin) {
        for(i=0; i<(ny*nz); ++i, row+=nx)
            genfmin(row, g, h, p, nx, w);
    } else {
        for(i=0; i<(ny*nz); ++i, row+=nx)
            genfmax(row, g, h, p, nx, w);
    }
  } /* end if (w>1) */

  /* if y dimension of SE > 1 then perform max/min on each column */
  if (b > 1) {
    /* set column element offsets */
    for (p[0]=0, i=1; i<ny; ++i)
      p[i] = nx + p[i-1];

    if (usemin) {
        for (i=0; i<nz; ++i, col+=((nx*ny)-nx))
            for (j=0; j<nx; ++j, ++col)
                genfmin(col, g, h, p, ny, b);
    } else {
        for (i=0; i<nz; ++i, col+=((nx*ny)-nx))
            for (j=0; j<nx; ++j, ++col)
                genfmax(col, g, h, p, ny, b);
    }
    /* gen max/min for each column (within each slice) */
  } /* end if (b>1) */

  /* finally, if depth of SE > 1 then perform max/min on each slice */
  if (d > 1) {
    /* set slice element offsets */
    for (p[0]=0, i=1; i<nz; ++i)
      p[i] = (nx*ny) + p[i-1];
    /* gen max/min for each slice */
    if (usemin) {
        for (i=0; i<(nx*ny); ++i, ++slice)
            genfmin(slice, g, h, p, nz, d);
    } else {
        for (i=0; i<(nx*ny); ++i, ++slice)
            genfmax(slice, g, h, p, nz, d);
    }
  } /* end if (d>1) */

  /* clean up */
  free(g);
  free(h);
  free(p);

} /* end rect3dminmax */

#if 0
void rect3dminmax_CHAR(PIX_TYPE *in, int nx, int ny, int nz, int w, int b,
		int d, void (*func) () )
{
  int        i, j;               /* indexing variables */
  int        maxdim;             /* maximum dimension */
  PIX_TYPE   *row, *col, *slice; /* row/col/slice pointers */
  PIX_TYPE   *h, *g;             /* fowards and backwards arrays (for func) */
  INT4_TYPE  *p;                 /* row/col/slice offset array */

  /* Calculate max dimension */
  maxdim = nx<ny?ny:nx;
  maxdim = maxdim<nz?nz:maxdim;

  /* Allocate memory for max/min and offset buffers */
  /* calloc() args swapped */
  g = (PIX_TYPE *)calloc(maxdim, sizeof(PIX_TYPE));
  h = (PIX_TYPE *)calloc(maxdim, sizeof(PIX_TYPE));
  p = (INT4_TYPE *)calloc(maxdim, sizeof(INT4_TYPE));


  /* set row, col and slice buffers to start of image buffer */
  row = col = slice = in;

  /* if width of SE > 1 then perform max/min on each row */
  if (w > 1) {
    /* set row element offsets */
    for (p[0]=0, i=1; i<nx; ++i)
      p[i] = 1 + p[i-1];

    /* gen max/min for each row (within each slice) */
    for(i=0; i<(ny*nz); ++i, row+=nx)
      (*func)(row, g, h, p, nx, w);

  } /* end if (w>1) */

  /* if y dimension of SE > 1 then perform max/min on each column */
  if (b > 1) {
    /* set column element offsets */
    for (p[0]=0, i=1; i<ny; ++i)
      p[i] = nx + p[i-1];

    /* gen max/min for each column (within each slice) */
    for (i=0; i<nz; ++i, col+=((nx*ny)-nx))
      for (j=0; j<nx; ++j, ++col)
	(*func)(col, g, h, p, ny, b);

  } /* end if (b>1) */

  /* finally, if depth of SE > 1 then perform max/min on each slice */
  if (d > 1) {
    /* set slice element offsets */
    for (p[0]=0, i=1; i<nz; ++i)
      p[i] = (nx*ny) + p[i-1];

    /* gen max/min for each slice */
    for (i=0; i<(nx*ny); ++i, ++slice)
      (*func)(slice, g, h, p, nz, d);

  } /* end if (d>1) */

  /* clean up */
  free(g);
  free(h);
  free(p);

} /* end rect3dminmax */

#endif

//void rect3dminmax_int4(INT4_TYPE *in, int nx, int ny, int nz, int w, int b,
//		int d, void (*func) () )
//{
//  int        i, j;               /* indexing variables */
//  int        maxdim;             /* maximum dimension */
//  INT4_TYPE   *row, *col, *slice; /* row/col/slice pointers */
//  INT4_TYPE   *h, *g;             /* fowards and backwards arrays (for func) */
//  INT4_TYPE  *p;                 /* row/col/slice offset array */
//
//  /* Calculate max dimension */
//  maxdim = nx<ny?ny:nx;
//  maxdim = maxdim<nz?nz:maxdim;
//
//  /* Allocate memory for max/min and offset buffers */
//  /* calloc() args swapped */
//  g = (INT4_TYPE *)calloc(maxdim, sizeof(INT4_TYPE));
//  h = (INT4_TYPE *)calloc(maxdim, sizeof(INT4_TYPE));
//  p = (INT4_TYPE *)calloc(maxdim, sizeof(INT4_TYPE));
//
//
//  /* set row, col and slice buffers to start of image buffer */
//  row = col = slice = in;
//
//  /* if width of SE > 1 then perform max/min on each row */
//  if (w > 1) {
//    /* set row element offsets */
//    for (p[0]=0, i=1; i<nx; ++i)
//      p[i] = 1 + p[i-1];
//
//    /* gen max/min for each row (within each slice) */
//    for(i=0; i<(ny*nz); ++i, row+=nx)
//      (*func)(row, g, h, p, nx, w);
//
//  } /* end if (w>1) */
//
//  /* if y dimension of SE > 1 then perform max/min on each column */
//  if (b > 1) {
//    /* set column element offsets */
//    for (p[0]=0, i=1; i<ny; ++i)
//      p[i] = nx + p[i-1];
//
//    /* gen max/min for each column (within each slice) */
//    for (i=0; i<nz; ++i, col+=((nx*ny)-nx))
//      for (j=0; j<nx; ++j, ++col)
//	(*func)(col, g, h, p, ny, b);
//
//  } /* end if (b>1) */
//
//  /* finally, if depth of SE > 1 then perform max/min on each slice */
//  if (d > 1) {
//    /* set slice element offsets */
//    for (p[0]=0, i=1; i<nz; ++i)
//      p[i] = (nx*ny) + p[i-1];
//
//    /* gen max/min for each slice */
//    for (i=0; i<(nx*ny); ++i, ++slice)
//      (*func)(slice, g, h, p, nz, d);
//
//  } /* end if (d>1) */
//
//  /* clean up */
//  free(g);
//  free(h);
//  free(p);
//
//} /* end rect3dminmax_int4 */


#endif // RECT3DMM_HPP
