/*------------------------------------------------------------------------
 *
 * Prototypes for the minimal version of the
 * Library of Image Analysis Routines
 *
 * From Imview
 *
 * Hugues Talbot	 4 Jan 2001
 *
 *-----------------------------------------------------------------------*/

#ifndef LIARP_
#define LIARP_

// pure definition of macros,


#if !defined(liarmax) && !defined(HAS_MINMAX)
#define liarmax(a,b) ((a)<(b)?(b):(a))
#endif

#if !defined(liarmin) && !defined(HAS_MINMAX)
#define liarmin(a,b) ((a)>(b)?(b):(a))
#endif


#include "generic_macros.h"

/* pixel types */

#ifndef PIX_TYPE
#define PIX_TYPE unsigned char
#define PIX_TYPE_MAX 0xFF
#define PIX_TYPE_MIN 0
#endif

#ifndef UINT1_TYPE
#define UINT1_TYPE unsigned char
#define UINT1_TYPE_MAX 0xff
#define UINT1_TYPE_MIN 0
#endif

#ifndef CHAR_TYPE
#define INT1_TYPE char
#define INT1_TYPE_MAX 127
#define INT1_TYPE_MIN -127
#endif

#ifndef INT1_TYPE
#define INT1_TYPE char
#define INT1_TYPE_MAX 127
#define INT1_TYPE_MIN -127
#endif

#ifndef INT2_TYPE
#define INT2_TYPE short
#define INT2_TYPE_MAX 32767
#define INT2_TYPE_MIN -32767
#endif

#ifndef UINT2_TYPE
#define UINT2_TYPE unsigned short
#define UINT2_TYPE_MAX 65535
#define UINT2_TYPE_MIN 0
#endif


#ifndef INT4_TYPE
#define INT4_TYPE int
#define INT4_TYPE_MAX 2147483647
#define INT4_TYPE_MIN -2147483647
#endif

#ifndef UINT4_TYPE
#define UINT4_TYPE unsigned int
#define UINT4_TYPE_MAX 4294967295
#define UINT4_TYPE_MIN 0
#endif

#ifndef SEED_TYPE
#define SEED_TYPE int
#define SEED_TYPE_MAX  0x7FFFFFFF
#define SEED_TYPE_MIN   -0x80000000
#endif

#ifndef SHRT_TYPE
#define SHRT_TYPE short int
#define SHRT_TYPE_MAX   32767
#define SHRT_TYPE_MIN  -32767
#endif

#ifndef  USHRT_TYPE
#define USHRT_TYPE  unsigned short
#define USHRT_TYPE_MAX   65535
#define USHRT_TYPE_MIN   0
#endif

#ifndef INT_TYPE
#define INT_TYPE  int
#define INT_TYPE_MAX   2147483647
#define INT_TYPE_MIN   -2147483647
#endif

#ifndef UINT_TYPE
#define UINT_TYPE unsigned
#define UINT_TYPE_MAX 4294967295U
#define UNINT_TYPE_MIN 0
#endif

#ifndef LONG_TYPE
#define LONG_TYPE long
#define LONG_TYPE_MAX   2147483647
#define LONG_TYPE_MIN  -2147483647
#endif

#ifndef ULONG_TYPE
#define ULONG_TYPE  unsigned long
#define ULONG_TYPE_MAX 4294967295U
#define ULONG_TYPE_MIN 0
#endif

#ifndef FLT_TYPE
#define FLT_TYPE  float
#define FLT_TYPE_MAX  3.40282347e+38
#define FLT_TYPE_MIN  1.17549435e-38
#endif

#ifndef DBL_TYPE
#define DBL_TYPE double
#define DBL_TYPE_MAX 1.7976931348623157e+308
#define DBL_TYPE_MIN 2.2250738585072014e-308
#endif

#define BINARY_CODE 10
#define INT1_CODE   30
#define CHAR_CODE   30   /* CHAR is signed */
#define PIX_CODE    31   /* PIX is unsigned */
#define UINT1_CODE  31
#define SHORT_CODE  35
#define INT2_CODE   35
#define UINT2_CODE  37
#define INT4_CODE   40
#define INT_CODE    40
#define UINT4_CODE  42
#define UINT_CODE   42
#define INT8_CODE   43
#define UINT8_CODE  44
#define FLOAT_CODE  45
#define DBL_CODE    50
#define DOUBLE_CODE 50

/* default string length */
#define DFLTSTRLEN 1024

/* messages functions */
#define LIARassert(x)  assert(x)

/* prototypes from liarmsgs.c */
#define dbgprintf LIARdebug
#define errprintf LIARerror


// exact copy of the structure in wmisc.h
// located in the c2liar
// these codes are defined in liarlmts.h
    /* typedef enum */
    /* { */
    /*     IM_DEFAULT  = -1,     /\* used in arith function when specifying the output type *\/     */
    /*     IM_BADPIXEL = 0,      /\* error flag for unknown/illegal pixel types (enum value 0)*\/ */

    /*     /\* 1 byte pixel types *\/ */
    /*     IM_BINARY   = BINARY_CODE,     /\* binary (enum value 10) *\/  */
    /*     IM_CHAR     = CHAR_CODE,       /\* char,int (enum value 30)*\/  */
    /*     IM_INT1     = INT1_CODE,       /\* char,int (enum value 30)*\/ */
    /*     IM_UINT1    = UINT1_CODE,      /\* unsigned int (enum value 31)*\/ */

    /*     /\* 2 byte pixel types *\/ */
    /*     IM_SHORT    = SHORT_CODE,      /\* int (enum value 35)*\/ */
    /*     IM_INT2     = INT2_CODE,       /\* int (enum value 35)*\/ */
    /*     IM_UINT2    = UINT2_CODE,      /\* unsigned int (enum value 37)*\/ */

    /*     /\* 4 byte pixel types *\/ */
    /*     IM_INT      = INT_CODE,        /\* int (enum value 40)*\/ */
    /*     IM_INT4     = INT4_CODE,       /\* int (enum value 40)*\/ */
    /*     IM_UINT4    = UINT4_CODE,      /\* unsigned int (enum value 42)*\/ */

    /*     /\* 8 byte pixel types *\/ */
    /*     IM_INT8     = INT8_CODE,       /\* int (enum value 43)*\/ */
    /*     IM_UINT8    = UINT8_CODE,      /\* unsigned int (enum value 44)*\/ */

    /*     /\* real-valued pixel types *\/ */
    /*     IM_FLOAT    = FLOAT_CODE,      /\* float (enum value 45)*\/ */
    /*     IM_DOUBLE   = DOUBLE_CODE      /\* double (enum value 50)*\/ */
    /* } */
    /*     liar_pixtype; */

    // extremely weird Hugues Talbot	21 Dec 2010
    typedef enum {
        IM_BINARY = 0,
        IM_INT1 = 1,
        IM_UINT1 = 2,
        IM_INT2 = 3,
        IM_UINT2 = 4,
        IM_INT4 = 5,
        IM_UINT4 = 6,
        IM_INT8 = 7,
        IM_UINT8 = 8,
        IM_FLOAT = 9,
        IM_DOUBLE = 10
    } pixtype;

    // flat series (2D and 3D)
//#include "fseries.h"
//#include "fseries3d.h"

    typedef void (*computerankfunc)(PIX_TYPE *in, 
				    PIX_TYPE *out,
				    PIX_TYPE *offset,
				    int nx,
				    int nk);
// pure C function declarations
#ifdef __cplusplus
extern "C" {
#endif

    int LIARdebug(const char *fmt, ...);
    int LIARerror(const char *msg, ...);
    long *bresenham3d(int dx, int dy, int dz,
		    int imwidth, int imheight, int imdepth,
		    int *ol, int *period);
    long *periodic3d(int dx, int dy, int dz,
		     int imwidth, int imheight, int imdepth,
		      int *ol, int *period);
    long *bresenham(int xin,int yin,int angle,int width,int *period);
    long *periodic(int x, int y, int angle, int width, int *period);

    /* from chngeval.c */
    void changePIXval(PIX_TYPE *IN, long size,
                      PIX_TYPE val1,
                      PIX_TYPE val2);
    
    void changeSEEDval(SEED_TYPE *IN, long size,
                       SEED_TYPE val1,
                       SEED_TYPE val2);

    INT4_TYPE testINTneighs(SEED_TYPE *v,LONG_TYPE *func,INT4_TYPE n);

    void getsubset_3d(void *in,         /* Input buffer  */
                      void **out,       /* Output buffer */
                      int size,         /* Image pixel size */
                      int inx,          /* nb of cols (input) */
                      int iny,          /* nb of rows (input) */
                      int inz,          /* nb of slices (input) */
                      int *outx,        /* nb of cols (output) */
                      int *outy,        /* nb of rows (output) */
                      int *outz,        /* nb of slices (output) */
                      int offsetx,      /* offset value in x */
                      int offsety,      /* offset value in y */
                      int offsetz);      /* offset value in z */
    
    void addborder_3d(void *in,         /* Input buffer  */
                      void **out,       /* Output buffer */
                      int size,         /* Image pixel size */
                      int inx,          /* nb of cols (input) */
                      int iny,          /* nb of rows (input) */
                      int inz,          /* nb of slices (input) */
                      int *outx,        /* nb of cols (output) */
                      int *outy,        /* nb of rows (output) */
                      int *outz,        /* nb of slices (output) */
                      int offsetx,      /* offset value in x */
                      int offsety,      /* offset value in y */
                      int offsetz);      /* offset value in z */
    
    
    /* from setpxbrd.c */
    void setPIXborder(PIX_TYPE *image, int nx, int ny, int nz,
                      PIX_TYPE val);

    /* from setneigh.c */
    int setNeighFunc3d(long *Nf,	 /* pointer to a long vector of size 26  */
                       int con,	 /*  */
                       int nx,	 /*  */
                       int ny,	 /*  */
                       int nz);	 /*  */

    /* from lsrgrow3d.c */
    int lsrgrow3d(void **bufin,	/* input buffer */
                  SEED_TYPE **bufout,	/* output buffer */
                  SEED_TYPE *seeds,	/* seed image */
                  int inimagetype,               /*  pixel type of input image */
                  int inpixsize,                  /* pixel size of input image */
                  int seedpixsize,             /* pixel size of seeds image */
                  int inx,		/* nb of cols in the input */
                  int iny,		/* nb of rows in the input */
                  int inz,		/* nb of slices in the input */
                  int comps,		/* nb of components in the input */
                  int metric,	/* distance measures */
                  int borders,	/* to display region borders only */
                  int connect,	/* connectivity of the image */
                  int showgrey);	/* output greyscale using pixel avge */
        

#ifdef __cplusplus
}
#endif


#endif /* LIARP_ */
