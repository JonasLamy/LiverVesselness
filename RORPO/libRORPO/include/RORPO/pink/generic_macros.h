/*
 * File:		generic_macros.h
 *
 * Written by:		Image Analysis Group staff,
 * 			CSIRO Mathematical and Information Sciences.
 *
 * Date:		March 2001
 *
 *
 * CSIRO Mathematical and Information Sciences is the owner of all
 * copyright subsisting in the software contained in this file. It may
 * not be disclosed to, or used by, anyone without the prior approval 
 * of the Chief, CSIRO Mathematical and Information Sciences.
 *
*/

#ifndef GENERIC_MACROS_H
#define GENERIC_MACROS_H
/* A macro for concatenating the function name,its suffix and the mask type */

#undef XCAT2
#undef CAT2
#define CAT2(A, B) A ## B
#define XCAT2(A, B) CAT2(A, B)


#undef XCAT3
#undef CAT3
#define CAT3(A,B,C) A ## B ## C
#define XCAT3(A,B,C) CAT3(A,B,C)

#undef XCAT4
#undef CAT4
#define CAT4(A,B,C,D) A ## B ## C ## D
#define XCAT4(A,B,C,D) CAT4(A,B,C,D)

/* A macro for mentioning a concatenated function name in error messages */
#undef STRINGIFY
#undef XSTRINGIFY
#define XSTRINGIFY(A) #A
#define STRINGIFY(A) XSTRINGIFY(A)

#endif
