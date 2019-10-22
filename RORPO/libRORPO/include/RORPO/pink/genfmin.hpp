/*
 * File:		genfmin.c
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

/* Ed breen wrote a first version of this
 * converted into a stand-alone C file by Hugues Talbot	      13 Mar 1995
 * converted to pixtype-independent code by Annick Le Hegarat 24 Sep 2002
 */

#ifndef GENFMIN_HPP
#define GENFMIN_HPP

#include "liarp.h"

template <typename Type>
void genfmin(Type *f,
            Type *g,
            Type *h,
            long *p,
            unsigned int nx, unsigned int K)

{
  /* ORIGINAL AUTHOR: Ed Breen
  ** DATE: May  4 1993
  **
  ** f: input array. Where 1st member is *(f+p[0]) and
                           2nd member is = *(f+p[1]) etc.
  ** g: forward array
  ** h: backward array
  ** p: array of offsets
  ** nx: the number of elements in each array
  ** K:  the extent of mask
  */

    unsigned int i,j,k,r,r1;

    if (!(K%2))                     /* enforce the odd extent */
	K++;
    k = nx/K;
    r1 = nx%K;

    /* do forward array */
    for(j=0;j<k;j++) {
        *g = *(f+*p);
        for(g++,p++,i=1;i<K;i++,g++,p++)
            *g = liarmin(*(f+*p),*(g-1));
    }
    if(r1) {
        *g = *(f+*p);
        for(g++,p++,i=1;i<r1;i++,g++,p++)
            *g = liarmin(*(f+*p),*(g-1));
    }
    p--;
    if(nx <= (K>>1)) {
        g--;
        for(i=0;i<nx;i++,p--)
            *(f+*p) = *g;
        return;
    }
    h += nx - 1;
    g -= nx;

    /* do backward array */
    if(r1) {
        *h = *(f+*p);
        for(h--,p--,i=1;i<r1;i++,h--,p--)
            *h = liarmin(*(f+*p),*(h+1));
    }
    for(j=0;j<k;j++) {
        *h = *(f+*p);
        for(h--,p--,i=1;i<K;i++,h--,p--)
            *h = liarmin(*(f+*p),*(h+1));
    }

    /* reset pointers */
    p++;
    h++;
    r = K>>1;
    g+=r;
    if(nx <= K) {
        r1 = nx - r - 1;
        for(i=0;i<r1;i++,p++,g++)
            *(f+*p) = *g;
        r1 +=  K - nx + 1;
        for(;i<r1;i++,p++)
            *(f+*p) = *g;
        for(h++;i<nx;i++,h++,p++)
            *(f+*p) = *h;
        return;
    }

    /* do left border */
    for(i=0;i<r;i++,p++,g++)
        *(f+*p) = *g;

    /* do middle values */
    for(i=K-1;i<nx;i++,p++,h++,g++)
        *(f+*p) = liarmin(*g,*h);

    /* reset pointers to end position */
    h += (K-2);
    p += (r-1);

    /* do right border */
    if(r1 && k) {
        for(h-=r1,i=r1;i<K;i++,h--)
            *h = liarmin(*(h),*(h+1));
        h += K;
    }
    h -= r;
    for(i=0;i<r;i++,h--,p--)
        *(f+*p) = *(h);

}

#endif // GENFMIN_HPP
