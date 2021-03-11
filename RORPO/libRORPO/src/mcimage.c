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
   Librairie mcimage :

   fonctions pour les entrees/sorties fichiers et l'allocation de structures
   image en memoire.

   Michel Couprie 1996

   MC Update septembre 2000 : lecture de fichiers BMP truecolor
   MC Update octobre 2000 : ecriture de fichiers BMP truecolor
   MC Update janvier 2002 : lecture d'elements structurants (readse)
   MC Update mars 2002 : nettoie la gestion des noms
   MC Update decembre 2002 : type FLOAT
   MC Update decembre 2002 : convertgen
   MC Update avril 2003 : convertfloat, convertlong
   MC Update janvier 2006 : adoption des nouveaux "magic numbers" pour
                            les formats byte 3d, idem 2d (P2 et P5)
		            P7 (raw 3d) est conserv� pour la compatibilit�
   MC Update mars 2008 : fixe bug perte m�moire dans convertXXXX
   MC Update aout 2008 : nouveaux formats pgm �tendus PC, PD pour les "double"
   MC Update janvier 2011 : types COMPLEX DCOMPLEX
   MC Update mars 2011 : lecture/�criture des images 'short'
   HT Update avril 2011 : lecture des images TIFF
   MC Update avril 2011 : les images couleurs sont maintenant lues
                          par readimage comme 'multiband'
			  les images 2D � 3 bandes sont trait�es
			  par writeimage comme des images couleurs
   MC Update juillet 2011 : fonctions writelist2, writelist3
*/

#include <stdio.h>
#include <stdint.h>

#ifndef INT32_MAX
# define INT32_MAX		(2147483647)
#endif

// #include <unistd.h> in Microsoft Windows it does not exist, but we only need a subset of it
#ifdef UNIXIO
#  include <unistd.h>
#else /* NOT UNIXIO */
#  include <stdlib.h>
//#  include <io.h>
#endif /* NOT UNIXIO */

#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>


#ifdef HAVE_TIFF_LIB
#  include "liarp.h"
#  include "liarwrap.h"
#  include "savetiff.h"
#endif
/* HAVE_TIFF_LIB */

/* keep those private */
#include "RORPO/pink/mcutil.h"
#include "RORPO/pink/mcimage.h"
#include "RORPO/pink/mccodimage.h"

#define BUFFERSIZE 10000
/*
#define VERBOSE
*/

// define the following to print a warning when allocating huge image (>=2Gb)
#define WARN_HUGE

// Fopen strategy
__pink__inline FILE* pink_fopen_read( const char * filename )
{
# ifdef UNIXIO
  return fopen( filename, "r" );
# else /* NOT UNIXIO */
#   ifdef DOSIO
    return fopen( filename, "rb" );
#   else /* NOT DOSIO */
    //#     warning: FALLING BACK ON DEFAULT
      return fopen( filename, "rb" );
#   endif /* NOT DOSIO */
# endif /* NOT UNIXIO */
} /* pink_fopen_read */

__pink__inline FILE* pink_fopen_write( const char * filename )
{
# ifdef UNIXIO
  return fopen( filename, "w" );
# else /* NOT UNIXIO */
#   ifdef DOSIO
    return fopen( filename, "wb" );
#   else /* NOT DOSIO */
    //#     warning: FALLING BACK ON DEFAULT
      return fopen( filename, "wb" );
#   endif /* NOT DOSIO */
# endif /* NOT UNIXIO */
} /* pink_fopen_write */


/* ==================================== */
struct xvimage *allocimage(
  char * name,
  index_t rs,   /* row size */
  index_t cs,   /* col size */
  index_t ds,   /* depth */
  int32_t dt)   /* data type */
/* ==================================== */
#undef F_NAME
#define F_NAME "allocimage"
{
  index_t N = rs * cs * ds;             /* taille image */
  struct xvimage *g;
  index_t es;                          /* type size */

#ifdef WARN_HUGE
  if (((int64_t)rs * (int64_t)cs * (int64_t)ds) >= HUGE_IMAGE_SIZE)
  {
    fprintf(stderr, "%s: WARNING huge image\n", F_NAME);
  }
#endif

  switch (dt)
  {
    case VFF_TYP_BIT:      es = 1; break;
    case VFF_TYP_1_BYTE:   es = 1; break;
    case VFF_TYP_2_BYTE:   es = 2; break;
    case VFF_TYP_4_BYTE:   es = 4; break;
    case VFF_TYP_FLOAT:    es = sizeof(float); break;
    case VFF_TYP_DOUBLE:   es = sizeof(double); break;
    case VFF_TYP_COMPLEX:  es = 2*sizeof(float); break;
    case VFF_TYP_DCOMPLEX: es = 2*sizeof(double); break;
    default: fprintf(stderr,"%s: bad data type %d\n", F_NAME, dt);
             return NULL;
  } /* switch (t) */

  g = (struct xvimage *)malloc(sizeof(struct xvimage));
  if (g == NULL)
  {
    fprintf(stderr,"%s: malloc failed (%d bytes)\n", F_NAME, (int)sizeof(struct xvimage));
    return NULL;
  }

  g->image_data = (void *)calloc(1, N * es);
  if (g->image_data == NULL)
  {
#ifdef MC_64_BITS
    fprintf(stderr,"%s: calloc failed (%lld bytes)\n", F_NAME, (long long int)(N * es));
#else
    fprintf(stderr,"%s: calloc failed (%ld bytes)\n", F_NAME, (long int)(N * es));
#endif
    return NULL;
  }

  if (name != NULL)
  {
    g->name = (char *)calloc(1,strlen(name)+1);
    if (g->name == NULL)
    {   fprintf(stderr,"%s: malloc failed for name\n", F_NAME);
        return NULL;
    }
    strcpy((char *)(g->name), name);
  }
  else
    g->name = NULL;

  rowsize(g) = rs;
  colsize(g) = cs;
  depth(g) = ds;
  datatype(g) = dt;
  tsize(g) = 1;
  nbands(g) = 1;
  g->xdim = g->ydim = g->zdim = 0.0;
  g->xmin = g->ymin = g->zmin = 0;
  g->xmax = g->ymax = g->zmax = 0;

  return g;
} /* allocimage() */

/* ==================================== */
struct xvimage *allocmultimage(
  char * name,
  index_t rs,   /* row size */
  index_t cs,   /* col size */
  index_t ds,   /* depth */
  index_t ts,   /* time size */
  index_t nb,   /* nb of bands */
  int32_t dt)   /* data type */
/* ==================================== */
#undef F_NAME
#define F_NAME "allocmultimage"
{
  index_t N = rs * cs * ds * ts * nb; /* taille image */
  struct xvimage *g;
  index_t es;                    /* type size */

#ifdef WARN_HUGE
  if (((int64_t)rs * (int64_t)cs * (int64_t)ds * (int64_t)ts * (int64_t)nb) >= HUGE_IMAGE_SIZE)
  {
    fprintf(stderr, "%s: WARNING huge image\n", F_NAME);
  }
#endif

  switch (dt)
  {
    case VFF_TYP_BIT:      es = 1; break;
    case VFF_TYP_1_BYTE:   es = 1; break;
    case VFF_TYP_2_BYTE:   es = 2; break;
    case VFF_TYP_4_BYTE:   es = 4; break;
    case VFF_TYP_FLOAT:    es = sizeof(float); break;
    case VFF_TYP_DOUBLE:   es = sizeof(double); break;
    case VFF_TYP_COMPLEX:  es = 2*sizeof(float); break;
    case VFF_TYP_DCOMPLEX: es = 2*sizeof(double); break;
    default: fprintf(stderr,"%s: bad data type %d\n", F_NAME, dt);
             return NULL;
  } /* switch (t) */

  g = (struct xvimage *)malloc(sizeof(struct xvimage));
  if (g == NULL)
  {
    fprintf(stderr,"%s: malloc failed (%d bytes)\n", F_NAME, (int)sizeof(struct xvimage));
    return NULL;
  }

  g->image_data = (void *)calloc(1, N * es);
  if (g->image_data == NULL)
  {
#ifdef MC_64_BITS
    fprintf(stderr,"%s: calloc failed (%lld bytes)\n", F_NAME, (long long int)(N * es));
#else
    fprintf(stderr,"%s: calloc failed (%ld bytes)\n", F_NAME, (long int)(N * es));
#endif
    return NULL;
  }

  if (name != NULL)
  {
    g->name = (char *)calloc(1,strlen(name)+1);
    if (g->name == NULL)
    {   fprintf(stderr,"%s: malloc failed for name\n", F_NAME);
        return NULL;
    }
    strcpy((char *)(g->name), name);
  }
  else
    g->name = NULL;

  rowsize(g) = rs;
  colsize(g) = cs;
  depth(g) = ds;
  tsize(g) = ts;
  nbands(g) = nb;
  datatype(g) = dt;
  g->xdim = g->ydim = g->zdim = 0.0;
  g->xmin = g->ymin = g->zmin = 0;
  g->xmax = g->ymax = g->zmax = 0;

  return g;
} /* allocmultimage() */

/* ==================================== */
void razimage(struct xvimage *f)
/* ==================================== */
#undef F_NAME
#define F_NAME "razimage"
{
  index_t rs = rowsize(f);         /* taille ligne */
  index_t cs = colsize(f);         /* taille colonne */
  index_t ds = depth(f);           /* nb plans */
  index_t ts = tsize(f);           /* time size */
  index_t nb = nbands(f);          /* nb of bands */
  index_t N = rs * cs * ds * ts * nb;   /* taille image */
  index_t es;
  uint8_t *F = UCHARDATA(f);

  switch(datatype(f))
  {
    case VFF_TYP_BIT:      es = 1; break;
    case VFF_TYP_1_BYTE:   es = 1; break;
    case VFF_TYP_2_BYTE:   es = 2; break;
    case VFF_TYP_4_BYTE:   es = 4; break;
    case VFF_TYP_FLOAT:    es = sizeof(float); break;
    case VFF_TYP_DOUBLE:   es = sizeof(double); break;
    case VFF_TYP_COMPLEX:  es = 2*sizeof(float); break;
    case VFF_TYP_DCOMPLEX: es = 2*sizeof(double); break;
    default: fprintf(stderr,"%s: bad data type %d\n", F_NAME, datatype(f));
             return;
  } /* switch (t) */
  memset(F, 0, N * es);
} /* razimage() */

/* ==================================== */
struct xvimage *allocheader(
  char * name,
  index_t rs,   /* row size */
  index_t cs,   /* col size */
  index_t d,    /* depth */
  int32_t t)    /* data type */
/* ==================================== */
#undef F_NAME
#define F_NAME "allocheader"
{
  struct xvimage *g;

  g = (struct xvimage *)malloc(sizeof(struct xvimage));
  if (g == NULL)
  {   fprintf(stderr,"%s: malloc failed\n", F_NAME);
      return NULL;
  }
  if (name != NULL)
  {
    g->name = (char *)calloc(1,strlen(name)+1);
    if (g->name == NULL)
    {   fprintf(stderr,"%s: malloc failed for name\n", F_NAME);
        return NULL;
    }
    strcpy((char *)(g->name), name);
  }
  else
    g->name = NULL;

  g->image_data = NULL;
  rowsize(g) = rs;
  colsize(g) = cs;
  depth(g) = d;
  datatype(g) = t;
  tsize(g)=1;
  nbands(g)=1;

  g->xdim = g->ydim = g->zdim = 0.0;

  return g;
} /* allocheader() */

/* ==================================== */
int32_t showheader(char * name)
/* ==================================== */
#undef F_NAME
#define F_NAME "showheader"
{
  char buffer[BUFFERSIZE];
  FILE *fd = NULL;
  struct stat fdstat;
  index_t rs, cs, ds, nb, c, fs, es;
  char *read;

  fd = pink_fopen_read(name);

  if (!fd)
  {
    fprintf(stderr, "%s: file not found: %s\n", F_NAME, name);
    return 0;
  }

  read = fgets(buffer, BUFFERSIZE, fd);
  if (!read)
  {
    fprintf(stderr, "%s: fgets returned without reading\n", F_NAME);
    return 0;
  }
  if (buffer[0] != 'P')
  {
    fprintf(stderr,"%s: invalid image format: %c%c\n", F_NAME, buffer[0], buffer[1]);
    return 0;
  }
  switch (buffer[1])
  {
    case '2': printf("type: P%c (ascii byte)\n", buffer[1]); es = sizeof(int8_t); break;
    case '3': printf("type: P%c (ascii byte rgb)\n", buffer[1]); es = sizeof(int8_t); break;
    case '5': printf("type: P%c (raw byte)\n", buffer[1]); es = sizeof(int8_t); break;
    case '6': printf("type: P%c (raw byte rgb)\n", buffer[1]); es = sizeof(int8_t); break;
    case '7': printf("type: P%c (raw byte 3d - ext. MC [OBSOLETE - USE P5])\n", buffer[1]); es = sizeof(int8_t); break;
    case '8': printf("type: P%c (raw int32_t - ext. MC)\n", buffer[1]); es = sizeof(int32_t); break;
    case '9': printf("type: P%c (raw float - ext. MC)\n", buffer[1]); es = sizeof(float); break;
    case 'A': printf("type: P%c (ascii float - ext. LN)\n", buffer[1]); es = sizeof(float); break;
    case 'B': printf("type: P%c (ascii int32_t - ext. MC)\n", buffer[1]); es = sizeof(int32_t); break;
    case 'C': printf("type: P%c (raw double - ext. MC)\n", buffer[1]); es = sizeof(double); break;
    case 'D': printf("type: P%c (ascii double - ext. LN)\n", buffer[1]); es = sizeof(double); break;
    case 'E': printf("type: P%c (raw single precision complex - ext. MC)\n", buffer[1]); es = sizeof(fcomplex); break;
    case 'F': printf("type: P%c (ascii single precision complex - ext. MC)\n", buffer[1]); es = sizeof(fcomplex); break;
              break;
    default:
      fprintf(stderr,"%s: invalid image format: P%c\n", F_NAME, buffer[1]);
      return 0;
  } /* switch */

  do
  {
    read = fgets(buffer, BUFFERSIZE, fd);
    if (!read)
    {
      fprintf(stderr, "%s: fgets returned without reading\n", F_NAME);
      return 0;
    }
    if (buffer[0] == '#') /* commentaire */
      printf("comment: %s", buffer+1);
  } while (!isdigit(buffer[0]));

#ifdef MC_64_BITS
  c = sscanf(buffer, "%lld %lld %lld %lld", (long long int *)&rs, (long long int *)&cs, (long long int *)&ds, (long long int *)&nb);
#else
  c = sscanf(buffer, "%d %d %d %d", (int *)&rs, (int *)&cs, (int *)&ds, (int *)&nb);
#endif
  if (c == 2)
  {
#ifdef MC_64_BITS
    printf("size: rowsize = %lld ; colsize = %lld\n", (long long int)rs, (long long int)cs);
#else
    printf("size: rowsize = %d ; colsize = %d\n", rs, cs);
#endif
    ds = nb = 1;
  }
  else if (c == 3)
  {
#ifdef MC_64_BITS
    printf("size: rowsize = %lld ; colsize = %lld ; depth = %lld\n", (long long int)rs, (long long int)cs, (long long int)ds);
#else
    printf("size: rowsize = %d ; colsize = %d ; depth = %d\n", rs, cs, ds);
#endif
    nb = 1;
  }
  else if (c == 4)
#ifdef MC_64_BITS
    printf("size: rowsize = %lld ; colsize = %lld ; depth = %lld ; n. bands = %lld\n", (long long int)rs, (long long int)cs, (long long int)ds, (long long int)nb);
#else
    printf("size: rowsize = %d ; colsize = %d ; depth = %d ; n. bands = %d\n", rs, cs, ds, nb);
#endif
  else
  {
    fprintf(stderr,"%s: invalid image format: cannot find image size\n", F_NAME);
    return 0;
  }

  c = stat(name, &fdstat); assert(c == 0);
  fs = (index_t)fdstat.st_size;
#ifdef MC_64_BITS
  printf("header size = %lld\n", (long long int)(fs - (es * rs * cs * ds * nb)));
#else
  printf("header size = %d\n", fs - (es * rs * cs * ds * nb));
#endif

  fclose(fd);
  return 1;
} // showheader()

/* ==================================== */
void freeimage(struct xvimage *image)
/* ==================================== */
{
  if (image->name != NULL) free(image->name);
  free(image->image_data);
  free(image);
}

/* ==================================== */
struct xvimage *copyimage(struct xvimage *f)
/* ==================================== */
#undef F_NAME
#define F_NAME "copyimage"
{
  index_t rs = rowsize(f);         /* taille ligne */
  index_t cs = colsize(f);         /* taille colonne */
  index_t ds = depth(f);           /* nb plans */
  index_t ts = tsize(f);           /* time size */
  index_t nb = nbands(f);          /* nb of bands */
  index_t N = rs * cs * ds * ts * nb;        /* taille image */
  int32_t type = datatype(f);
  struct xvimage *g;

  g = allocmultimage(NULL, rs, cs, ds, ts, nb, type);
  if (g == NULL)
  {
    fprintf(stderr,"%s: allocimage failed\n", F_NAME);
    return NULL;
  }

  switch(type)
  {
  case VFF_TYP_1_BYTE: memcpy(g->image_data, f->image_data, (N*sizeof(int8_t))); break;
  case VFF_TYP_2_BYTE: memcpy(g->image_data, f->image_data,  (N*sizeof(int16_t))); break;
  case VFF_TYP_4_BYTE: memcpy(g->image_data, f->image_data, (N*sizeof(int32_t))); break;
  case VFF_TYP_FLOAT:  memcpy(g->image_data, f->image_data, (N*sizeof(float))); break;
  case VFF_TYP_DOUBLE: memcpy(g->image_data, f->image_data, (N*sizeof(double))); break;
  case VFF_TYP_COMPLEX:  memcpy(g->image_data, f->image_data, (2*N*sizeof(float))); break;
  case VFF_TYP_DCOMPLEX: memcpy(g->image_data, f->image_data, (2*N*sizeof(double))); break;
  default:
      fprintf(stderr,"%s: bad data type %d\n", F_NAME, type);
      return NULL;
  } /* switch(f->datatype) */

  if (f->name != NULL)
  {
    g->name = (char *)calloc(1,strlen(f->name) + 1);
    if (g->name == NULL)
    {   fprintf(stderr,"%s: malloc failed for name\n", F_NAME);
        return NULL;
    }
    strcpy(g->name, f->name);
  }
  return g;
} // copyimage()

/* ==================================== */
int32_t copy2image(struct xvimage *dest, struct xvimage *source)
/* ==================================== */
#undef F_NAME
#define F_NAME "copy2image"
{
  index_t rs = rowsize(source);         /* taille ligne */
  index_t cs = colsize(source);         /* taille colonne */
  index_t ds = depth(source);           /* nb plans */
  index_t ts = tsize(source);           /* time size */
  index_t nb = nbands(source);          /* nb of bands */
  index_t N = rs * cs * ds * ts * nb;   /* taille image */
  if ((rowsize(dest) != rs) || (colsize(dest) != cs) || (depth(dest) != ds))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  if (datatype(dest) != datatype(source))
  {
    fprintf(stderr, "%s: incompatible image types\n", F_NAME);
    return 0;
  }
  switch(datatype(source))
  {
    case VFF_TYP_1_BYTE:
      {
        uint8_t *S = UCHARDATA(source);
        uint8_t *D = UCHARDATA(dest);
        memcpy(D, S, N*sizeof(char));
        break;
      }
    case VFF_TYP_4_BYTE:
      {
        int32_t *S = SLONGDATA(source);
        int32_t *D = SLONGDATA(dest);
        memcpy(D, S, N*sizeof(int32_t));
        break;
      }
    case VFF_TYP_FLOAT:
      {
        float *S = FLOATDATA(source);
        float *D = FLOATDATA(dest);
        memcpy(D, S, N*sizeof(float));
        break;
      }
    case VFF_TYP_DOUBLE:
      {
        double *S = DOUBLEDATA(source);
        double *D = DOUBLEDATA(dest);
        memcpy(D, S, N*sizeof(double));
        break;
      }
    case VFF_TYP_COMPLEX:
      {
        float *S = FLOATDATA(source);
        float *D = FLOATDATA(dest);
        memcpy(D, S, 2*N*sizeof(float));
        break;
      }
    case VFF_TYP_DCOMPLEX:
      {
        double *S = DOUBLEDATA(source);
        double *D = DOUBLEDATA(dest);
        memcpy(D, S, 2*N*sizeof(double));
        break;
      }
    default:
      fprintf(stderr,"%s: bad data type %d\n", F_NAME, datatype(source));
      return 0;
  } /* switch(f->datatype) */
  return 1;
} // copy2image()

/* ==================================== */
int32_t equalimages(struct xvimage *im1, struct xvimage *im2)
/* ==================================== */
// checks whether the two images are equal
#undef F_NAME
#define F_NAME "equalimages"
{
  index_t rs = rowsize(im1);          /* taille ligne */
  index_t cs = colsize(im1);          /* taille colonne */
  index_t ds = depth(im1);            /* nb plans */
  index_t ts = tsize(im1);            /* time size */
  index_t nb = nbands(im1);           /* nb of bands */
  index_t N = rs * cs * ds * ts * nb; /* taille image */

  if ((rowsize(im2) != rs) || (colsize(im2) != cs) || (depth(im2) != ds) ||
      (tsize(im2) != ts) || (nbands(im2) != nb)) return 0;
  if (datatype(im2) != datatype(im1)) return 0;
  switch(datatype(im1))
  {
    case VFF_TYP_1_BYTE:
      {
        uint8_t *I1 = UCHARDATA(im1);
        uint8_t *I2 = UCHARDATA(im2);
        if (memcmp(I1, I2, N*sizeof(char)) != 0) return 0;
        break;
      }
    case VFF_TYP_4_BYTE:
      {
        int32_t *I1 = SLONGDATA(im1);
        int32_t *I2 = SLONGDATA(im2);
        if (memcmp(I1, I2, N*sizeof(int32_t)) != 0) return 0;
        break;
      }
    case VFF_TYP_FLOAT:
      {
        float *I1 = FLOATDATA(im1);
        float *I2 = FLOATDATA(im2);
        if (memcmp(I1, I2, N*sizeof(float)) != 0) return 0;
        break;
      }
    case VFF_TYP_DOUBLE:
      {
        double *I1 = DOUBLEDATA(im1);
        double *I2 = DOUBLEDATA(im2);
        if (memcmp(I1, I2, N*sizeof(double)) != 0) return 0;
        break;
      }
    case VFF_TYP_COMPLEX:
      {
        float *I1 = FLOATDATA(im1);
        float *I2 = FLOATDATA(im2);
        if (memcmp(I1, I2, 2*N*sizeof(float)) != 0) return 0;
        break;
      }
    case VFF_TYP_DCOMPLEX:
      {
        double *I1 = DOUBLEDATA(im1);
        double *I2 = DOUBLEDATA(im2);
        if (memcmp(I1, I2, 2*N*sizeof(double)) != 0) return 0;
        break;
      }
  } /* switch(f->datatype) */
  return 1;
} // equalimages()

/* ==================================== */
int32_t convertgen(struct xvimage **f1, struct xvimage **f2)
/* ==================================== */
// Converts the images f1, f2 to the type of the most general one.
// Returns the code of the chosen type.
#undef F_NAME
#define F_NAME "convertgen"
{
  struct xvimage *im1 = *f1;
  struct xvimage *im2 = *f2;
  struct xvimage *im3;
  int32_t type1 = datatype(im1);
  int32_t type2 = datatype(im2);
  int32_t type, typemax = mcmax(type1,type2);

  if (type1 == type2) return type1;
  if (type1 < type2) { im1 = *f2;  im2 = *f1; } // im1: rien a changer
  // il faut convertir 'im2' a 'typemax'
  type = datatype(im2);
  if (type == VFF_TYP_1_BYTE)
  {
    index_t i, rs=rowsize(im2), cs=colsize(im2), ds=depth(im2), N=rs*cs*ds;
    uint8_t *F = UCHARDATA(im2);
    im3 = allocimage(NULL, rs, cs, ds, typemax);
    if (im3 == NULL)
    {
      fprintf(stderr,"%s: allocimage failed\n", F_NAME);
      return 0;
    }
    if (typemax == VFF_TYP_4_BYTE)
    {
      int32_t *L = SLONGDATA(im3);
      for (i = 0; i < N; i++) L[i] = (int32_t)F[i];
    }
    else if (typemax == VFF_TYP_FLOAT)
    {
      float *FL = FLOATDATA(im3);
      for (i = 0; i < N; i++) FL[i] = (float)F[i];
    }
    else if (typemax == VFF_TYP_COMPLEX)
    {
      fcomplex *CL = COMPLEXDATA(im3);
      razimage(im3);
      for (i = 0; i < N; i++) CL[i].re = (float)F[i];
    }
    else
    {
      fprintf(stderr,"%s: bad data type\n", F_NAME);
      return 0;
    }
  }
  else if (type == VFF_TYP_4_BYTE)
  {
    index_t i, rs=rowsize(im2), cs=colsize(im2), ds=depth(im2), N=rs*cs*ds;
    int32_t *L = SLONGDATA(im2);
    im3 = allocimage(NULL, rs, cs, ds, typemax);
    if (im3 == NULL)
    {
      fprintf(stderr,"%s: allocimage failed\n", F_NAME);
      return 0;
    }
    if (typemax == VFF_TYP_FLOAT)
    {
      float *FL = FLOATDATA(im3);
      for (i = 0; i < N; i++) FL[i] = (float)L[i];
    }
    else
    if (typemax == VFF_TYP_COMPLEX)
    {
      fcomplex *CL = COMPLEXDATA(im3);
      razimage(im3);
      for (i = 0; i < N; i++) CL[i].re = (float)L[i];
    }
    else
    {
      fprintf(stderr,"%s: bad data type\n", F_NAME);
      return 0;
    }
  }
  else if (type == VFF_TYP_FLOAT)
  {
    index_t i, rs=rowsize(im2), cs=colsize(im2), ds=depth(im2), N=rs*cs*ds;
    float *FL = FLOATDATA(im2);
    im3 = allocimage(NULL, rs, cs, ds, typemax);
    if (im3 == NULL)
    {
      fprintf(stderr,"%s: allocimage failed\n", F_NAME);
      return 0;
    }
    if (typemax == VFF_TYP_COMPLEX)
    {
      fcomplex *CL = COMPLEXDATA(im3);
      razimage(im3);
      for (i = 0; i < N; i++) CL[i].re = (float)FL[i];
    }
    else
    {
      fprintf(stderr,"%s: bad data type\n", F_NAME);
      return 0;
    }
  }
  else
  {
    fprintf(stderr,"%s: bad data type\n", F_NAME);
    return 0;
  }

  if (type1 < type2)
  {
    freeimage(*f1);
    *f1 = im3;
  }
  else
  {
    freeimage(*f2);
    *f2 = im3;
  }
  return typemax;
} // convertgen()

/* ==================================== */
int32_t convertlong(struct xvimage **f1)
/* ==================================== */
// Converts the image f1 to int32_t.
#undef F_NAME
#define F_NAME "convertlong"
{
  struct xvimage *im1 = *f1;
  struct xvimage *im3;
  int32_t type = datatype(im1);
  index_t i, rs=rowsize(im1), cs=colsize(im1), ds=depth(im1);
  index_t N=rs*cs*ds;
  int32_t *FL;

  if (type == VFF_TYP_4_BYTE) return 1;

  im3 = allocimage(NULL, rs, cs, ds, VFF_TYP_4_BYTE);
  if (im3 == NULL)
  {
    fprintf(stderr,"%s: allocimage failed\n", F_NAME);
    return 0;
  }
  FL = SLONGDATA(im3);

  if (type == VFF_TYP_1_BYTE)
  {
    uint8_t *F = UCHARDATA(im1);
    for (i = 0; i < N; i++) FL[i] = (int32_t)F[i];
  }
  else if (type == VFF_TYP_FLOAT)
  {
    float *F = FLOATDATA(im1);
    for (i = 0; i < N; i++) FL[i] = (int32_t)F[i];
  }
  else
  {
    fprintf(stderr,"%s: bad data type\n", F_NAME);
    return 0;
  }

  freeimage(*f1);
  *f1 = im3;
  return 1;
} // convertlong()

/* ==================================== */
int32_t convertfloat(struct xvimage **f1)
/* ==================================== */
// Converts the image f1 to float.
#undef F_NAME
#define F_NAME "convertfloat"
{
  struct xvimage *im1 = *f1;
  struct xvimage *im3;
  int32_t type = datatype(im1);
  index_t i, rs=rowsize(im1), cs=colsize(im1), ds=depth(im1);
  index_t N=rs*cs*ds;
  float *FL;

  if (type == VFF_TYP_FLOAT) return 1;

  im3 = allocimage(NULL, rs, cs, ds, VFF_TYP_FLOAT);
  if (im3 == NULL)
  {
    fprintf(stderr,"%s: allocimage failed\n", F_NAME);
    return 0;
  }
  FL = FLOATDATA(im3);

  if (type == VFF_TYP_1_BYTE)
  {
    uint8_t *F = UCHARDATA(im1);
    for (i = 0; i < N; i++) FL[i] = (float)F[i];
  }
  else if (type == VFF_TYP_4_BYTE)
  {
    int32_t *F = SLONGDATA(im1);
    for (i = 0; i < N; i++) FL[i] = (float)F[i];
  }
  else
  {
    fprintf(stderr,"%s: bad data type\n", F_NAME);
    return 0;
  }

  freeimage(*f1);
  *f1 = im3;
  return 1;
} // convertfloat()

/* ==================================== */
void list2image(struct xvimage * image, double *P, index_t n)
/* ==================================== */
#undef F_NAME
#define F_NAME "list2image"
{
  index_t rs, cs, ds, ps, x, y, z, i;
  index_t N;
  uint8_t *F;

  rs = rowsize(image);
  cs = colsize(image);
  ds = depth(image);
  ps = rs * cs;
  N = ps * ds;
  F = UCHARDATA(image);
  if (ds == 1) // 2D
  {
    for (i = 0; i < n; i++)
    {
      x = arrondi(P[2*i]); y = arrondi(P[2*i + 1]);
      if ((x >= 0) && (x < rs) && (y >= 0) && (y < cs))
	F[y * rs + x] = NDG_MAX;
    }
  }
  else // 3D
  {
    for (i = 0; i < n; i++)
    {
      x = arrondi(P[3*i]);
      y = arrondi(P[3*i + 1]);
      z = arrondi(P[3*i + 2]);
      if ((x >= 0) && (x < rs) && (y >= 0) && (y < cs) && (z >= 0) && (z < ds))
	F[z*ps + y*rs + x] = NDG_MAX;
    }
  }
} // list2image()

/* ==================================== */
double * image2list(struct xvimage * image, index_t *n)
/* ==================================== */
#undef F_NAME
#define F_NAME "image2list"
{
  index_t rs, cs, ds, ps, x, y, z;
  index_t N;
  uint8_t *F;
  index_t n1;
  double * P1;

  rs = rowsize(image);
  cs = colsize(image);
  ds = depth(image);
  ps = rs * cs;
  N = ps * ds;
  F = UCHARDATA(image);
  n1 = 0;                     /* compte le nombre de points non nuls pour image1 */
  for (x = 0; x < N; x++) if (F[x]) n1++;
  if (ds == 1) // 2D
  {
    P1 = (double *)calloc(1, n1 * 2 * sizeof(double));
    if (P1 == NULL)
    {
      fprintf(stderr,"%s: malloc failed for P1\n", F_NAME);
      return NULL;
    }
    n1 = 0;
    for (y = 0; y < cs; y++)
      for (x = 0; x < rs; x++)
	if (F[y * rs + x]) { P1[2*n1] = (double)x; P1[2*n1 + 1] = (double)y; n1++; }
  }
  else // 3D
  {
    P1 = (double *)calloc(1, n1 * 3 * sizeof(double));
    if (P1 == NULL)
    {
      fprintf(stderr,"%s: malloc failed for P1\n", F_NAME);
      return NULL;
    }
    n1 = 0;
    for (z = 0; z < ds; z++)
      for (y = 0; y < cs; y++)
	for (x = 0; x < rs; x++)
	  if (F[z*ps + y*rs + x])
	  {
	    P1[3*n1] = (double)x;
	    P1[3*n1 + 1] = (double)y;
	    P1[3*n1 + 2] = (double)z;
	    n1++;
	  }
  }
  *n = n1;
  return P1;
} // image2list()

/* ==================================== */
void writeimage(struct xvimage * image, const char *filename)
/* ==================================== */
#undef F_NAME
#define F_NAME "writeimage"
{
  index_t rs, cs, ds, np;
  rs = rowsize(image);
  cs = colsize(image);
  ds = depth(image);
  np = nbands(image);

  if ((rs<=25) && (cs<=25) && (ds<=25) &&// (np==1) &&
      ((datatype(image) == VFF_TYP_1_BYTE) || (datatype(image) == VFF_TYP_4_BYTE) ||
       (datatype(image) == VFF_TYP_FLOAT) || (datatype(image) == VFF_TYP_DOUBLE) ||
       (datatype(image) == VFF_TYP_COMPLEX) || (datatype(image) == VFF_TYP_DCOMPLEX)))
  {
    fprintf(stderr,"%s: writing image in ASCII mode\n", F_NAME);
    writeascimage(image, filename);
  }
  else
    writerawimage(image, filename);
} /* writeimage() */

/* ==================================== */
void writerawimage(struct xvimage * image, const char *filename)
/* ==================================== */
#undef F_NAME
#define F_NAME "writerawimage"
{
  FILE *fd = NULL;
  index_t i, rs, cs, ds, nb;
  index_t N, ret;
  int32_t color = 0;

  rs = rowsize(image);
  cs = colsize(image);
  ds = depth(image);
  nb = nbands(image);
  N = rs * cs * ds * nb;

  fd = pink_fopen_write(filename);

  if (!fd)
  {
    fprintf(stderr, "%s: cannot open file: %s\n", F_NAME, filename);
    exit(0);
  }

  if (datatype(image) == VFF_TYP_1_BYTE)
  {
    if ((ds == 1) && (nb == 3)) color = 1;
    if (color)
      fputs("P6\n", fd);
    else
      fputs("P5\n", fd);
    if ((image->xdim != 0.0) && (ds > 1))
      fprintf(fd, "#xdim %g\n#ydim %g\n#zdim %g\n", image->xdim, image->ydim, image->zdim);
    if ((image->xdim != 0.0) && (ds == 1))
      fprintf(fd, "#xdim %g\n#ydim %g\n", image->xdim, image->ydim);

    if ((nb > 1) && !color)
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)ds, (long long int)nb);
#else
      fprintf(fd, "%d %d %d %d\n", rs, cs, ds, nb);
#endif
    else if (ds > 1)
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)ds);
#else
      fprintf(fd, "%d %d %d\n", rs, cs, ds);
#endif
    else
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld\n", (long long int)rs, (long long int)cs);
#else
      fprintf(fd, "%d %d\n", rs, cs);
#endif
    fprintf(fd, "255\n");

    if (color)
    {
      uint8_t *b1, *b2, *b3;
      N = rs * cs;
      b1 = UCHARDATA(image);
      b2 = b1 + N;
      b3 = b2 + N;
      for (i = 0; i < N; i++)
      {
	fputc((int32_t)(b1[i]), fd);
	fputc((int32_t)(b2[i]), fd);
	fputc((int32_t)(b3[i]), fd);
      }
    }
    else
    {
      ret = fwrite(UCHARDATA(image), sizeof(char), N, fd);
      if (ret != N)
      {
#ifdef MC_64_BITS
	fprintf(stderr, "%s: only %lld items written\n", F_NAME, (long long int)ret);
#else
	fprintf(stderr, "%s: only %d items written\n", F_NAME, ret);
#endif
	exit(0);
      }
    }
  }
  else if (datatype(image) == VFF_TYP_2_BYTE)
  {
    uint16_t tmp;
    uint8_t tmp1;
    index_t i;

    fputs("P5\n", fd);
    if ((image->xdim != 0.0) && (ds > 1))
      fprintf(fd, "#xdim %g\n#ydim %g\n#zdim %g\n", image->xdim, image->ydim, image->zdim);
    if ((image->xdim != 0.0) && (ds == 1))
      fprintf(fd, "#xdim %g\n#ydim %g\n", image->xdim, image->ydim);
    if (nb > 1)
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)ds, (long long int)nb);
#else
      fprintf(fd, "%d %d %d %d\n", rs, cs, ds, nb);
#endif
    else if (ds > 1)
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)ds);
#else
      fprintf(fd, "%d %d %d\n", rs, cs, ds);
#endif
    else
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld\n", (long long int)rs, (long long int)cs);
#else
      fprintf(fd, "%d %d\n", rs, cs);
#endif
    fprintf(fd, "65535\n");

    // Standard PGM format imposes big-endian for 2-byte images, but
    // standard PC architectures are little-endian
    for (i = 0; i < N; i++)
    {
      tmp = USHORTDATA(image)[i];
      // conversion little-endian -> big-endian
      tmp1 = tmp & 0x00ff;
      tmp = tmp >> 8;
      tmp = tmp | (((uint16_t)tmp1) << 8);
      (void)fwrite(&tmp, sizeof(uint16_t), 1, fd);
    }
  }
  else if (datatype(image) == VFF_TYP_4_BYTE)
  {
    fputs("P8\n", fd);
    if ((image->xdim != 0.0) && (ds > 1))
      fprintf(fd, "#xdim %g\n#ydim %g\n#zdim %g\n", image->xdim, image->ydim, image->zdim);
    if ((image->xdim != 0.0) && (ds == 1))
      fprintf(fd, "#xdim %g\n#ydim %g\n", image->xdim, image->ydim);
    if (nb > 1)
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)ds, (long long int)nb);
#else
      fprintf(fd, "%d %d %d %d\n", rs, cs, ds, nb);
#endif
    else if (ds > 1)
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)ds);
#else
      fprintf(fd, "%d %d %d\n", rs, cs, ds);
#endif
    else
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld\n", (long long int)rs, (long long int)cs);
#else
      fprintf(fd, "%d %d\n", rs, cs);
#endif
    fprintf(fd, "4294967295\n");

    ret = fwrite(SLONGDATA(image), sizeof(int32_t), N, fd);
    if (ret != N)
    {
#ifdef MC_64_BITS
      fprintf(stderr, "%s: only %lld items written\n", F_NAME, (long long int)ret);
#else
      fprintf(stderr, "%s: only %d items written\n", F_NAME, ret);
#endif
      exit(0);
    }
  }
  else if (datatype(image) == VFF_TYP_FLOAT)
  {
    fputs("P9\n", fd);
    if ((image->xdim != 0.0) && (ds > 1))
      fprintf(fd, "#xdim %g\n#ydim %g\n#zdim %g\n", image->xdim, image->ydim, image->zdim);
    if ((image->xdim != 0.0) && (ds == 1))
      fprintf(fd, "#xdim %g\n#ydim %g\n", image->xdim, image->ydim);
    if (nb > 1)
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)ds, (long long int)nb);
#else
      fprintf(fd, "%d %d %d %d\n", rs, cs, ds, nb);
#endif
    else if (ds > 1)
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)ds);
#else
      fprintf(fd, "%d %d %d\n", rs, cs, ds);
#endif
    else
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld\n", (long long int)rs, (long long int)cs);
#else
      fprintf(fd, "%d %d\n", rs, cs);
#endif
    fprintf(fd, "0\n");

    ret = fwrite(FLOATDATA(image), sizeof(float), N, fd);
    if (ret != N)
    {
#ifdef MC_64_BITS
      fprintf(stderr, "%s: only %lld items written\n", F_NAME, (long long int)ret);
#else
      fprintf(stderr, "%s: only %d items written\n", F_NAME, ret);
#endif
      exit(0);
    }
  }
  else if (datatype(image) == VFF_TYP_DOUBLE)
  {
    fputs("PC\n", fd);
    if ((image->xdim != 0.0) && (ds > 1))
      fprintf(fd, "#xdim %g\n#ydim %g\n#zdim %g\n", image->xdim, image->ydim, image->zdim);
    if ((image->xdim != 0.0) && (ds == 1))
      fprintf(fd, "#xdim %g\n#ydim %g\n", image->xdim, image->ydim);
    if (nb > 1)
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)ds, (long long int)nb);
#else
      fprintf(fd, "%d %d %d %d\n", rs, cs, ds, nb);
#endif
    else if (ds > 1)
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)ds);
#else
      fprintf(fd, "%d %d %d\n", rs, cs, ds);
#endif
    else
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld\n", (long long int)rs, (long long int)cs);
#else
      fprintf(fd, "%d %d\n", rs, cs);
#endif
    fprintf(fd, "0\n");

    ret = fwrite(DOUBLEDATA(image), sizeof(double), N, fd);
    if (ret != N)
    {
#ifdef MC_64_BITS
      fprintf(stderr, "%s: only %lld items written\n", F_NAME, (long long int)ret);
#else
      fprintf(stderr, "%s: only %d items written\n", F_NAME, ret);
#endif
      exit(0);
    }
  }
  else if (datatype(image) == VFF_TYP_COMPLEX)
  {
    fputs("PE\n", fd);
    if ((image->xdim != 0.0) && (ds > 1))
      fprintf(fd, "#xdim %g\n#ydim %g\n#zdim %g\n", image->xdim, image->ydim, image->zdim);
    if ((image->xdim != 0.0) && (ds == 1))
      fprintf(fd, "#xdim %g\n#ydim %g\n", image->xdim, image->ydim);
    if (nb > 1)
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)ds, (long long int)nb);
#else
      fprintf(fd, "%d %d %d %d\n", rs, cs, ds, nb);
#endif
    else if (ds > 1)
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)ds);
#else
      fprintf(fd, "%d %d %d\n", rs, cs, ds);
#endif
    else
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld\n", (long long int)rs, (long long int)cs);
#else
      fprintf(fd, "%d %d\n", rs, cs);
#endif
    fprintf(fd, "0\n");

    ret = fwrite(FLOATDATA(image), sizeof(float), N+N, fd);
    if (ret != N+N)
    {
#ifdef MC_64_BITS
      fprintf(stderr, "%s: only %lld items written\n", F_NAME, (long long int)ret);
#else
      fprintf(stderr, "%s: only %d items written\n", F_NAME, ret);
#endif
      exit(0);
    }
  }
  else
  {   fprintf(stderr,"%s: bad datatype: %d\n", F_NAME, datatype(image));
      exit(0);
  }

  fclose(fd);
} /* writerawimage() */

/* ==================================== */
void writese(struct xvimage * image, const char *filename, index_t x, index_t y, index_t z)
/* ==================================== */
#undef F_NAME
#define F_NAME "writese"
{
  FILE *fd = NULL;
  index_t rs, cs, d, ps, i;
  index_t N, ret;

  rs = rowsize(image);
  cs = colsize(image);
  d = depth(image);
  ps = rs * cs;
  N = ps * d;

  fd = pink_fopen_write(filename);

  if (!fd)
  {
    fprintf(stderr, "%s: cannot open file: %s\n", F_NAME, filename);
    exit(0);
  }

  if (datatype(image) == VFF_TYP_1_BYTE)
  {
    if (d > 1)
    {
      if ((rs<=25) && (cs<=25) && (d<=25)) fputs("P2\n", fd); else fputs("P5\n", fd);
#ifdef MC_64_BITS
      fprintf(fd, "#origin %lld %lld %lld\n", (long long int)x, (long long int)y, (long long int)z);
#else
      fprintf(fd, "#origin %d %d %d\n", x, y, z);
#endif
    }
    else
    {
      if ((rs<=25) && (cs<=25) && (d<=25)) fputs("P2\n", fd); else fputs("P5\n", fd);
#ifdef MC_64_BITS
      fprintf(fd, "#origin %lld %lld\n", (long long int)x, (long long int)y);
#else
      fprintf(fd, "#origin %d %d\n", x, y);
#endif
    }
    /*    fputs("# CREATOR: writese by MC - 07/1996\n", fd); */
    if (d > 1)
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)d);
#else
      fprintf(fd, "%d %d %d\n", rs, cs, d);
#endif
    else
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld\n", (long long int)rs, (long long int)cs);
#else
      fprintf(fd, "%d %d\n", rs, cs);
#endif
    fprintf(fd, "255\n");

    if ((rs<=25) && (cs<=25) && (d<=25))
    {
      for (i = 0; i < N; i++)
      {
	if (i % rs == 0) fprintf(fd, "\n");
	if (i % ps == 0) fprintf(fd, "\n");
	fprintf(fd, "%3d ", (int32_t)(UCHARDATA(image)[i]));
      } /* for i */
      fprintf(fd, "\n");
    }
    else
    {
      ret = fwrite(UCHARDATA(image), sizeof(char), N, fd);
      if (ret != N)
      {
#ifdef MC_64_BITS
	fprintf(stderr, "%s: only %lld items written\n", F_NAME, (long long int)ret);
#else
	fprintf(stderr, "%s: only %d items written\n", F_NAME, ret);
#endif
	exit(0);
      }
    }
  }
  else
  {   fprintf(stderr,"%s: bad datatype: %d\n", F_NAME, datatype(image));
      exit(0);
  }

  fclose(fd);
} /* writese() */

/* ==================================== */
void writeascimage(struct xvimage * image, const char *filename)
/* ==================================== */
#undef F_NAME
#define F_NAME "writeascimage"
{
  FILE *fd = NULL;
  index_t rs, cs, ps, ds, nb, i;
  index_t N;
  int32_t color = 0;

  fd = pink_fopen_write(filename);
  if (!fd)
  {
    fprintf(stderr, "%s: cannot open file: %s\n", F_NAME, filename);
    exit(0);
  }

  rs = rowsize(image);
  cs = colsize(image);
  ds = depth(image);
  nb = nbands(image);
  ps = rs * cs;
  N = ps * ds * nb;

  if (datatype(image) == VFF_TYP_1_BYTE)
  {
    if ((ds == 1) && (nb == 3)) color = 1;
    if (color)
      fputs("P3\n", fd);
    else
      fputs("P2\n", fd);
    if ((image->xdim != 0.0) && (ds > 1))
      fprintf(fd, "#xdim %g\n#ydim %g\n#zdim %g\n", image->xdim, image->ydim, image->zdim);
    if ((nb > 1) && !color)
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)ds, (long long int)nb);
#else
      fprintf(fd, "%d %d %d %d\n", rs, cs, ds, nb);
#endif
    else if (ds > 1)
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)ds);
#else
      fprintf(fd, "%d %d %d\n", rs, cs, ds);
#endif
    else
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld\n", (long long int)rs, (long long int)cs);
#else
      fprintf(fd, "%d %d\n", rs, cs);
#endif
    fprintf(fd, "255\n");

    if (color)
    {
      uint8_t *b1, *b2, *b3;
      N = rs * cs;
      b1 = UCHARDATA(image);
      b2 = b1 + N;
      b3 = b2 + N;
      for (i = 0; i < N; i++)
      {
	if (i % rs == 0) fprintf(fd, "\n");
	fprintf(fd, "%d ", (int32_t)(b1[i]));
	fprintf(fd, "%d ", (int32_t)(b2[i]));
	fprintf(fd, "%d ", (int32_t)(b3[i]));
      } /* for i */
    }
    else
    {
      if (N > 8000) // grandes images : pas de padding (blancs)
      {
	for (i = 0; i < N; i++)
	{
	  if (i % rs == 0) fprintf(fd, "\n");
	  if (i % ps == 0) fprintf(fd, "\n");
	  fprintf(fd, "%d ", (int32_t)(UCHARDATA(image)[i]));
	} /* for i */
      }
      else
      {
	for (i = 0; i < N; i++)
	{
	  if (i % rs == 0) fprintf(fd, "\n");
	  if (i % ps == 0) fprintf(fd, "\n");
	  fprintf(fd, "%3d ", (int32_t)(UCHARDATA(image)[i]));
	} /* for i */
      }
      fprintf(fd, "\n");
    }
  }
  else if (datatype(image) == VFF_TYP_2_BYTE)
  {
    fputs("P2\n", fd);
    if ((image->xdim != 0.0) && (ds > 1))
      fprintf(fd, "#xdim %g\n#ydim %g\n#zdim %g\n", image->xdim, image->ydim, image->zdim);
    if (nb > 1)
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)ds, (long long int)nb);
#else
      fprintf(fd, "%d %d %d %d\n", rs, cs, ds, nb);
#endif
    else if (ds > 1)
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)ds);
#else
      fprintf(fd, "%d %d %d\n", rs, cs, ds);
#endif
    else
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld\n", (long long int)rs, (long long int)cs);
#else
      fprintf(fd, "%d %d\n", rs, cs);
#endif
    fprintf(fd, "65535\n");

    for (i = 0; i < N; i++)
    {
      if (i % rs == 0) fprintf(fd, "\n");
      if (i % ps == 0) fprintf(fd, "\n");
      fprintf(fd, "%d ", (int16_t)(USHORTDATA(image)[i]));
    } /* for i */
    fprintf(fd, "\n");
  }
  else if (datatype(image) == VFF_TYP_4_BYTE)
  {
    fputs("PB\n", fd);
    if ((image->xdim != 0.0) && (ds > 1))
      fprintf(fd, "#xdim %g\n#ydim %g\n#zdim %g\n", image->xdim, image->ydim, image->zdim);
    if (nb > 1)
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)ds, (long long int)nb);
#else
      fprintf(fd, "%d %d %d %d\n", rs, cs, ds, nb);
#endif
    else if (ds > 1)
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)ds);
#else
      fprintf(fd, "%d %d %d\n", rs, cs, ds);
#endif
    else
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld\n", (long long int)rs, (long long int)cs);
#else
      fprintf(fd, "%d %d\n", rs, cs);
#endif
    fprintf(fd, "4294967295\n");

    for (i = 0; i < N; i++)
    {
      if (i % rs == 0) fprintf(fd, "\n");
      if (i % ps == 0) fprintf(fd, "\n");
      fprintf(fd, "%ld ", (long int)(SLONGDATA(image)[i]));
    } /* for i */
    fprintf(fd, "\n");
  }
  else if (datatype(image) == VFF_TYP_FLOAT)
  {
    fputs("PA\n", fd);
    if ((image->xdim != 0.0) && (ds > 1))
      fprintf(fd, "#xdim %g\n#ydim %g\n#zdim %g\n", image->xdim, image->ydim, image->zdim);
    if (nb > 1)
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)ds, (long long int)nb);
#else
      fprintf(fd, "%d %d %d %d\n", rs, cs, ds, nb);
#endif
    else if (ds > 1)
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)ds);
#else
      fprintf(fd, "%d %d %d\n", rs, cs, ds);
#endif
    else
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld\n", (long long int)rs, (long long int)cs);
#else
      fprintf(fd, "%d %d\n", rs, cs);
#endif
    fprintf(fd, "1\n");

    for (i = 0; i < N; i++)
    {
      if (i % rs == 0) fprintf(fd, "\n");
      if (i % ps == 0) fprintf(fd, "\n");
      fprintf(fd, "%8g ", FLOATDATA(image)[i]);
    } /* for i */
    fprintf(fd, "\n");
  }
  else if (datatype(image) == VFF_TYP_DOUBLE)
  {
    fputs("PD\n", fd);
    if ((image->xdim != 0.0) && (ds > 1))
      fprintf(fd, "#xdim %g\n#ydim %g\n#zdim %g\n", image->xdim, image->ydim, image->zdim);
    if (nb > 1)
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)ds, (long long int)nb);
#else
      fprintf(fd, "%d %d %d %d\n", rs, cs, ds, nb);
#endif
    else if (ds > 1)
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)ds);
#else
      fprintf(fd, "%d %d %d\n", rs, cs, ds);
#endif
    else
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld\n", (long long int)rs, (long long int)cs);
#else
      fprintf(fd, "%d %d\n", rs, cs);
#endif
    fprintf(fd, "1\n");

    for (i = 0; i < N; i++)
    {
      if (i % rs == 0) fprintf(fd, "\n");
      if (i % ps == 0) fprintf(fd, "\n");
      fprintf(fd, "%8g ", DOUBLEDATA(image)[i]);
    } /* for i */
    fprintf(fd, "\n");
  }
  else if (datatype(image) == VFF_TYP_COMPLEX)
  {
    fputs("PF\n", fd);
    if ((image->xdim != 0.0) && (ds > 1))
      fprintf(fd, "#xdim %g\n#ydim %g\n#zdim %g\n", image->xdim, image->ydim, image->zdim);
    if (nb > 1)
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)ds, (long long int)nb);
#else
      fprintf(fd, "%d %d %d %d\n", rs, cs, ds, nb);
#endif
    else if (ds > 1)
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)ds);
#else
      fprintf(fd, "%d %d %d\n", rs, cs, ds);
#endif
    else
#ifdef MC_64_BITS
      fprintf(fd, "%lld %lld\n", (long long int)rs, (long long int)cs);
#else
      fprintf(fd, "%d %d\n", rs, cs);
#endif
    fprintf(fd, "1\n");

    for (i = 0; i < N; i++)
    {
      if (i % rs == 0) fprintf(fd, "\n");
      if (i % ps == 0) fprintf(fd, "\n");
      fprintf(fd, "%8g %8g  ", FLOATDATA(image)[i+i], FLOATDATA(image)[i+i+1]);
    } /* for i */
    fprintf(fd, "\n");
  }
  else
  {   fprintf(stderr,"%s: bad datatype: %d\n", F_NAME, datatype(image));
      exit(0);
  }
  fclose(fd);
}

/* ==================================== */
void printimage(struct xvimage * image)
/* ==================================== */
#undef F_NAME
#define F_NAME "printimage"
{
  index_t rs, cs, d, ps, i;
  index_t N;

  rs = rowsize(image);
  cs = colsize(image);
  d = depth(image);
  ps = rs * cs;
  N = ps * d;

  if (datatype(image) == VFF_TYP_1_BYTE)
  {
    for (i = 0; i < N; i++)
    {
      if (i % rs == 0) printf("\n");
      if (i % ps == 0) printf("\n");
      printf("%3d ", (int32_t)(UCHARDATA(image)[i]));
    } /* for i */
    printf("\n");
  }
  else
  {   fprintf(stderr,"%s: bad datatype: %d\n", F_NAME, datatype(image));
      exit(0);
  }
}

/* ==================================== */
void writergbimage(
  struct xvimage * redimage,
  struct xvimage * greenimage,
  struct xvimage * blueimage,
  const char *filename)
/* ==================================== */
#undef F_NAME
#define F_NAME "writergbimage"
{
  FILE *fd = NULL;
  index_t rs, cs, i;
  index_t N;
  int32_t nndg;

  fd = pink_fopen_write(filename);

  if (!fd)
  {
    fprintf(stderr, "%s: cannot open file: %s\n", F_NAME, filename);
    exit(0);
  }

  rs = redimage->row_size;
  cs = redimage->col_size;
  if ((greenimage->row_size != rs) || (greenimage->col_size != cs) ||
      (blueimage->row_size != rs) || (blueimage->col_size != cs))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    exit(0);
  }

  N = rs * cs;
  nndg = 255;

  fputs("P6\n", fd);
  /*  fputs("# CREATOR: writeimage by MC - 07/1996\n", fd); */
  fprintf(fd, "##rgb\n");
#ifdef MC_64_BITS
  fprintf(fd, "%lld %lld\n", (long long int)rs, (long long int)cs);
#else
  fprintf(fd, "%d %d\n", rs, cs);
#endif
  fprintf(fd, "%d\n", nndg);

  for (i = 0; i < N; i++)
  {
    fputc((int32_t)(UCHARDATA(redimage)[i]), fd);
    fputc((int32_t)(UCHARDATA(greenimage)[i]), fd);
    fputc((int32_t)(UCHARDATA(blueimage)[i]), fd);
  } /* for i */

  fclose(fd);
} // writergbimage()

/* ==================================== */
void writergbascimage(
  struct xvimage * redimage,
  struct xvimage * greenimage,
  struct xvimage * blueimage,
  const char *filename)
/* ==================================== */
#undef F_NAME
#define F_NAME "writergbascimage"
{
  FILE *fd = NULL;
  index_t rs, cs, i, j;
  index_t N;
  int32_t nndg;

  fd = pink_fopen_write(filename);

  if (!fd)
  {
    fprintf(stderr, "%s: cannot open file: %s\n", F_NAME, filename);
    exit(0);
  }

  rs = redimage->row_size;
  cs = redimage->col_size;
  if ((greenimage->row_size != rs) || (greenimage->col_size != cs) ||
      (blueimage->row_size != rs) || (blueimage->col_size != cs))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    exit(0);
  }

  N = rs * cs;
  nndg = 255;

  fputs("P3\n", fd);
  fprintf(fd, "##rgb\n");
#ifdef MC_64_BITS
  fprintf(fd, "%lld %lld\n", (long long int)rs, (long long int)cs);
#else
  fprintf(fd, "%d %d\n", rs, cs);
#endif
  fprintf(fd, "%d\n", nndg);

  for (j = 0; j < cs; j++)
  {
    for (i = 0; i < rs; i++)
    {
      fprintf(fd, " %d", (int32_t)(UCHARDATA(redimage)[i]));
      fprintf(fd, " %d", (int32_t)(UCHARDATA(greenimage)[i]));
      fprintf(fd, " %d", (int32_t)(UCHARDATA(blueimage)[i]));
    } /* for i */
    fprintf(fd, "\n");
  } /* for j */

  fclose(fd);
} // writergbascimage()

/* ==================================== */
void writelongimage(struct xvimage * image,  const char *filename)
/* ==================================== */
#undef F_NAME
#define F_NAME "writelongimage"
/*
   obsolete - utiliser maintenant writeimage
*/
{
  FILE *fd = NULL;
  index_t rs, cs, d;
  index_t N, ret;
  int32_t nndg;

  fd = pink_fopen_write(filename);

  if (!fd)
  {
    fprintf(stderr, "%s: cannot open file: %s\n", F_NAME, filename);
    exit(0);
  }

  rs = rowsize(image);
  cs = colsize(image);
  d = depth(image);
  N = rs * cs * d;
  nndg = 255;

  fputs("P8\n", fd);
  /*  fputs("# CREATOR: writelongimage by MC - 07/1996\n", fd); */
  if (d > 1)
#ifdef MC_64_BITS
    fprintf(fd, "%lld %lld %lld\n", (long long int)rs, (long long int)cs, (long long int)d);
#else
    fprintf(fd, "%d %d %d\n", rs, cs, d);
#endif
  else
#ifdef MC_64_BITS
    fprintf(fd, "%lld %lld\n", (long long int)rs, (long long int)cs);
#else
    fprintf(fd, "%d %d\n", rs, cs);
#endif
  fprintf(fd, "%d\n", nndg);

  ret = fwrite(SLONGDATA(image), sizeof(int32_t), N, fd);
  if (ret != N)
  {
#ifdef MC_64_BITS
    fprintf(stderr, "%s: only %lld items written\n", F_NAME, (long long int)ret);
#else
    fprintf(stderr, "%s: only %d items written\n", F_NAME, ret);
#endif
    exit(0);
  }

  fclose(fd);
} /* writelongimage() */

#ifdef HAVE_TIFF_LIB
struct xvimage *readtiffimage(const char *filename)
{
    struct xvimage *image=NULL;
    IMAGE *tiffimage = NULL;
    int datasize = 0;

    tiffimage = imloadtiff(filename);

    /* convert tiffimage into image */
    image = (struct xvimage *)malloc(sizeof(struct xvimage));
    /* fairly straightforward */
    image->row_size             = tiffimage->nx;
    image->col_size              = tiffimage->ny;
    image->depth_size          = tiffimage->nz;
    image->time_size            = tiffimage->nt;
    image->num_data_bands = tiffimage->nc; /* number of components */

    fprintf(stderr, "dimensions: nx=%d, ny=%d, nz=%d, nt=%d, nc=%d\n",
            tiffimage->nx, tiffimage->ny,tiffimage->nz, tiffimage->nt, tiffimage->nc);

    /* this may be a bit dicey */
    switch (tiffimage->pt) { /* pixel type */
    case IM_BINARY:
        image->data_storage_type = VFF_TYP_BIT; /* probably incorrect */
        datasize = 1;
        break;
    case IM_UINT1:
        image->data_storage_type = VFF_TYP_1_BYTE;
        datasize = 1;
        break;
    case IM_UINT2:
        image->data_storage_type = VFF_TYP_2_BYTE;
        datasize = 2;
        break;
    case IM_INT4:
        image->data_storage_type = VFF_TYP_4_BYTE;
        datasize =4;
        break;
    case IM_FLOAT:
        image->data_storage_type = VFF_TYP_FLOAT;
        datasize = 4;
        break;
    case IM_DOUBLE:
        image->data_storage_type = VFF_TYP_DOUBLE;
        datasize = 8;
        break;
    default:
        fprintf(stderr, "Unhandled data type %d\n", tiffimage->pt);
        break;
    }

    if (datasize > 0) {
        int i;
        long componentsize , totalsize;
        /* allocate memory for image */
        componentsize = datasize* \
            tiffimage->nx *                                     \
            tiffimage->ny *                                     \
            tiffimage->nz *                                     \
            tiffimage->nt;
        totalsize = componentsize *  tiffimage->nc;
        image->image_data = (void *)malloc(totalsize * sizeof(char));
        if (image->image_data != NULL) {
            for (i = 0 ; i < tiffimage->nc ; ++i) {
	      // the void* does not work in Windows(tm), but char* should have the same size and properties
	      // unit test for tiffs!
	      memcpy( (char*)image->image_data + i*componentsize, tiffimage->buff[i], componentsize);
            }
        } else {
            fprintf(stderr, "Not enough memory, requested=%ld bytes\n", totalsize);
        }
    }

    /* error conditions */
    if ((datasize == 0) || (image->image_data == NULL)) {
        fprintf(stderr, "Reading TIFF image failed\n");
        free(image);
        image=NULL;
    }

    return image;
}


/* ==================================== */
void writetiffimage(struct xvimage * image, const char *filename)
/* ==================================== */
#undef F_NAME
#define F_NAME "writetiffimage"
{
    int    slice, datasize = 0;
    IMAGE *tiffimage=NULL;

    tiffimage = (IMAGE*)malloc(sizeof(IMAGE));

    tiffimage->nx = image->row_size;
    tiffimage->ny = image->col_size;
    tiffimage->nz = image->depth_size;
    tiffimage->nt = image->time_size;
    tiffimage->nc = image->num_data_bands;

    /* std::cerr << "Dimensions: nx=" << tiffimage->nx
        <<  " ny=" << tiffimage->ny
        <<  " nz=" << tiffimage->nz
        <<  " nt=" << tiffimage->nt
        <<  " nc=" << tiffimage->nc << std::endl;
    */

    switch(image->data_storage_type) {
        case VFF_TYP_1_BYTE:
        tiffimage->pt = IM_UINT1;
        datasize = 1;
        break;
        case VFF_TYP_2_BYTE:
        tiffimage->pt = IM_UINT2;
        datasize = 2;
        break;
        case VFF_TYP_4_BYTE:
        tiffimage->pt = IM_INT4;
        datasize = 4;
        break;
        case VFF_TYP_FLOAT:
        tiffimage->pt = IM_FLOAT;
        datasize = 4;
        break;
        case VFF_TYP_DOUBLE:
        tiffimage->pt = IM_DOUBLE;
        datasize = 8;
        break;
        default:
            fprintf(stderr, "Unhandled data type %d\n ", (int)tiffimage->pt);
        break;
    }

    // exclude some types of data we cannot save yet
    do {
        if (datasize == 0) {
            break;
        }
        if (tiffimage->nc > 1) {
            fprintf(stderr, "Multi-component TIFF not supported yet\n");
            break;
        }
        if (tiffimage->nt > 1) {
            fprintf(stderr, "Time-series TIFF not supported yet\n");
            break;
        }
    //   now we should only have at most grey-level 3D images
        for (slice = 0 ; slice < tiffimage->nz ; ++slice) {
                long slicesize = tiffimage->nx*tiffimage->ny*datasize;
                void *fakebuff[1];
                int   start[2], end[2];
                int pi, sf, spp=1, bps;
		void *theslice;
		int retval;
                //std::cerr << "Slice= " << slice << std::endl;

                setTifftype(tiffimage->pt,     /* pixel type */
                            IM_SINGLE,     /* image type */
                            &pi,            /* photometric interpretation */
                            &sf,            /* sample format */
                            &spp,           /* samples per pixel (this parameter is I/O) */
                            &bps);
                //std::cerr << "Type= " << pi << "spp=" << spp << std::endl;

                theslice = (uint8_t*)image->image_data + slice*slicesize;
                fakebuff[0] = theslice;

                start[0] = 0;
                end[0] = tiffimage->nx-1;
                start[1] = 0;
                end[1] = tiffimage->ny-1;

                retval = save_tiff(fakebuff,	   /* output buffer  */
                          slice,   /* 0 = write, > 0 = append */
                          start,	   /* dimension start */
                          end,	   /* dimension end */
                          pi,	   /* photometric interpretation */
                          sf,	   /* storage format */
                          spp,	   /* samples per pixels */
                          bps,	   /* bits per pixel */
                          NULL, /* colour map (RGB) */
                          0,	   /* nb of colour in the LUT */
                          filename,  /* output file name  */
                          TIFF_CMP_NONE);  /* compression scheme */

                if (retval != 0)
                    fprintf(stderr, "TIFF writing failed with error %d\n", retval);
        }
    } while (0);

    return;
}


#endif
//HAVE_TIFF_LIB

/* ==================================== */
struct xvimage * readimage( const char *filename )
/* ==================================== */
#undef F_NAME
#define F_NAME "readimage"
{
  char buffer[BUFFERSIZE];
  FILE *fd = NULL;
  index_t rs, cs, ds, nb, N, i;
  struct xvimage * image;
  int32_t ascii, color;
  int32_t typepixel;
  int32_t c, ndgmax;
  double xdim=1.0, ydim=1.0, zdim=1.0;
  char *read;
  char tag;
  int bool;

  fd = pink_fopen_read(filename);

  if (!fd)
  {
    fprintf(stderr, "%s: file not found: %s\n", F_NAME, filename);
    return NULL;
  }

  read = fgets(buffer, BUFFERSIZE, fd);
    /* P5: raw byte or short bw  ; P2: ascii byte or short bw */
    /* P6: raw byte rgb ; P3: ascii rgb */
    /* P8: raw int32_t 2d-3d...  ==  extension MC */
    /* P9: raw float 2d-3d...  ==  extension MC */
    /* PA: ascii float 2d-3d...  ==  extension LN */
    /* PB: ascii int32_t 2d-3d...  ==  extension MC */
    /* PC: raw double 2d-3d...  ==  extension MC */
    /* PD: ascii double 2d-3d...  ==  extension LN */
    /* P7: raw byte 3d : OBSOLETE - left for compatibility */
    /* PE: raw single precision complex 2d-3d...  ==  extension MC */
    /* PF: ascii single precision complex 2d-3d...  ==  extension MC */
  if (!read)
  {
    fprintf(stderr, "%s: fgets returned without reading\n", F_NAME);
    return 0;
  }

  /* HT: TIFF */
  if ((strncmp(buffer, "II", 2) == 0) || (strncmp(buffer, "MM", 2) ==0)) {
      image = NULL;
#     ifdef HAVE_TIFF_LIB
      image=readtiffimage(filename);
#     else /* NOT HAVE_TIFF_LIB */
      fprintf(stderr, "TIFF image not supported at this time\n");
#     endif /* NOT HAVE_TIFF_LIB */
      /*otherwise, return NULL */
  } else { /* extended PGM/PNM format */

      if (buffer[0] != 'P')
      {   fprintf(stderr,"%s: invalid image format\n", F_NAME);
          return NULL;
      }
      tag = buffer[1];

#     ifdef MC_64_BITS
      c = sscanf(buffer+2, "%lld %lld %d", (long long int *)&rs, (long long int *)&cs, (int *)&ndgmax);
#     else /* NOT MC_64_BITS */
      c = sscanf(buffer+2, "%d %d %d", (int *)&rs, (int *)&cs, (int *)&ndgmax);
#     endif /* NOT MC_64_BITS */

      if (c == 3) /* format pgm MatLab : tout sur une ligne */
      {
          nb = ds = 1;
          goto readdata;
      }

      do
      {
          read = fgets(buffer, BUFFERSIZE, fd); /* commentaire */
          if (!read)
          {
              fprintf(stderr, "%s: fgets returned without reading\n", F_NAME);
              return 0;
          }
          if (strncmp(buffer, "#xdim", 5) == 0)
              sscanf(buffer+5, "%lf", &xdim);
          else if (strncmp(buffer, "#ydim", 5) == 0)
              sscanf(buffer+5, "%lf", &ydim);
          else if (strncmp(buffer, "#zdim", 5) == 0)
              sscanf(buffer+5, "%lf", &zdim);
      } while (!isdigit(buffer[0]));

#     ifdef MC_64_BITS
      c = sscanf(buffer, "%lld %lld %lld %lld", (long long int *)&rs, (long long int *)&cs, (long long int *)&ds, (long long int *)&nb);
#     else /* NOT MC_64_BITS */
      c = sscanf(buffer, "%d %d %d %d", (int *)&rs, (int *)&cs, (int *)&ds, (int *)&nb);
#     endif /* NOT MC_64_BITS */
      if (c == 2) nb = ds = 1;
      else if (c == 3) nb = 1;
      else if (c != 4)
      {   fprintf(stderr,"%s: invalid image format\n", F_NAME);
          return NULL;
      }

#     ifdef WARN_HUGE
      if (((int64_t)rs * (int64_t)cs * (int64_t)ds * (int64_t)nb) >= HUGE_IMAGE_SIZE)
      {
          fprintf(stderr, "%s: WARNING huge image\n", F_NAME);
      }
#     endif

      read = fgets(buffer, BUFFERSIZE, fd);
      if (!read)
      {
          fprintf(stderr, "%s: fgets returned without reading\n", F_NAME);
          return 0;
      }

      sscanf(buffer, "%d", (int *)&ndgmax);

      color = 0;
      switch (tag)
      {
        case '3':
	  color = 1; nb = 3;
        case '2':
          ascii = 1;
          if (ndgmax <= 255) typepixel = VFF_TYP_1_BYTE;
          else if (ndgmax <= 65535) typepixel = VFF_TYP_2_BYTE;
          else
          {
              fprintf(stderr,"%s: wrong ndgmax = %d\n", F_NAME, ndgmax);
              return(NULL);
          }
          break;
        case '6':
	  color = 1; nb = 3;
        case '5':
        case '7':
          ascii = 0;
          if (ndgmax <= 255) typepixel = VFF_TYP_1_BYTE;
          else if (ndgmax <= 65535) typepixel = VFF_TYP_2_BYTE;
          else
          {
              fprintf(stderr,"%s: wrong ndgmax = %d\n", F_NAME, ndgmax);
              return(NULL);
          }
          break;
        case '8': ascii = 0; typepixel = VFF_TYP_4_BYTE; break;
        case '9': ascii = 0; typepixel = VFF_TYP_FLOAT; break;
        case 'A': ascii = 1; typepixel = VFF_TYP_FLOAT; break;
        case 'B': ascii = 1; typepixel = VFF_TYP_4_BYTE; break;
        case 'C': ascii = 0; typepixel = VFF_TYP_DOUBLE; break;
        case 'D': ascii = 1; typepixel = VFF_TYP_DOUBLE; break;
        case 'E': ascii = 0; typepixel = VFF_TYP_COMPLEX; break;
        case 'F': ascii = 1; typepixel = VFF_TYP_COMPLEX; break;
        default:
          fprintf(stderr,"%s: invalid image format: P%c\n", F_NAME, tag);
          return NULL;
      } /* switch */

 readdata:
      image = allocmultimage(NULL, rs, cs, ds, 1, nb, typepixel);
      if (image == NULL)
      {   fprintf(stderr,"%s: alloc failed\n", F_NAME);
          return(NULL);
      }
      image->xdim = xdim;
      image->ydim = ydim;
      image->zdim = zdim;

      if (typepixel == VFF_TYP_1_BYTE)
      {
          if (ascii)
          {
	    if (color)
	    {
	      uint8_t *b1, *b2, *b3;
	      N = rs * cs;
	      b1 = UCHARDATA(image);
	      b2 = b1 + N;
	      b3 = b2 + N;
	      for (i = 0; i < N; i++)
              {
		bool = fscanf(fd, "%d", &c);
		b1[i] = (uint8_t)c;
		bool = fscanf(fd, "%d", &c);
		b2[i] = (uint8_t)c;
		bool = fscanf(fd, "%d", &c);
		b3[i] = (uint8_t)c;
              } // for i
	    } // if color
	    else
	    {
	      N = rs * cs * ds * nb;
	      for (i = 0; i < N; i++)
              {
                  bool = fscanf(fd, "%d", &c);
                  UCHARDATA(image)[i] = (uint8_t)c;
              } // for i
	    }
          } // if (ascii)
          else
          {
	    if (color)
	    {
	      uint8_t *b1, *b2, *b3;
	      N = rs * cs;
	      b1 = UCHARDATA(image);
	      b2 = b1 + N;
	      b3 = b2 + N;
	      for (i = 0; i < N; i++)
              {
		b1[i] = fgetc(fd);
		b2[i] = fgetc(fd);
		b3[i] = fgetc(fd);
              } // for i
	    } // if (color)
	    else
	    {
	      index_t ret;
	      N   = rs * cs * ds * nb;
              ret = fread(UCHARDATA(image), sizeof(char), N, fd);
              if (ret != N)
              {
#                 ifdef MC_64_BITS
                  fprintf(stderr,"%s: fread failed: %lld asked ; %lld read\n", F_NAME, (long long int)N, (long long int)ret);
#                 else /* NOT MC_64_BITS */
                  fprintf(stderr,"%s: fread failed: %d asked ; %d read\n", F_NAME, N, ret);
#                 endif /* NOT MC_64_BITS */
                  return(NULL);
              }
	    }
          }
      } // if (typepixel == VFF_TYP_1_BYTE)
      else
      {
          if (color)
	  {
	    fprintf(stderr,"%s: non supported color image type\n", F_NAME);
	    return(NULL);
	  }
	  N = rs * cs * ds * nb;
          if (typepixel == VFF_TYP_2_BYTE)
          {
              if (ascii)
              {
                  uint16_t tmp;
                  for (i = 0; i < N; i++)
                  {
                      bool = fscanf(fd, "%hd", &tmp); (SSHORTDATA(image)[i]) = tmp;
                  } /* for i */
              }
              else // Standard PGM format imposes big-endian for 2-byte images, but
              {    // standard PC architectures are little-endian
                  uint16_t tmp;
                  uint8_t tmp1;
                  for (i = 0; i < N; i++)
                  {
                      bool = fread(&tmp, sizeof(uint16_t), 1, fd);
                      // conversion big-endian -> little-endian
                      tmp1 = tmp & 0x00ff;
                      tmp = tmp >> 8;
                      tmp = tmp | (((uint16_t)tmp1) << 8);
                      (SSHORTDATA(image)[i]) = (int16_t)tmp;
                  }
              }
          } /* if (typepixel == VFF_TYP_2_BYTE) */
          else
              if (typepixel == VFF_TYP_4_BYTE)
              {
                  if (ascii)
                  {
                      long int tmp;
                      for (i = 0; i < N; i++)
                      {
                          bool = fscanf(fd, "%ld", &tmp); (SLONGDATA(image)[i]) = (int32_t)tmp;
                      } /* for i */
                  }
                  else
                  {
                      index_t ret = fread(SLONGDATA(image), sizeof(int32_t), N, fd);
                      if (ret != N)
                      {
#                         ifdef MC_64_BITS
                          fprintf(stderr,"%s: fread failed: %lld asked ; %lld read\n", F_NAME, (long long int)N, (long long int)ret);
#                         else /* NOT MC_64_BITS */
                          fprintf(stderr,"%s: fread failed: %d asked ; %d read\n", F_NAME, N, ret);
#                         endif /* NOT MC_64_BITS */
                          return(NULL);
                      }
                  }
              } /* if (typepixel == VFF_TYP_4_BYTE) */
              else
                  if (typepixel == VFF_TYP_FLOAT)
                  {
                      if (ascii)
                      {
                          for (i = 0; i < N; i++)
                          {
                              bool = fscanf(fd, "%f", &(FLOATDATA(image)[i]));
                          } /* for i */
                      }
                      else
                      {
                          index_t ret = fread(FLOATDATA(image), sizeof(float), N, fd);
                          if (ret != N)
                          {
#                             ifdef MC_64_BITS
                              fprintf(stderr,"%s: fread failed: %lld asked ; %lld read\n", F_NAME, (long long int)N, (long long int)ret);
#                             else /* NOT MC_64_BITS */
                              fprintf(stderr,"%s: fread failed: %d asked ; %d read\n", F_NAME, N, ret);
#                             endif /* NOT MC_64_BITS */
                              return(NULL);
                          }
                      }
                  } /* if (typepixel == VFF_TYP_FLOAT) */
                  else
                      if (typepixel == VFF_TYP_DOUBLE)
                      {
                          if (ascii)
                          {
                              for (i = 0; i < N; i++)
                              {
                                  bool = fscanf(fd, "%lf", &(DOUBLEDATA(image)[i]));
                              } /* for i */
                          }
                          else
                          {
                              index_t ret = fread(DOUBLEDATA(image), sizeof(double), N, fd);
                              if (ret != N)
                              {
#                                 ifdef MC_64_BITS
                                  fprintf(stderr,"%s: fread failed: %lld asked ; %lld read\n", F_NAME, (long long int)N, (long long int)ret);
#                                 else /* NOT MC_64_BITS */
                                  fprintf(stderr,"%s: fread failed: %d asked ; %d read\n", F_NAME, N, ret);
#                                 endif /* NOT MC_64_BITS */
                                  return(NULL);
                              }
                          }
                      } /* if (typepixel == VFF_TYP_DOUBLE) */
                      else
                          if (typepixel == VFF_TYP_COMPLEX)
                          {
                              if (ascii)
                              {
                                  for (i = 0; i < N+N; i++)
                                  {
                                      bool = fscanf(fd, "%f", &(FLOATDATA(image)[i]));
                                  } /* for i */
                              }
                              else
                              {
                                  index_t ret = fread(FLOATDATA(image), sizeof(float), N+N, fd);
                                  if (ret != N+N)
                                  {
#                                     ifdef MC_64_BITS
                                      fprintf(stderr,"%s: fread failed: %lld asked ; %lld read\n", F_NAME, (long long int)(N+N), (long long int)ret);
#                                     else /* NOT MC_64_BITS */
                                      fprintf(stderr,"%s: fread failed: %d asked ; %d read\n", F_NAME, N+N, ret);
#                                     endif /* NOT MC_64_BITS */
                                      return(NULL);
                                  }
                              }
                          } /* if (typepixel == VFF_TYP_COMPLEX) */
      } // else if (typepixel == VFF_TYP_1_BYTE)
  } /* if TIFF */
  fclose(fd);

  return image;
} /* readimage() */

/* ==================================== */
struct xvimage * readheader(const char *filename)
/* ==================================== */
#undef F_NAME
#define F_NAME "readheader"
{
  char buffer[BUFFERSIZE];
  FILE *fd = NULL;
  index_t rs, cs, d;
  struct xvimage * image;
  int32_t ascii;
  int32_t typepixel;
  int32_t c, ndgmax;
  double xdim=1.0, ydim=1.0, zdim=1.0;
  char *read;

  fd = pink_fopen_read(filename);

  if (!fd)
  {
    fprintf(stderr, "%s: file not found: %s\n", F_NAME, filename);
    return NULL;
  }

  read = fgets(buffer, BUFFERSIZE, fd);
  if (!read)
  {
    fprintf(stderr, "%s: fgets returned without reading\n", F_NAME);
    return 0;
  }
  if (buffer[0] != 'P')
  {
    fprintf(stderr,"%s: invalid image format: %c%c\n", F_NAME, buffer[0], buffer[1]);
    return NULL;
  }
  switch (buffer[1])
  {
    case '2': ascii = 1; typepixel = VFF_TYP_1_BYTE; break;
    case '5':
    case '7': ascii = 0; typepixel = VFF_TYP_1_BYTE; break;
    case '8': ascii = 0; typepixel = VFF_TYP_4_BYTE; break;
    case '9': ascii = 0; typepixel = VFF_TYP_FLOAT; break;
    case 'A': ascii = 1; typepixel = VFF_TYP_FLOAT; break;
    case 'B': ascii = 1; typepixel = VFF_TYP_4_BYTE; break;
    case 'C': ascii = 0; typepixel = VFF_TYP_DOUBLE; break;
    case 'D': ascii = 1; typepixel = VFF_TYP_DOUBLE; break;
    case 'E': ascii = 0; typepixel = VFF_TYP_COMPLEX; break;
    case 'F': ascii = 1; typepixel = VFF_TYP_COMPLEX; break;
    default:
      fprintf(stderr,"%s: invalid image format: %c%c\n", F_NAME, buffer[0], buffer[1]);
      return NULL;
  } /* switch */

  do
  {
    read = fgets(buffer, BUFFERSIZE, fd); /* commentaire */
    if (!read)
    {
      fprintf(stderr, "%s: fgets returned without reading\n", F_NAME);
      return 0;
    }
    if (strncmp(buffer, "#xdim", 5) == 0)
      sscanf(buffer+5, "%lf", &xdim);
    else if (strncmp(buffer, "#ydim", 5) == 0)
      sscanf(buffer+5, "%lf", &ydim);
    else if (strncmp(buffer, "#zdim", 5) == 0)
      sscanf(buffer+5, "%lf", &zdim);
  } while (!isdigit(buffer[0]));

# ifdef MC_64_BITS
  c = sscanf(buffer, "%lld %lld %lld", (long long int *)&rs, (long long int *)&cs, (long long int *)&d);
# else /* NOT MC_64_BITS */
  c = sscanf(buffer, "%d %d %d", (int *)&rs, (int *)&cs, (int *)&d);
# endif /* NOT MC_64_BITS */
  if (c == 2) d = 1;
  else if (c != 3)
  {   fprintf(stderr,"%s: invalid image format\n", F_NAME);
      return NULL;
  }

  read = fgets(buffer, BUFFERSIZE, fd);
  if (!read)
  {
    fprintf(stderr, "%s: fgets returned without reading\n", F_NAME);
    return 0;
  }

  sscanf(buffer, "%d", (int *)&ndgmax);

  image = allocheader(NULL, rs, cs, d, typepixel);
  if (image == NULL)
  {   fprintf(stderr,"%s: alloc failed\n", F_NAME);
      return(NULL);
  }
  image->xdim = xdim;
  image->ydim = ydim;
  image->zdim = zdim;

  fclose(fd);

  return image;
} /* readheader() */

/* ==================================== */
struct xvimage * readse(const char *filename, index_t *x, index_t *y, index_t*z)
/* ==================================== */
#undef F_NAME
#define F_NAME "readse"
/*
Specialisation de readimage pour les elements structurants.
L'origine est donnee dans le fichier pgm par une ligne de commentaire
de la forme :
#origin x y [z]
*/
{
  char buffer[BUFFERSIZE];
  FILE *fd = NULL;
  index_t rs, cs, d, i;
  index_t N;
  struct xvimage * image;
  int32_t ascii;
  int32_t typepixel;
  int32_t c, ndgmax;
  int32_t dimorigin = 0;
  char *read;
  int bool;

  fd = pink_fopen_read(filename);

  if (!fd)
  {
    fprintf(stderr, "%s: file not found: %s\n", F_NAME, filename);
    return NULL;
  }

  read = fgets(buffer, BUFFERSIZE, fd);
  if (!read)
  {
    fprintf(stderr, "%s: fgets returned without reading\n", F_NAME);
    return 0;
  }
  if (buffer[0] != 'P')
  {   fprintf(stderr,"%s: invalid image format\n", F_NAME);
      return NULL;
  }
  switch (buffer[1])
  {
    case '2': ascii = 1; typepixel = VFF_TYP_1_BYTE; break;
    case '5':
    case '7': ascii = 0; typepixel = VFF_TYP_1_BYTE; break;
    default:
      fprintf(stderr,"%s: invalid image format\n", F_NAME);
      return NULL;
  } /* switch */

  do
  {
    read = fgets(buffer, BUFFERSIZE, fd); /* commentaire */
    if (!read)
    {
      fprintf(stderr, "%s: fgets returned without reading\n", F_NAME);
      return 0;
    }
    if (strncmp(buffer, "#origin", 7) == 0)
    {
#ifdef MC_64_BITS
      dimorigin = sscanf(buffer+7, "%lld %lld %lld", (long long int *)x, (long long int *)y, (long long int *)z);
#else
      dimorigin = sscanf(buffer+7, "%d %d %d", (int *)x, (int *)y, (int *)z);
#endif
#ifdef VERBOSE
#ifdef MC_64_BITS
      if (dimorigin == 2) fprintf(stderr, "%s: origin %lld %lld\n", F_NAME, (long long int)*x, (long long int)*y);
#else
      if (dimorigin == 2) fprintf(stderr, "%s: origin %d %d\n", F_NAME, *x, *y);
#endif
#ifdef MC_64_BITS
      if (dimorigin == 3) fprintf(stderr, "%s: origin %lld %lld %lld\n", F_NAME, (long long int)*x, (long long int)*y, (long long int)*z);
#else
      if (dimorigin == 3) fprintf(stderr, "%s: origin %d %d %d\n", F_NAME, *x, *y, *z);
#endif
#endif
    }
  } while (buffer[0] == '#');

  if (!dimorigin)
  {
#ifdef VERBOSE
    fprintf(stderr,"%s: origin missing for structuring element\n", F_NAME);
#endif
    return NULL;
  }

#ifdef MC_64_BITS
  c = sscanf(buffer, "%lld %lld %lld", (long long int *)&rs, (long long int *)&cs, (long long int *)&d);
#else
  c = sscanf(buffer, "%d %d %d", (int *)&rs, (int *)&cs, (int *)&d);
#endif
  if (c != dimorigin)
  {   fprintf(stderr,"%s: incompatible origin and image dimensions\n", F_NAME);
      return NULL;
  }
  if (c == 2) d = 1;
  else if (c != 3)
  {   fprintf(stderr,"%s: invalid image format\n", F_NAME);
      return NULL;
  }

  read = fgets(buffer, BUFFERSIZE, fd);
  if (!read)
  {
    fprintf(stderr, "%s: fgets returned without reading\n", F_NAME);
    return NULL;
  }

  sscanf(buffer, "%d", (int *)&ndgmax);
  N = rs * cs * d;

  image = allocimage(NULL, rs, cs, d, typepixel);
  if (image == NULL)
  {   fprintf(stderr,"%s: alloc failed\n", F_NAME);
      return NULL;
  }

  if (typepixel == VFF_TYP_1_BYTE)
  {
    if (ascii)
    {
      if (ndgmax == 255)
        for (i = 0; i < N; i++)
        {
          bool = fscanf(fd, "%d", &c);
          UCHARDATA(image)[i] = (uint8_t)c;
        } /* for i */
      else if (ndgmax == 65535)
        for (i = 0; i < N; i++)
        {
          bool = fscanf(fd, "%d", &c);
          UCHARDATA(image)[i] = (uint8_t)(c/256);
        } /* for i */
      else
      {   fprintf(stderr,"%s: wrong ndgmax = %d\n", F_NAME, ndgmax);
          return(NULL);
      }
    }
    else
    {
      index_t ret = fread(UCHARDATA(image), sizeof(char), N, fd);
      if (ret != N)
      {
#ifdef MC_64_BITS
        fprintf(stderr,"%s: fread failed: %lld asked ; %lld read\n", F_NAME, (long long int)N, (long long int)ret);
#else
        fprintf(stderr,"%s: fread failed: %d asked ; %d read\n", F_NAME, N, ret);
#endif
        return NULL;
      }
    }
  } /* if (typepixel == VFF_TYP_1_BYTE) */
  else
  if (typepixel == VFF_TYP_4_BYTE)
  {
    index_t ret = fread(SLONGDATA(image), sizeof(int32_t), N, fd);
    if (ret != N)
    {
#ifdef MC_64_BITS
      fprintf(stderr,"%s: fread failed: %lld asked ; %lld read\n", F_NAME, (long long int)N, (long long int)ret);
#else
      fprintf(stderr,"%s: fread failed: %d asked ; %d read\n", F_NAME, N, ret);
#endif
      return NULL;
    }
  } /* if (typepixel == VFF_TYP_4_BYTE) */

  fclose(fd);

  return image;
} /* readse() */

/* ==================================== */
int32_t readrgbimage(
  const char *filename,
  struct xvimage ** r,
  struct xvimage ** g,
  struct xvimage ** b)
/* ==================================== */
#undef F_NAME
#define F_NAME "readrgbimage"
{
  char buffer[BUFFERSIZE];
  FILE *fd = NULL;
  index_t rs, cs, i, ds;
  index_t N;
  int32_t ascii = 0;
  int32_t c;
  char *read;
  int32_t nndg, ndgmax;
  int bool;

  fd = pink_fopen_read(filename);

  if (!fd)
  {
    fprintf(stderr, "%s: file not found: %s\n", F_NAME, filename);
    return 0;
  }

  read = fgets(buffer, BUFFERSIZE, fd); /* P6: raw int32_t rgb ; P3: ascii rgb */
  if (!read)
  {
    fprintf(stderr, "%s: fgets returned without reading\n", F_NAME);
    return 0;
  }
  if (buffer[0] != 'P')
  {   fprintf(stderr,"%s: invalid image format\n", F_NAME);
      return 0;
  }

  switch (buffer[1])
  {
    case '3': ascii = 1; break;
    case '6': ascii = 0; break;
    default:
      fprintf(stderr,"%s: invalid image format\n", F_NAME);
      return 0;
  } /* switch */

#ifdef MC_64_BITS
  c = sscanf(buffer+2, "%lld %lld %d", (long long int *)&rs, (long long int *)&cs, &ndgmax);
#else
  c = sscanf(buffer+2, "%d %d %d", (int *)&rs, (int *)&cs, (int *)&ndgmax);
#endif

  if (c == 3) /* format ppm MatLab : tout sur une ligne */
  {
    ds = 1;
    goto readdata;
  }



  do
  {
    read = fgets(buffer, BUFFERSIZE, fd); /* commentaire */
    if (!read)
    {
      fprintf(stderr, "%s: fgets returned without reading\n", F_NAME);
      return 0;
    }
  } while (!isdigit(buffer[0]));

#ifdef MC_64_BITS
  c = sscanf(buffer, "%lld %lld", (long long int *)&rs, (long long int *)&cs);
#else
  c = sscanf(buffer, "%d %d", (int *)&rs, (int *)&cs);
#endif
  if (c != 2)
  {   fprintf(stderr,"%s: invalid image format\n", F_NAME);
      return 0;
  }

  read = fgets(buffer, BUFFERSIZE, fd);
  if (!read)
  {
    fprintf(stderr, "%s: fgets returned without reading\n", F_NAME);
    return 0;
  }
  sscanf(buffer, "%d", (int *)&nndg);

  readdata:
  N = rs * cs;

  *r = allocimage(NULL, rs, cs, 1, VFF_TYP_1_BYTE);
  *g = allocimage(NULL, rs, cs, 1, VFF_TYP_1_BYTE);
  *b = allocimage(NULL, rs, cs, 1, VFF_TYP_1_BYTE);
  if ((*r == NULL) && (*g == NULL) && (*b == NULL))
  {   fprintf(stderr,"%s: allocimage failed\n", F_NAME);
      return(0);
  }

  if (ascii)
    for (i = 0; i < N; i++)
    {
      bool = fscanf(fd, "%d", &c);
      (UCHARDATA(*r))[i] = (uint8_t)c;
      bool = fscanf(fd, "%d", &c);
      (UCHARDATA(*g))[i] = (uint8_t)c;
      bool = fscanf(fd, "%d", &c);
      (UCHARDATA(*b))[i] = (uint8_t)c;
    } /* for i */
  else
    for (i = 0; i < N; i++)
    {
      (UCHARDATA(*r))[i] = fgetc(fd);
      (UCHARDATA(*g))[i] = fgetc(fd);
      (UCHARDATA(*b))[i] = fgetc(fd);
    } /* for i */

  fclose(fd);
  return 1;
} /* readrgbimage() */

/* ==================================== */
struct xvimage * readlongimage(const char *filename)
/* ==================================== */
#undef F_NAME
#define F_NAME "readlongimage"
/*
   obsolete - utiliser maintenant readimage
*/
{
  char buffer[BUFFERSIZE];
  FILE *fd = NULL;
  index_t rs, cs, d, ret;
  index_t N;
  struct xvimage * image;
  int32_t c, nndg;
  char *read;

  fd = pink_fopen_read(filename);

  if (!fd)
  {
    fprintf(stderr, "%s: file not found: %s\n", F_NAME, filename);
    return NULL;
  }

  read = fgets(buffer, BUFFERSIZE, fd); /* P8: raw int32_t 3d  ==  extension MC */
  if (!read)
  {
    fprintf(stderr, "%s: fgets returned without reading\n", F_NAME);
    return 0;
  }

  if ((buffer[0] != 'P') || (buffer[1] != '8'))
  {   fprintf(stderr,"%s: invalid image format\n", F_NAME);
      return NULL;
  }

  do
  {
    read = fgets(buffer, BUFFERSIZE, fd); /* commentaire */
    if (!read)
    {
      fprintf(stderr, "%s: fgets returned without reading\n", F_NAME);
      return 0;
    }
  } while (buffer[0] == '#');

#ifdef MC_64_BITS
  c = sscanf(buffer, "%lld %lld %lld", (long long int *)&rs, (long long int *)&cs, (long long int *)&d);
#else
  c = sscanf(buffer, "%d %d %d", (int *)&rs, (int *)&cs, (int *)&d);
#endif
  if (c == 2) d = 1;
  else if (c != 3)
  {   fprintf(stderr,"%s: invalid image format - c = %d \n", F_NAME, c);
      return NULL;
  }

  read = fgets(buffer, BUFFERSIZE, fd);
  if (!read)
  {
    fprintf(stderr, "%s: fgets returned without reading\n", F_NAME);
    return 0;
  }

  sscanf(buffer, "%d", (int *)&nndg);
  N = rs * cs * d;

  image = allocimage(NULL, rs, cs, d, VFF_TYP_4_BYTE);
  if (image == NULL)
  {   fprintf(stderr,"%s: alloc failed\n", F_NAME);
      return(NULL);
  }

  ret = fread(SLONGDATA(image), sizeof(int32_t), N, fd);
  if (ret != N)
  {
#ifdef MC_64_BITS
    fprintf(stderr,"%s: fread failed: %lld asked ; %lld read\n", F_NAME, (long long int)N, (long long int)ret);
#else
    fprintf(stderr,"%s: fread failed: %d asked ; %d read\n", F_NAME, N, ret);
#endif
    return(NULL);
  }

  fclose(fd);
  return image;
} /* readlongimage() */

/* =========================================================================== */
/* =========================================================================== */
/* BMP files */
/* =========================================================================== */
/* =========================================================================== */

struct BITMAPFILEHEADER {   /* size 14 bytes */
  char Signature[2];        /* size 2 bytes: 'BM' */
  uint32_t FileSize;   /* size 4 bytes: File size in bytes */
  uint32_t reserved;   /* size 4 bytes: unused (=0) */
  uint32_t DataOffset; /* size 4 bytes: File offset to Raster Data */
};

struct BITMAPINFOHEADER {    /* size 40 bytes */
  uint32_t Size;        /* size 4 bytes: Size of InfoHeader =40 */
  uint32_t Width;       /* size 4 bytes: Bitmap Width */
  uint32_t Height;      /* size 4 bytes: Bitmap Height */
  uint16_t Planes;     /* size 2 bytes: Number of Planes (=1) */
  uint16_t BitCount;   /* size 2 bytes: Bits per Pixel    */
                             /*   1 = monochrome palette. NumColors = 1
                                  4 = 4bit palletized. NumColors = 16
                                  8 = 8bit palletized. NumColors = 256
                                  16 = 16bit RGB. NumColors = 65536 (?)
                                  24 = 24bit RGB. NumColors = 16M
			     */
  uint32_t Compression; /* size 4 bytes: Type of Compression    */
                             /*
                                  0 = BI_RGB   no compression
                                  1 = BI_RLE8 8bit RLE encoding
                                  2 = BI_RLE4 4bit RLE encoding
			     */
  uint32_t ImageSize;   /* size 4 bytes: (compressed) Size of Image   */
                             /* It is valid to set this =0 if Compression = 0 */
  uint32_t XpixelsPerM; /* size 4 bytes: horizontal resolution: Pixels/meter */
  uint32_t YpixelsPerM; /* size 4 bytes: vertical resolution: Pixels/meter */
  uint32_t ColorsUsed;  /* size 4 bytes: Number of actually used colors */
  uint32_t ColorsImportant; /* size 4 bytes: Number of important colors (0 = all) */
};

/*
       ColorTable
                      4 * NumColors bytes
                                        present only if Info.BitsPerPixel <= 8
                                        colors should be ordered by importance

          Red
                      1 byte
                                        Red intensity
          Green
                      1 byte
                                        Green intensity
          Blue
                      1 byte
                                        Blue intensity
          reserved
                      1 byte
                                        unused (=0)
         repeated NumColors times

       Raster Data
                      Info.ImageSize bytes
                                        The pixel data

Raster Data encoding

       Depending on the image's BitCount and on the Compression flag
       there are 6 different encoding schemes. All of them share the
       following:

       Pixels are stored bottom-up, left-to-right. Pixel lines are
       padded with zeros to end on a 32bit (4byte) boundary. For
       uncompressed formats every line will have the same number of bytes.
       Color indices are zero based, meaning a pixel color of 0
       represents the first color table entry, a pixel color of 255
       (if there are that many) represents the 256th entry. For images with more
       than 256 colors there is no color table.


       Raster Data encoding for 1bit / black & white images

       BitCount = 1 Compression = 0
       Every byte holds 8 pixels, its highest order bit representing
       the leftmost pixel of those. There are 2 color table entries. Some
       readers will ignore them though, and assume that 0 is black and 1 is white.
       If you are storing black and white pictures you should
       stick to this, with any other 2 colors this is not an issue.
       Remember padding with zeros up to a 32bit boundary (This can be up to
       31 zeros/pixels!)


       Raster Data encoding for 4bit / 16 color images

       BitCount = 4 Compression = 0
       Every byte holds 2 pixels, its high order 4 bits representing the left of those.
       There are 16 color table entries. These colors do not
       have to be the 16 MS-Windows standard colors. Padding each line with
       zeros up to a 32bit boundary will result in up to 28 zeros
       = 7 'wasted pixels'.


       Raster Data encoding for 8bit / 256 color images

       BitCount = 8 Compression = 0
       Every byte holds 1 pixel. There are 256 color table entries.
       Padding each line with zeros up to a 32bit boundary will result in up to
       3 bytes of zeros = 3 'wasted pixels'.


       Raster Data encoding for 16bit / hicolor images

       BitCount = 16 Compression = 0
       Every 2bytes / 16bit holds 1 pixel.
       <information missing: the 16 bit was introduced together with
       Video For Windows? Is it a memory-only-format?>
       The pixels are no color table pointers. There are no color table entries.
       Padding each line with zeros up to a 16bit boundary will
       result in up to 2 zero bytes.


       Raster Data encoding for 24bit / truecolor images

       BitCount = 24 Compression = 0
       Every 4bytes / 32bit holds 1 pixel. The first holds its red,
       the second its green, and the third its blue intensity. The fourth byte is
       reserved and should be zero. There are no color table entries.
       The pixels are no color table pointers. No zero padding necessary.


       Raster Data compression for 4bit / 16 color images

       BitCount = 4 Compression = 2
       The pixel data is stored in 2bytes / 16bit chunks.  The first of these
       specifies the number of consecutive pixels with the same pair
       of color. The second byte defines two color indices. The resulting
       pixel pattern will be interleaved high-order 4bits and low order 4
       bits (ABABA...). If the first byte is zero, the second defines an escape code.
       The End-of-Bitmap is zero padded to end on a 32bit boundary.
       Due to the 16bit-ness of this structure this will always be either
       two zero bytes or none.

       n (byte 1)
                c (Byte 2)
                                                               Description
       >0
                any
                         n pixels are to be drawn. The 1st, 3rd, 5th, ... pixels'
                         color is in c's high-order 4 bits, the even pixels' color
                         is in c's low-order 4 bits. If both color indices are the same,
                         it results in just n pixels of color c
       0
                0
                         End-of-line
       0
                1
                         End-of-Bitmap
       0
                2
                         Delta. The following 2 bytes define an unsigned offset
                         in x and y direction (y being up) The skipped pixels
                         should get a color zero.
       0
                >=3
                         The following c bytes will be read as single pixel colors
                         just as in uncompressed files. up to 12 bits of zeros
                         follow, to put the file/memory pointer on a 16bit boundary again.


                                 Example for 4bit RLE
       Compressed Data
                                           Expanded data
       03 04
                        0 4 0
       05 06
                        0 6 0 6 0
       00 06 45 56 67 00
                        4 5 5 6 6 7
       04 78
                        7 8 7 8
       00 02 05 01
                        Move 5 right and 1 up. (Windows docs say down, which is wrong)
       00 00
                        End-of-line
       09 1E
                        1 E 1 E 1 E 1 E 1
       00 01
                        EndofBitmap
       00 00
                        Zero padding for 32bit boundary




       Raster Data compression for 8bit / 256 color images

       BitCount = 8 Compression = 1
       The pixel data is stored in 2bytes / 16bit chunks.  The first of these
       specifies the number of consecutive pixels with the same
       color. The second byte defines their color index. If the first byte
       is zero, the second defines an escape code. The End-of-Bitmap
       is zero padded to end on a 32bit boundary. Due to the 16bit-ness of
       this structure this will always be either two zero bytes or none.

       n (byte 1)
                c (Byte 2)
                                                               Description
       >0
                any
                         n pixels of color number c
       0
                0
                         End-of-line
       0
                1
                         EndOfBitmap
       0
                2
                         Delta. The following 2 bytes define an unsigned offset
                         in x and y direction (y being up) The skipped pixels
                         should get a color zero.
       0
                >=3
                         The following c bytes will be read as single pixel colors
                         just as in uncompressed files. A zero follows, if c is
                         odd, putting the file/memory pointer on a 16bit boundary again.


                                 Example for 8bit RLE
       Compressed Data
                                           Expanded data
       03 04
                        04 04 04
       05 06
                        06 06 06 06 06
       00 03 45 56 67 00
                        45 56 67
       02 78
                        78 78
       00 02 05 01
                        Move 5 right and 1 up. (Windows docs say down, which is wrong)
       00 00
                        End-of-line
       09 1E
                        1E 1E 1E 1E 1E 1E 1E 1E 1E
       00 01
                        End-of-bitmap
       00 00
                        Zero padding for 32bit boundary

*/

void freadushort(uint16_t *ptr, FILE* fd)
{
  uint16_t tmp; uint8_t t1, t2;
  t1 = getc(fd);  t2 = getc(fd);
  tmp = t2;
  tmp = tmp*256+t1;
  *ptr = tmp;
}

void freadulong(uint32_t *ptr, FILE* fd)
{
  uint32_t tmp; uint8_t t1, t2, t3, t4;
  t1 = getc(fd);  t2 = getc(fd);  t3 = getc(fd);  t4 = getc(fd);
  tmp = t4;
  tmp = tmp*256+t3;
  tmp = tmp*256+t2;
  tmp = tmp*256+t1;
  *ptr = tmp;
}

/* =============================================================== */
int32_t readbmp(const char *filename, struct xvimage ** r, struct xvimage ** g, struct xvimage ** b)
/* =============================================================== */
#undef F_NAME
#define F_NAME "readbmp"
{
  FILE *fd = NULL;
  struct BITMAPFILEHEADER FileHeader;
  struct BITMAPINFOHEADER InfoHeader;
  uint8_t *R, *G, *B;
  int32_t i, j, rs, cs;
  size_t fread_out;

  fd = pink_fopen_read(filename);

  if (!fd)
  {
    fprintf(stderr, "%s: cannot open file: %s\n", F_NAME, filename);
    return 0;
  }

  fread_out = fread(&(FileHeader.Signature), sizeof(char), 2, fd);
  freadulong(&(FileHeader.FileSize), fd);
  freadulong(&(FileHeader.reserved), fd);
  freadulong(&(FileHeader.DataOffset), fd);
#ifdef VERBOSE
  printf("signature = %c%c\n", FileHeader.Signature[0], FileHeader.Signature[1]);
  printf("file size = %ld\n", FileHeader.FileSize);
  printf("reserved = %ld\n", FileHeader.reserved);
  printf("data offset = %ld\n", FileHeader.DataOffset);
#endif
  freadulong(&(InfoHeader.Size), fd);
  freadulong(&(InfoHeader.Width), fd);
  freadulong(&(InfoHeader.Height), fd);
  freadushort(&(InfoHeader.Planes), fd);
  freadushort(&(InfoHeader.BitCount), fd);
  freadulong(&(InfoHeader.Compression), fd);
  freadulong(&(InfoHeader.ImageSize), fd);
  freadulong(&(InfoHeader.XpixelsPerM), fd);
  freadulong(&(InfoHeader.YpixelsPerM), fd);
  freadulong(&(InfoHeader.ColorsUsed), fd);
  freadulong(&(InfoHeader.ColorsImportant), fd);
#ifdef VERBOSE
  printf("Size = %d\n", InfoHeader.Size);
  printf("Width = %d\n", InfoHeader.Width);
  printf("Height = %d\n", InfoHeader.Height);
  printf("Planes = %d\n", InfoHeader.Planes);
  printf("BitCount = %d\n", InfoHeader.BitCount);
  printf("Compression = %d\n", InfoHeader.Compression);
  printf("ImageSize = %d\n", InfoHeader.ImageSize);
  printf("XpixelsPerM = %d\n", InfoHeader.XpixelsPerM);
  printf("YpixelsPerM = %d\n", InfoHeader.YpixelsPerM);
  printf("ColorsUsed = %d\n", InfoHeader.ColorsUsed);
  printf("ColorsImportant = %d\n", InfoHeader.ColorsImportant);
#endif
  if ((InfoHeader.Compression != 0) && (InfoHeader.BitCount != 24))
  {
    fprintf(stderr, "restricted bmp format conversion:\n");
    fprintf(stderr, "compression tag must be 0 (No compression), found: %d\n", InfoHeader.Compression);
    fprintf(stderr, "bitcount/pixel must be 24 (True color), found: %d\n", InfoHeader.BitCount);
    fprintf(stderr, "cannot process file\n");
    return 0;
  }
  rs = InfoHeader.Width;
  cs = InfoHeader.Height;
  *r = allocimage(NULL, rs, cs, 1, VFF_TYP_1_BYTE);
  *g = allocimage(NULL, rs, cs, 1, VFF_TYP_1_BYTE);
  *b = allocimage(NULL, rs, cs, 1, VFF_TYP_1_BYTE);
  if ((*r == NULL) || (*g == NULL) || (*b == NULL))
  {
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return 0;
  }
  R = ((UCHARDATA(*r)));
  G = ((UCHARDATA(*g)));
  B = ((UCHARDATA(*b)));
  for (j = cs-1; j >= 0 ; j--)
  for (i = 0; i < rs; i++)
  {
    B[(j*rs)+i] = (uint8_t)getc(fd);
    G[(j*rs)+i] = (uint8_t)getc(fd);
    R[(j*rs)+i] = (uint8_t)getc(fd);
    /* (void)getc(fd); */
  }
  fclose(fd);
  return 1;
} /* readbmp() */

void fwriteushort(uint16_t us, FILE* fd)
{
  putc((uint8_t)(us & 0xff), fd); us = us >> 8;
  putc((uint8_t)(us & 0xff), fd); us = us >> 8;
}

void fwriteulong(uint32_t ul, FILE* fd)
{
  putc((uint8_t)(ul & 0xff), fd); ul = ul >> 8;
  putc((uint8_t)(ul & 0xff), fd); ul = ul >> 8;
  putc((uint8_t)(ul & 0xff), fd); ul = ul >> 8;
  putc((uint8_t)(ul & 0xff), fd); ul = ul >> 8;
}

/* =============================================================== */
void writebmp(
  struct xvimage * redimage,
  struct xvimage * greenimage,
  struct xvimage * blueimage,
  const char *filename)
/* =============================================================== */
#undef F_NAME
#define F_NAME "writebmp"
{
  FILE *fd = NULL;
  uint8_t *R, *G, *B;
  int32_t i, j, rs, cs, N;

  rs = rowsize(redimage);
  cs = colsize(redimage);
  N = rs * cs;

  if ((rs != rowsize(greenimage)) || (cs != colsize(greenimage)) ||
      (rs != rowsize(blueimage)) || (cs != colsize(blueimage)))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    exit(0);
  }

  R = UCHARDATA(redimage);
  G = UCHARDATA(greenimage);
  B = UCHARDATA(blueimage);

  fd = pink_fopen_write(filename);

  if (!fd)
  {
    fprintf(stderr, "%s: cannot open file: %s\n", F_NAME, filename);
    exit(0);
  }

  putc('B', fd); putc('M', fd);
  fwriteulong(N + 54, fd);
  fwriteulong(0, fd);
  fwriteulong(54, fd);
  fwriteulong(40, fd);
  fwriteulong(rs, fd);
  fwriteulong(cs, fd);
  fwriteushort(1, fd);
  fwriteushort(24, fd);
  fwriteulong(0, fd);
  fwriteulong(N, fd);
  fwriteulong(0, fd);
  fwriteulong(0, fd);
  fwriteulong(0, fd);
  fwriteulong(0, fd);

  for (j = cs-1; j >= 0 ; j--)
  for (i = 0; i < rs; i++)
  {
    putc(B[(j*rs)+i], fd);
    putc(G[(j*rs)+i], fd);
    putc(R[(j*rs)+i], fd);
  }

  fclose(fd);
} /* writebmp() */

/* =========================================================================== */
/* =========================================================================== */
/* RGB files */
/* =========================================================================== */
/* =========================================================================== */

/* RGB file format is documented on:

http://reality.sgi.com/grafica/sgiimage.html

*/

struct RGBFILEHEADER {       /* size 108 bytes */
  uint16_t magic;      /* size 2 bytes: magic number = 474 */
  uint8_t compression; /* size 1 byte: 0 for no compression */
  uint8_t bytespercha; /* size 1 byte: nb. bytes per channel */
  uint16_t dim;        /* size 2 bytes: nb. channels */
  uint16_t width;      /* size 2 bytes: image row size */
  uint16_t height;     /* size 2 bytes: image col size */
  uint16_t components; /* size 2 bytes: components */
  uint32_t mincol;      /* size 4 bytes: 0 */
  uint32_t maxcol;      /* size 4 bytes: 255 */
  uint32_t dummy;       /* size 4 bytes: dummy */
  char name[80];             /* size 80 bytes: image name or comment */
  uint32_t cmaptype;    /* size 4 bytes: 0 for NORMAL RGB */
}; /** plus 404 bytes dummy padding to make header 512 bytes **/

/* =============================================================== */
int32_t readrgb(const char *filename, struct xvimage ** r, struct xvimage ** g, struct xvimage ** b)
/* =============================================================== */
#undef F_NAME
#define F_NAME "readrgb"
{
  FILE *fd = NULL;
  struct RGBFILEHEADER FileHeader;
  uint8_t *R, *G, *B;
  index_t i, j, rs, cs;
  index_t N;

  fd = pink_fopen_read(filename);

  if (!fd)
  {
    fprintf(stderr, "%s: cannot open file: %s\n", F_NAME, filename);
    return 0;
  }

  freadushort(&(FileHeader.magic), fd);
  FileHeader.compression = (uint8_t)getc(fd);
  FileHeader.bytespercha = (uint8_t)getc(fd);
  freadushort(&(FileHeader.dim), fd);
  freadushort(&(FileHeader.width), fd);
  freadushort(&(FileHeader.height), fd);

  if (FileHeader.magic != 474)
  {
    fprintf(stderr, "bad rgb format magic number: 474 expected, %d found\n",
            FileHeader.magic);
    return 0;
  }

  if (FileHeader.compression != 0)
  {
    fprintf(stderr, "restricted rgb format conversion:\n");
    fprintf(stderr, "compression tag must be 0 (No compression), found: %d\n", FileHeader.compression);
    return 0;
  }
  rs = FileHeader.width;
  cs = FileHeader.height;
  N = rs * cs;
  *r = allocimage(NULL, rs, cs, 1, VFF_TYP_1_BYTE);
  *g = allocimage(NULL, rs, cs, 1, VFF_TYP_1_BYTE);
  *b = allocimage(NULL, rs, cs, 1, VFF_TYP_1_BYTE);
  if ((*r == NULL) || (*g == NULL) || (*b == NULL))
  {
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    return 0;
  }
  R = ((UCHARDATA(*r)));
  G = ((UCHARDATA(*g)));
  B = ((UCHARDATA(*b)));

  for (i = 0; i < 502; i++)  /** padding bytes **/
    (void)getc(fd);

  for (j = cs-1; j >= 0 ; j--)
  for (i = 0; i < rs; i++)  /** red bytes **/
    R[(j*rs)+i] = (uint8_t)getc(fd);
  for (j = cs-1; j >= 0 ; j--)
  for (i = 0; i < rs; i++)  /** green bytes **/
    G[(j*rs)+i] = (uint8_t)getc(fd);
  for (j = cs-1; j >= 0 ; j--)
  for (i = 0; i < rs; i++)  /** blue bytes **/
    B[(j*rs)+i] = (uint8_t)getc(fd);
  fclose(fd);
  return 1;
} /* readrgb() */

/* =============================================================== */
void writelist2(const char *filename, int32_t *x, int32_t *y, int32_t npoints)
/* =============================================================== */
#undef F_NAME
#define F_NAME "writelist2"
{
  FILE *fd = NULL;
  int32_t i;

  fd = pink_fopen_write(filename);
  if (!fd)
  {
    fprintf(stderr, "%s: cannot open file: %s\n", F_NAME, filename);
    exit(1);
  }

  fprintf(fd, "b %d\n", npoints);
  for (i = 0; i < npoints; i++)
    fprintf(fd, "%d %d\n", x[i], y[i]);

  fclose(fd);
} // writelist2()

/* =============================================================== */
void writelist3(const char *filename, int32_t *x, int32_t *y, int32_t *z, int32_t npoints)
/* =============================================================== */
#undef F_NAME
#define F_NAME "writelist3"
{
  FILE *fd = NULL;
  int32_t i;

  fd = pink_fopen_write(filename);
  if (!fd)
  {
    fprintf(stderr, "%s: cannot open file: %s\n", F_NAME, filename);
    exit(1);
  }

  fprintf(fd, "B %d\n", npoints);
  for (i = 0; i < npoints; i++)
    fprintf(fd, "%d %d %d\n", x[i], y[i], z[i]);

  fclose(fd);
} // writelist3()
