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
/*
 Operateurs morphologiques geodesiques
 methode : propagation des changements par fifo
 d'apres la these de Michel Grimaud (pp 22)
 Michel Couprie - juillet 1996
 Update 12/02/2010: MC - fix memory leakage (missing calls to IndicsTermine)
*/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <RORPO/pink/mccodimage.h>
#include <RORPO/pink/mcfifo.h>
#include <RORPO/pink/mcindic.h>
#include <RORPO/pink/mcutil.h>
#include <RORPO/pink/lgeodesic.h>

/* ==================================== */
int32_t lgeodilat(
        struct xvimage *g,
        struct xvimage *f,
        int32_t connex,
        int32_t niter)
/* dilatation geodesique de g sous f */
/* g : image marqueur */
/* f : image masque */
/* connex : 4 ou 8 (2d) ou 6 ou 18 ou 26 (3d) */
/* niter : nombre d'iterations (ou -1 pour saturation) */
/* resultat dans g */
/* ==================================== */
#undef F_NAME
#define F_NAME "lgeodilat"
{
  COMPARE_SIZE(f, g);
  ACCEPTED_TYPES3(f, VFF_TYP_1_BYTE, VFF_TYP_2_BYTE, VFF_TYP_4_BYTE);
  ACCEPTED_TYPES3(g, VFF_TYP_1_BYTE, VFF_TYP_2_BYTE, VFF_TYP_4_BYTE);
  if (datatype(f) != datatype(g))
  {
    fprintf(stderr, "%s: bad image type(s)\n", F_NAME);
    return 0;
  }

    if (depth(f) == 1)
    {
      if (datatype(f) == VFF_TYP_1_BYTE) return lgeodilat2d(g, f, connex, niter);
      if (datatype(f) == VFF_TYP_2_BYTE) return lgeodilat2d_short(g, f, connex, niter);
      if (datatype(f) == VFF_TYP_4_BYTE) return lgeodilat2d_long(g, f, connex, niter);
    }
    if (datatype(f) == VFF_TYP_1_BYTE) return lgeodilat3d(g, f, connex, niter);
    if (datatype(f) == VFF_TYP_2_BYTE) return lgeodilat3d_short(g, f, connex, niter);
    if (datatype(f) == VFF_TYP_4_BYTE) return lgeodilat3d_long(g, f, connex, niter);
    return 1;
} // lgeodilat()

/* ==================================== */
int32_t lgeodilat2d(
        struct xvimage *g,
        struct xvimage *f,
        int32_t connex,
        int32_t niter)
/* dilatation geodesique de g sous f */
/* g : image marqueur */
/* f : image masque */
/* connex : 4 ou 8 */
/* niter : nombre d'iterations (ou -1 pour saturation) */
/* resultat dans g */
/* ==================================== */
#undef F_NAME
#define F_NAME "lgeodilat2d"
{
  index_t nbchang, iter;
  index_t x;                       /* index muet de pixel */
  index_t y;                       /* index muet (generalement un voisin de x) */
  index_t k;                       /* index muet */
  index_t rs = rowsize(g);         /* taille ligne */
  index_t cs = colsize(g);         /* taille colonne */
  index_t N = rs * cs;             /* taille image */
  uint8_t *G = UCHARDATA(g);      /* l'image marqueur */
  uint8_t *F = UCHARDATA(f);      /* l'image masque */
  uint8_t *H;                     /* image de travail */
  uint8_t *temp;
  uint8_t sup;
  Fifo * FIFO[2];
  int32_t incr_vois;

  switch (connex)
  {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
  } /* switch (connex) */

  if ((rowsize(f) != rs) || (colsize(f) != cs))
  {
    fprintf(stderr, "%s: incompatible sizes\n", F_NAME);
    return 0;
  }

  if (depth(f) != 1)
  {
    fprintf(stderr, "%s: only works for 2d images\n", F_NAME);
    return 0;
  }

  FIFO[0] = CreeFifoVide(N);
  FIFO[1] = CreeFifoVide(N);
  if ((FIFO[0] == NULL) || (FIFO[1] == NULL))
  {   fprintf(stderr,"%s : CreeFifoVide failed\n", F_NAME);
      return(0);
  }

  IndicsInit(N);

  for (x = 0; x < N; x++)      /* mise en fifo initiale de tous les points */
  {
    FifoPush(FIFO[1], x);
    Set(x, 1);
  }

  H = (uint8_t *)calloc(1,N*sizeof(char));
  if (H == NULL)
  {   fprintf(stderr,"%s : malloc failed for H\n", F_NAME);
      return(0);
  }

  for (x = 0; x < N; x++)      /* force G à être <= F */
    if (G[x] > F[x]) G[x] = F[x];

  iter = 0;
  do
  {
    iter += 1;
    nbchang = 0;
    while (! FifoVide(FIFO[iter % 2]))
    {
      x = FifoPop(FIFO[iter % 2]);
      UnSet(x, iter % 2);
      sup = G[x];
      for (k = 0; k < 8; k += incr_vois)
      {
        y = voisin(x, k, rs, N);
        if ((y != -1) && (G[y] > sup)) sup = G[y];
      } /* for k */

      sup = mcmin(sup, F[x]);
      if (G[x] != sup) /* changement: on enregistre x ainsi que ses voisins */
      {
        nbchang += 1;
        if (! IsSet(x, (iter + 1) % 2))
        {
          FifoPush(FIFO[(iter + 1) % 2], x);
          Set(x, (iter + 1) % 2);
	}
        for (k = 0; k < 8; k += 1)
        {
          y = voisin(x, k, rs, N);
          if ((y != -1) && (! IsSet(y, (iter + 1) % 2)))
          {
            FifoPush(FIFO[(iter + 1) % 2], y);
            Set(y, (iter + 1) % 2);
          }
        } /* for k */
      }
      H[x] = sup;

    } /* while ! FifoVide */

    /* echange les roles de G et H */
    temp = G;
    G = H;
    H = temp;

#ifdef VERBOSE
    printf("iteration %d, nbchang %d\n", iter, nbchang);
#endif
  } while (((niter == -1) || (iter < niter)) && (nbchang != 0));

  /* remet le resultat dans g si necessaire */
  if (G != UCHARDATA(g))
  {
    for (x = 0; x < N; x++)
      (UCHARDATA(g))[x] = G[x];
    free(G);
  }
  else
    free(H);

  FifoTermine(FIFO[0]);
  FifoTermine(FIFO[1]);
  IndicsTermine();
  return 1;
} /* lgeodilat2d() */

/* ==================================== */
int32_t lgeodilat2d_short(
        struct xvimage *g,
        struct xvimage *f,
        int32_t connex,
        int32_t niter)
/* dilatation geodesique de g sous f */
/* g : image marqueur */
/* f : image masque */
/* connex : 4 ou 8 */
/* niter : nombre d'iterations (ou -1 pour saturation) */
/* resultat dans g */
/* ==================================== */
#undef F_NAME
#define F_NAME "lgeodilat2d_short"
{
  index_t nbchang, iter;
  index_t x;                       /* index muet de pixel */
  index_t y;                       /* index muet (generalement un voisin de x) */
  index_t k;                       /* index muet */
  index_t rs = rowsize(g);         /* taille ligne */
  index_t cs = colsize(g);         /* taille colonne */
  index_t N = rs * cs;             /* taille image */
  int16_t *G = SSHORTDATA(g);      /* l'image marqueur */
  int16_t *F = SSHORTDATA(f);      /* l'image masque */
  int16_t *H;                      /* image de travail */
  int16_t *temp;
  int16_t sup;
  Fifo * FIFO[2];
  int32_t incr_vois;

  switch (connex)
  {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
  } /* switch (connex) */

  if ((rowsize(f) != rs) || (colsize(f) != cs))
  {
    fprintf(stderr, "%s: incompatible sizes\n", F_NAME);
    return 0;
  }

  if (depth(f) != 1)
  {
    fprintf(stderr, "%s: only works for 2d images\n", F_NAME);
    return 0;
  }

  FIFO[0] = CreeFifoVide(N);
  FIFO[1] = CreeFifoVide(N);
  if ((FIFO[0] == NULL) || (FIFO[1] == NULL))
  {   fprintf(stderr,"%s : CreeFifoVide failed\n", F_NAME);
      return(0);
  }

  IndicsInit(N);

  for (x = 0; x < N; x++)      /* mise en fifo initiale de tous les points */
  {
    FifoPush(FIFO[1], x);
    Set(x, 1);
  }

  H = (int16_t *)calloc(1,N*sizeof(int16_t));
  if (H == NULL)
  {   fprintf(stderr,"%s : malloc failed for H\n", F_NAME);
      return(0);
  }

  for (x = 0; x < N; x++)      /* force G à être <= F */
    if (G[x] > F[x]) G[x] = F[x];

  iter = 0;
  do
  {
    iter += 1;
    nbchang = 0;
    while (! FifoVide(FIFO[iter % 2]))
    {
      x = FifoPop(FIFO[iter % 2]);
      UnSet(x, iter % 2);
      sup = G[x];
      for (k = 0; k < 8; k += incr_vois)
      {
        y = voisin(x, k, rs, N);
        if ((y != -1) && (G[y] > sup)) sup = G[y];
      } /* for k */

      sup = mcmin(sup, F[x]);
      if (G[x] != sup) /* changement: on enregistre x ainsi que ses voisins */
      {
        nbchang += 1;
        if (! IsSet(x, (iter + 1) % 2))
        {
          FifoPush(FIFO[(iter + 1) % 2], x);
          Set(x, (iter + 1) % 2);
	}
        for (k = 0; k < 8; k += 1)
        {
          y = voisin(x, k, rs, N);
          if ((y != -1) && (! IsSet(y, (iter + 1) % 2)))
          {
            FifoPush(FIFO[(iter + 1) % 2], y);
            Set(y, (iter + 1) % 2);
          }
        } /* for k */
      }
      H[x] = sup;

    } /* while ! FifoVide */

    /* echange les roles de G et H */
    temp = G;
    G = H;
    H = temp;

#ifdef VERBOSE
    printf("iteration %d, nbchang %d\n", iter, nbchang);
#endif
  } while (((niter == -1) || (iter < niter)) && (nbchang != 0));

  /* remet le resultat dans g si necessaire */
  if (G != SSHORTDATA(g))
  {
    for (x = 0; x < N; x++)
      (SSHORTDATA(g))[x] = G[x];
    free(G);
  }
  else
    free(H);

  FifoTermine(FIFO[0]);
  FifoTermine(FIFO[1]);
  IndicsTermine();
  return 1;
} /* lgeodilat2d_short() */

/* ==================================== */
int32_t lgeodilat2d_long(
        struct xvimage *g,
        struct xvimage *f,
        int32_t connex,
        int32_t niter)
/* dilatation geodesique de g sous f */
/* g : image marqueur */
/* f : image masque */
/* connex : 4 ou 8 */
/* niter : nombre d'iterations (ou -1 pour saturation) */
/* resultat dans g */
/* ==================================== */
#undef F_NAME
#define F_NAME "lgeodilat2d_long"
{
  index_t nbchang, iter;
  index_t x;                       /* index muet de pixel */
  index_t y;                       /* index muet (generalement un voisin de x) */
  index_t k;                       /* index muet */
  index_t rs = rowsize(g);         /* taille ligne */
  index_t cs = colsize(g);         /* taille colonne */
  index_t N = rs * cs;             /* taille image */
  int32_t *G = SLONGDATA(g);      /* l'image marqueur */
  int32_t *F = SLONGDATA(f);      /* l'image masque */
  int32_t *H;                     /* image de travail */
  int32_t *temp;
  int32_t sup;
  Fifo * FIFO[2];
  int32_t incr_vois;

  switch (connex)
  {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
  } /* switch (connex) */

  if ((rowsize(f) != rs) || (colsize(f) != cs))
  {
    fprintf(stderr, "%s: incompatible sizes\n", F_NAME);
    return 0;
  }

  if (depth(f) != 1)
  {
    fprintf(stderr, "%s: only works for 2d images\n", F_NAME);
    return 0;
  }

  FIFO[0] = CreeFifoVide(N);
  FIFO[1] = CreeFifoVide(N);
  if ((FIFO[0] == NULL) || (FIFO[1] == NULL))
  {   fprintf(stderr,"%s : CreeFifoVide failed\n", F_NAME);
      return(0);
  }

  IndicsInit(N);

  for (x = 0; x < N; x++)      /* mise en fifo initiale de tous les points */
  {
    FifoPush(FIFO[1], x);
    Set(x, 1);
  }

  H = (int32_t *)calloc(1,N*sizeof(int32_t));
  if (H == NULL)
  {   fprintf(stderr,"%s : malloc failed for H\n", F_NAME);
      return(0);
  }

  for (x = 0; x < N; x++)      /* force G à être <= F */
    if (G[x] > F[x]) G[x] = F[x];

  iter = 0;
  do
  {
    iter += 1;
    nbchang = 0;
    while (! FifoVide(FIFO[iter % 2]))
    {
      x = FifoPop(FIFO[iter % 2]);
      UnSet(x, iter % 2);
      sup = G[x];
      for (k = 0; k < 8; k += incr_vois)
      {
        y = voisin(x, k, rs, N);
        if ((y != -1) && (G[y] > sup)) sup = G[y];
      } /* for k */

      sup = mcmin(sup, F[x]);
      if (G[x] != sup) /* changement: on enregistre x ainsi que ses voisins */
      {
        nbchang += 1;
        if (! IsSet(x, (iter + 1) % 2))
        {
          FifoPush(FIFO[(iter + 1) % 2], x);
          Set(x, (iter + 1) % 2);
	}
        for (k = 0; k < 8; k += 1)
        {
          y = voisin(x, k, rs, N);
          if ((y != -1) && (! IsSet(y, (iter + 1) % 2)))
          {
            FifoPush(FIFO[(iter + 1) % 2], y);
            Set(y, (iter + 1) % 2);
          }
        } /* for k */
      }
      H[x] = sup;

    } /* while ! FifoVide */

    /* echange les roles de G et H */
    temp = G;
    G = H;
    H = temp;

#ifdef VERBOSE
    printf("iteration %d, nbchang %d\n", iter, nbchang);
#endif
  } while (((niter == -1) || (iter < niter)) && (nbchang != 0));

  /* remet le resultat dans g si necessaire */
  if (G != SLONGDATA(g))
  {
    for (x = 0; x < N; x++)
      (SLONGDATA(g))[x] = G[x];
    free(G);
  }
  else
    free(H);

  FifoTermine(FIFO[0]);
  FifoTermine(FIFO[1]);
  IndicsTermine();
  return 1;
} /* lgeodilat2d_long() */

/* ==================================== */
int32_t lreconsdilat(
        struct xvimage *g,
        struct xvimage *f,
        int32_t connex)
/* reconstruction de g sous f */
/* g : image marqueur */
/* f : image masque */
/* resultat dans g */
/* ==================================== */
{
  return lgeodilat(g, f, connex, -1);
}

/* ==================================== */
int32_t lgeoeros(
        struct xvimage *g,
        struct xvimage *f,
        int32_t connex,
        int32_t niter)
/* reconstruction par erosion de g au dessus de f */
/* g : image marqueur */
/* f : image masque */
/* connex : 4 ou 8 */
/* niter : nombre d'iterations (ou -1 pour saturation) */
/* resultat dans g */
/* ==================================== */
#undef F_NAME
#define F_NAME "lgeoeros"
{
  index_t nbchang, iter;
  index_t x;                       /* index muet de pixel */
  index_t y;                       /* index muet (generalement un voisin de x) */
  index_t k;                       /* index muet */
  index_t rs = rowsize(g);         /* taille ligne */
  index_t cs = colsize(g);         /* taille colonne */
  index_t N = rs * cs;             /* taille image */
  uint8_t *G = UCHARDATA(g);      /* l'image marqueur (au dessus de f) */
  uint8_t *F = UCHARDATA(f);      /* l'image masque */
  uint8_t *H;                     /* image de travail */
  uint8_t *temp;
  uint8_t inf;
  Fifo * FIFO[2];
  int32_t incr_vois;

  switch (connex)
  {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
  } /* switch (connex) */

  if ((rowsize(f) != rs) || (colsize(f) != cs))
  {
    fprintf(stderr, "%s: incompatible sizes\n", F_NAME);
    return 0;
  }

  if (depth(f) != 1)
  {
    fprintf(stderr, "%s: only works for 2d images\n", F_NAME);
    return 0;
  }

  H = (uint8_t *)calloc(1,N*sizeof(char));
  if (H == NULL)
  {   fprintf(stderr,"%s : malloc failed for H\n", F_NAME);
      return(0);
  }

  FIFO[0] = CreeFifoVide(N);
  FIFO[1] = CreeFifoVide(N);
  if ((FIFO[0] == NULL) || (FIFO[1] == NULL))
  {   fprintf(stderr,"%s : CreeFifoVide failed\n", F_NAME);
      return(0);
  }

  IndicsInit(N);

  for (x = 0; x < N; x++)      /* mise en fifo initiale de tous les points */
  {
    FifoPush(FIFO[1], x);
    Set(x, 1);
  }

  for (x = 0; x < N; x++)      /* force G à être >= F */
    if (G[x] < F[x]) G[x] = F[x];

  iter = 0;
  do
  {
    iter += 1;
    nbchang = 0;
    while (! FifoVide(FIFO[iter % 2]))
    {
      x = FifoPop(FIFO[iter % 2]);
      UnSet(x, iter % 2);
      inf = G[x];
      for (k = 0; k < 8; k += incr_vois)
      {
        y = voisin(x, k, rs, N);
        if ((y != -1) && (G[y] < inf)) inf = G[y];
      } /* for k */

      inf = mcmax(inf, F[x]);
      if (G[x] != inf)           /* le point a change : on l'enregistre ainsi que ses voisins */
      {
        nbchang += 1;
        if (! IsSet(x, (iter + 1) % 2))
        {
          FifoPush(FIFO[(iter + 1) % 2], x);
          Set(x, (iter + 1) % 2);
	}
        for (k = 0; k < 8; k += 1)
        {
          y = voisin(x, k, rs, N);
          if ((y != -1) && (! IsSet(y, (iter + 1) % 2)))
          {
            FifoPush(FIFO[(iter + 1) % 2], y);
            Set(y, (iter + 1) % 2);
          }
        } /* for k */
      }
      H[x] = inf;

    } /* while ! FifoVide */

    /* echange les roles de G et H */
    temp = G;
    G = H;
    H = temp;

#ifdef VERBOSE
    printf("iteration %d, nbchang %d\n", iter, nbchang);
#endif
  } while (((niter == -1) || (iter < niter)) && (nbchang != 0));

  /* remet le resultat dans g si necessaire */
  if (G != UCHARDATA(g))
  {
    for (x = 0; x < N; x++)
      (UCHARDATA(g))[x] = G[x];
    free(G);
  }
  else
    free(H);

  FifoTermine(FIFO[0]);
  FifoTermine(FIFO[1]);
  IndicsTermine();
  return 1;
} /* lgeoeros() */

/* ==================================== */
int32_t ldeletecomp(
        struct xvimage *f,
        int32_t connex,
        int32_t x,
        int32_t y,
        int32_t z)
/* supprime la composante connexe de f (image binaire)
   qui contient le point (x,y,z) */
/* connex : 4, 8 (en 2D), 6, 18, 26 (en 3D) */
/* connex : 60, 260 (en 3D): idem 6 et 26 mais dans le plan xy seulement */
/* ==================================== */
#undef F_NAME
#define F_NAME "ldeletecomp"
{
  index_t i;                       /* index muet de pixel */
  index_t j;                       /* index muet (generalement un voisin de x) */
  index_t k;                       /* index muet */
  index_t rs = rowsize(f);         /* taille ligne */
  index_t cs = colsize(f);         /* taille colonne */
  index_t ds = depth(f);           /* nb plans */
  index_t ps = rs * cs;            /* taille plan */
  index_t N = ps * ds;             /* taille image */
  uint8_t *F = UCHARDATA(f);
  Fifo * FIFO;
  int32_t incr_vois;

  switch (connex)
  {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
  } /* switch (connex) */

  FIFO = CreeFifoVide(N);
  if (FIFO == NULL)
  {   fprintf(stderr,"%s : CreeFifoVide failed\n", F_NAME);
      return(0);
  }

  i = z*ps + y*rs + x;
  if (F[i]) { FifoPush(FIFO, i); F[i] = 0; }

  if ((connex == 4) || (connex == 8))
  {
    if (ds != 1)
    {   fprintf(stderr,"%s : connexity 4 or 8 not defined for 3D\n", F_NAME);
        return(0);
    }
    while (! FifoVide(FIFO))
    {
      i = FifoPop(FIFO);
      for (k = 0; k < 8; k += incr_vois)
      {
        j = voisin(i, k, rs, N);
        if ((j != -1) && F[j]) { FifoPush(FIFO, j); F[j] = 0; }
      } /* for k */
    } /* while ! FifoVide */
  }
  else if (connex == 6)
  {
    if (ds == 1)
    {   fprintf(stderr,"%s : connexity 6 not defined for 2D\n", F_NAME);
        return(0);
    }
    while (! FifoVide(FIFO))
    {
      i = FifoPop(FIFO);
      for (k = 0; k <= 10; k += 2)
      {
        j = voisin6(i, k, rs, ps, N);
        if ((j != -1) && F[j]) { FifoPush(FIFO, j); F[j] = 0; }
      } /* for k */
    } /* while ! FifoVide */
  }
  else if (connex == 18)
  {
    if (ds == 1)
    {   fprintf(stderr,"%s : connexity 18 not defined for 2D\n", F_NAME);
        return(0);
    }
    while (! FifoVide(FIFO))
    {
      i = FifoPop(FIFO);
      for (k = 0; k < 18; k += 1)
      {
        j = voisin18(i, k, rs, ps, N);
        if ((j != -1) && F[j]) { FifoPush(FIFO, j); F[j] = 0; }
      } /* for k */
    } /* while ! FifoVide */
  }
  else if (connex == 26)
  {
    if (ds == 1)
    {   fprintf(stderr,"%s : connexity 26 not defined for 2D\n", F_NAME);
        return(0);
    }
    while (! FifoVide(FIFO))
    {
      i = FifoPop(FIFO);
      for (k = 0; k < 26; k += 1)
      {
        j = voisin26(i, k, rs, ps, N);
        if ((j != -1) && F[j]) { FifoPush(FIFO, j); F[j] = 0; }
      } /* for k */
    } /* while ! FifoVide */
  }
  else if (connex == 60)
  {
    if (ds == 1)
    {   fprintf(stderr,"%s : connexity 6 not defined for 2D\n", F_NAME);
        return(0);
    }
    while (! FifoVide(FIFO))
    {
      i = FifoPop(FIFO);
      for (k = 0; k <= 6; k += 2)
      {
        j = voisin6(i, k, rs, ps, N);
        if ((j != -1) && F[j]) { FifoPush(FIFO, j); F[j] = 0; }
      } /* for k */
    } /* while ! FifoVide */
  }
  else if (connex == 260)
  {
    if (ds == 1)
    {   fprintf(stderr,"%s : connexity 26 not defined for 2D\n", F_NAME);
        return(0);
    }
    while (! FifoVide(FIFO))
    {
      i = FifoPop(FIFO);
      for (k = 9; k <= 16; k += 1)
      {
        j = voisin26(i, k, rs, ps, N);
        if ((j != -1) && F[j]) { FifoPush(FIFO, j); F[j] = 0; }
      } /* for k */
    } /* while ! FifoVide */
  }

  FifoTermine(FIFO);
  return 1;
} /* ldeletecomp() */

/* ==================================== */
int32_t lselectcomp(
        struct xvimage *f,
        int32_t connex,
        int32_t x,
        int32_t y,
        int32_t z)
/* extrait la composante connexe de f (image binaire)
   qui contient le point (x,y,z) */
/* connex : 4, 8 (en 2D), 6, 18, 26 (en 3D) */
/* connex : 60, 260 (en 3D): idem 6 et 26 mais dans le plan xy seulement */
/* ==================================== */
#undef F_NAME
#define F_NAME "lselectcomp"
{
  index_t i;                       /* index muet de pixel */
  index_t j;                       /* index muet (generalement un voisin de x) */
  index_t k;                       /* index muet */
  index_t rs = rowsize(f);         /* taille ligne */
  index_t cs = colsize(f);         /* taille colonne */
  index_t ds = depth(f);           /* nb plans */
  index_t ps = rs * cs;            /* taille plan */
  index_t N = ps * ds;             /* taille image */
  uint8_t *F = UCHARDATA(f);
  Fifo * FIFO;
  int32_t incr_vois;

  switch (connex)
  {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
  } /* switch (connex) */

  FIFO = CreeFifoVide(N);
  if (FIFO == NULL)
  {   fprintf(stderr,"%s : CreeFifoVide failed\n", F_NAME);
      return(0);
  }

  for (i = 0; i < N; i++) if (F[i]) F[i] = 254;
  i = z*ps + y*rs + x;
  if (F[i]) { FifoPush(FIFO, i); F[i] += 1; }

  if ((connex == 4) || (connex == 8))
  {
    if (ds != 1)
    {   fprintf(stderr,"%s : connexity 4 or 8 not defined for 3D\n", F_NAME);
        return(0);
    }
    while (! FifoVide(FIFO))
    {
      i = FifoPop(FIFO);
      for (k = 0; k < 8; k += incr_vois)
      {
        j = voisin(i, k, rs, N);
        if ((j != -1) && (F[j]==254)) { FifoPush(FIFO, j); F[j] += 1; }
      } /* for k */
    } /* while ! FifoVide */
  }
  else if (connex == 6)
  {
    if (ds == 1)
    {   fprintf(stderr,"%s : connexity 6 not defined for 2D\n", F_NAME);
        return(0);
    }
    while (! FifoVide(FIFO))
    {
      i = FifoPop(FIFO);
      for (k = 0; k <= 10; k += 2)
      {
        j = voisin6(i, k, rs, ps, N);
        if ((j != -1) && (F[j]==254)) { FifoPush(FIFO, j); F[j] += 1; }
      } /* for k */
    } /* while ! FifoVide */
  }
  else if (connex == 18)
  {
    if (ds == 1)
    {   fprintf(stderr,"%s : connexity 18 not defined for 2D\n", F_NAME);
        return(0);
    }
    while (! FifoVide(FIFO))
    {
      i = FifoPop(FIFO);
      for (k = 0; k < 18; k += 1)
      {
        j = voisin18(i, k, rs, ps, N);
        if ((j != -1) && (F[j]==254)) { FifoPush(FIFO, j); F[j] += 1; }
      } /* for k */
    } /* while ! FifoVide */
  }
  else if (connex == 26)
  {
    if (ds == 1)
    {   fprintf(stderr,"%s : connexity 26 not defined for 2D\n", F_NAME);
        return(0);
    }
    while (! FifoVide(FIFO))
    {
      i = FifoPop(FIFO);
      for (k = 0; k < 26; k += 1)
      {
        j = voisin26(i, k, rs, ps, N);
        if ((j != -1) && (F[j]==254)) { FifoPush(FIFO, j); F[j] += 1; }
      } /* for k */
    } /* while ! FifoVide */
  }
  else if (connex == 60)
  {
    if (ds == 1)
    {   fprintf(stderr,"%s : connexity 6 not defined for 2D\n", F_NAME);
        return(0);
    }
    while (! FifoVide(FIFO))
    {
      i = FifoPop(FIFO);
      for (k = 0; k <= 6; k += 2)
      {
        j = voisin6(i, k, rs, ps, N);
        if ((j != -1) && (F[j]==254)) { FifoPush(FIFO, j); F[j] += 1; }
      } /* for k */
    } /* while ! FifoVide */
  }
  else if (connex == 260)
  {
    if (ds == 1)
    {   fprintf(stderr,"%s : connexity 26 not defined for 2D\n", F_NAME);
        return(0);
    }
    while (! FifoVide(FIFO))
    {
      i = FifoPop(FIFO);
      for (k = 9; k <= 16; k += 1)
      {
        j = voisin26(i, k, rs, ps, N);
        if ((j != -1) && (F[j]==254)) { FifoPush(FIFO, j); F[j] += 1; }
      } /* for k */
    } /* while ! FifoVide */
  }
  FifoTermine(FIFO);
  for (i = 0; i < N; i++) if (F[i] != 255) F[i] = 0;
  return 1;
} /* lselectcomp() */

/* ==================================== */
int32_t lgeodilat3d(
        struct xvimage *g,
        struct xvimage *f,
        int32_t connex,
        int32_t niter)
/* reconstruction de g sous f */
/* g : image marqueur */
/* f : image masque */
/* connex : 6 ou 18 ou 26 */
/* niter : nombre d'iterations (ou -1 pour saturation) */
/* resultat dans g */
/* ==================================== */
#undef F_NAME
#define F_NAME "lgeodilat3d"
{
  index_t nbchang, iter;
  index_t x;                       /* index muet de pixel */
  index_t y;                       /* index muet (generalement un voisin de x) */
  index_t k;                       /* index muet */
  index_t rs = rowsize(g);         /* taille ligne */
  index_t cs = colsize(g);         /* taille colonne */
  index_t d = depth(g);            /* nombre plans */
  index_t n = rs * cs;             /* taille plan */
  index_t N = n * d;               /* taille image */
  uint8_t *G = UCHARDATA(g);      /* l'image marqueur */
  uint8_t *F = UCHARDATA(f);      /* l'image masque */
  uint8_t *H;                     /* image de travail */
  uint8_t *temp;
  uint8_t sup;
  Fifo * FIFO[2];

  if ((rowsize(f) != rs) || (colsize(f) != cs) || (depth(f) != d))
  {
    fprintf(stderr, "%s: incompatible sizes\n", F_NAME);
    return 0;
  }

  FIFO[0] = CreeFifoVide(N);
  FIFO[1] = CreeFifoVide(N);
  if ((FIFO[0] == NULL) || (FIFO[1] == NULL))
  {   fprintf(stderr,"%s : CreeFifoVide failed\n", F_NAME);
      return(0);
  }

  IndicsInit(N);

  for (x = 0; x < N; x++)      /* mise en fifo initiale de tous les points */    {
    FifoPush(FIFO[1], x);
    Set(x, 1);
  }

  H = (uint8_t *)calloc(1,N*sizeof(char));
  if (H == NULL)
  {   fprintf(stderr,"%s : malloc failed for H\n", F_NAME);
      return(0);
  }

  for (x = 0; x < N; x++)      /* force G à être <= F */
    if (G[x] > F[x]) G[x] = F[x];

  if (connex == 26)
  {
    iter = 0;
    do
    {
      iter += 1;
      nbchang = 0;
      while (! FifoVide(FIFO[iter % 2]))
      {
        x = FifoPop(FIFO[iter % 2]);
        UnSet(x, iter % 2);
        sup = G[x];
        for (k = 0; k < 26; k += 1)
        {
          y = voisin26(x, k, rs, n, N);
          if ((y != -1) && (G[y] > sup)) sup = G[y];
        } /* for k */

        sup = mcmin(sup, F[x]);
        if (G[x] != sup)
        {  /* changement: on enregistre x ainsi que ses voisins */
          nbchang += 1;
          if (! IsSet(x, (iter + 1) % 2))
          {
            FifoPush(FIFO[(iter + 1) % 2], x);
            Set(x, (iter + 1) % 2);
  	  }
          for (k = 0; k < 26; k += 1)
          {
            y = voisin26(x, k, rs, n, N);
            if ((y != -1) && (! IsSet(y, (iter + 1) % 2)))
            {
              FifoPush(FIFO[(iter + 1) % 2], y);
              Set(y, (iter + 1) % 2);
            }
          } /* for k */
        }
        H[x] = sup;
      } /* while ! FifoVide */
      temp = G; /* echange les roles de G et H */
      G = H;
      H = temp;
#ifdef VERBOSE
      printf("iteration %d, nbchang %d\n", iter, nbchang);
#endif
    } while (((niter == -1) || (iter < niter)) && (nbchang != 0));
  }
  else if (connex == 18)
  {
    iter = 0;
    do
    {
      iter += 1;
      nbchang = 0;
      while (! FifoVide(FIFO[iter % 2]))
      {
        x = FifoPop(FIFO[iter % 2]);
        UnSet(x, iter % 2);
        sup = G[x];
        for (k = 0; k < 18; k += 1)
        {
          y = voisin18(x, k, rs, n, N);
          if ((y != -1) && (G[y] > sup)) sup = G[y];
        } /* for k */

        sup = mcmin(sup, F[x]);
        if (G[x] != sup)
        {  /* changement: on enregistre x ainsi que ses voisins */
          nbchang += 1;
          if (! IsSet(x, (iter + 1) % 2))
          {
            FifoPush(FIFO[(iter + 1) % 2], x);
            Set(x, (iter + 1) % 2);
  	  }
          for (k = 0; k < 18; k += 1)
          {
            y = voisin18(x, k, rs, n, N);
            if ((y != -1) && (! IsSet(y, (iter + 1) % 2)))
            {
              FifoPush(FIFO[(iter + 1) % 2], y);
              Set(y, (iter + 1) % 2);
            }
          } /* for k */
        }
        H[x] = sup;
      } /* while ! FifoVide */
      temp = G; /* echange les roles de G et H */
      G = H;
      H = temp;
#ifdef VERBOSE
      printf("iteration %d, nbchang %d\n", iter, nbchang);
#endif
    } while (((niter == -1) || (iter < niter)) && (nbchang != 0));
  }
  else if (connex == 6)
  {
    iter = 0;
    do
    {
      iter += 1;
      nbchang = 0;
      while (! FifoVide(FIFO[iter % 2]))
      {
        x = FifoPop(FIFO[iter % 2]);
        UnSet(x, iter % 2);
        sup = G[x];
        for (k = 0; k <= 10; k += 2)
        {
          y = voisin6(x, k, rs, n, N);
          if ((y != -1) && (G[y] > sup)) sup = G[y];
        } /* for k */

        sup = mcmin(sup, F[x]);
        if (G[x] != sup)
        {  /* changement: on enregistre x ainsi que ses voisins */
          nbchang += 1;
          if (! IsSet(x, (iter + 1) % 2))
          {
            FifoPush(FIFO[(iter + 1) % 2], x);
            Set(x, (iter + 1) % 2);
  	  }
          for (k = 0; k <= 10; k += 2)
          {
            y = voisin6(x, k, rs, n, N);
            if ((y != -1) && (! IsSet(y, (iter + 1) % 2)))
            {
              FifoPush(FIFO[(iter + 1) % 2], y);
              Set(y, (iter + 1) % 2);
            }
          } /* for k */
        }
        H[x] = sup;
      } /* while ! FifoVide */
      temp = G; /* echange les roles de G et H */
      G = H;
      H = temp;
#ifdef VERBOSE
      printf("iteration %d, nbchang %d\n", iter, nbchang);
#endif
    } while (((niter == -1) || (iter < niter)) && (nbchang != 0));
  }
  else
  {
    fprintf(stderr, "%s: bad connexity\n", F_NAME);
    return 0;
  }

  /* remet le resultat dans g si necessaire */
  if (G != UCHARDATA(g))
  {
    for (x = 0; x < N; x++)
      (UCHARDATA(g))[x] = G[x];
    free(G);
  }
  else
    free(H);

  FifoTermine(FIFO[0]);
  FifoTermine(FIFO[1]);
  IndicsTermine();
  return 1;
} // lgeodilat3d(

/* ==================================== */
int32_t lgeodilat3d_short(
        struct xvimage *g,
        struct xvimage *f,
        int32_t connex,
        int32_t niter)
/* reconstruction de g sous f */
/* g : image marqueur */
/* f : image masque */
/* connex : 6 ou 18 ou 26 */
/* niter : nombre d'iterations (ou -1 pour saturation) */
/* resultat dans g */
/* ==================================== */
#undef F_NAME
#define F_NAME "lgeodilat3d_short"
{
  index_t nbchang, iter;
  index_t x;                       /* index muet de pixel */
  index_t y;                       /* index muet (generalement un voisin de x) */
  index_t k;                       /* index muet */
  index_t rs = rowsize(g);         /* taille ligne */
  index_t cs = colsize(g);         /* taille colonne */
  index_t d = depth(g);            /* nombre plans */
  index_t n = rs * cs;             /* taille plan */
  index_t N = n * d;               /* taille image */
  int16_t *G = SSHORTDATA(g);      /* l'image marqueur */
  int16_t *F = SSHORTDATA(f);      /* l'image masque */
  int16_t *H;                      /* image de travail */
  int16_t *temp;
  int16_t sup;
  Fifo * FIFO[2];

  if ((rowsize(f) != rs) || (colsize(f) != cs) || (depth(f) != d))
  {
    fprintf(stderr, "%s: incompatible sizes\n", F_NAME);
    return 0;
  }

  FIFO[0] = CreeFifoVide(N);
  FIFO[1] = CreeFifoVide(N);
  if ((FIFO[0] == NULL) || (FIFO[1] == NULL))
  {   fprintf(stderr,"%s : CreeFifoVide failed\n", F_NAME);
      return(0);
  }

  IndicsInit(N);

  for (x = 0; x < N; x++)      /* mise en fifo initiale de tous les points */    {
    FifoPush(FIFO[1], x);
    Set(x, 1);
  }

  H = (int16_t *)calloc(1,N*sizeof(int16_t));
  if (H == NULL)
  {   fprintf(stderr,"%s : malloc failed for H\n", F_NAME);
      return(0);
  }

  for (x = 0; x < N; x++)      /* force G à être <= F */
    if (G[x] > F[x]) G[x] = F[x];

  if (connex == 26)
  {
    iter = 0;
    do
    {
      iter += 1;
      nbchang = 0;
      while (! FifoVide(FIFO[iter % 2]))
      {
        x = FifoPop(FIFO[iter % 2]);
        UnSet(x, iter % 2);
        sup = G[x];
        for (k = 0; k < 26; k += 1)
        {
          y = voisin26(x, k, rs, n, N);
          if ((y != -1) && (G[y] > sup)) sup = G[y];
        } /* for k */

        sup = mcmin(sup, F[x]);
        if (G[x] != sup)
        {  /* changement: on enregistre x ainsi que ses voisins */
          nbchang += 1;
          if (! IsSet(x, (iter + 1) % 2))
          {
            FifoPush(FIFO[(iter + 1) % 2], x);
            Set(x, (iter + 1) % 2);
  	  }
          for (k = 0; k < 26; k += 1)
          {
            y = voisin26(x, k, rs, n, N);
            if ((y != -1) && (! IsSet(y, (iter + 1) % 2)))
            {
              FifoPush(FIFO[(iter + 1) % 2], y);
              Set(y, (iter + 1) % 2);
            }
          } /* for k */
        }
        H[x] = sup;
      } /* while ! FifoVide */
      temp = G; /* echange les roles de G et H */
      G = H;
      H = temp;
#ifdef VERBOSE
      printf("iteration %d, nbchang %d\n", iter, nbchang);
#endif
    } while (((niter == -1) || (iter < niter)) && (nbchang != 0));
  }
  else if (connex == 18)
  {
    iter = 0;
    do
    {
      iter += 1;
      nbchang = 0;
      while (! FifoVide(FIFO[iter % 2]))
      {
        x = FifoPop(FIFO[iter % 2]);
        UnSet(x, iter % 2);
        sup = G[x];
        for (k = 0; k < 18; k += 1)
        {
          y = voisin18(x, k, rs, n, N);
          if ((y != -1) && (G[y] > sup)) sup = G[y];
        } /* for k */

        sup = mcmin(sup, F[x]);
        if (G[x] != sup)
        {  /* changement: on enregistre x ainsi que ses voisins */
          nbchang += 1;
          if (! IsSet(x, (iter + 1) % 2))
          {
            FifoPush(FIFO[(iter + 1) % 2], x);
            Set(x, (iter + 1) % 2);
  	  }
          for (k = 0; k < 18; k += 1)
          {
            y = voisin18(x, k, rs, n, N);
            if ((y != -1) && (! IsSet(y, (iter + 1) % 2)))
            {
              FifoPush(FIFO[(iter + 1) % 2], y);
              Set(y, (iter + 1) % 2);
            }
          } /* for k */
        }
        H[x] = sup;
      } /* while ! FifoVide */
      temp = G; /* echange les roles de G et H */
      G = H;
      H = temp;
#ifdef VERBOSE
      printf("iteration %d, nbchang %d\n", iter, nbchang);
#endif
    } while (((niter == -1) || (iter < niter)) && (nbchang != 0));
  }
  else if (connex == 6)
  {
    iter = 0;
    do
    {
      iter += 1;
      nbchang = 0;
      while (! FifoVide(FIFO[iter % 2]))
      {
        x = FifoPop(FIFO[iter % 2]);
        UnSet(x, iter % 2);
        sup = G[x];
        for (k = 0; k <= 10; k += 2)
        {
          y = voisin6(x, k, rs, n, N);
          if ((y != -1) && (G[y] > sup)) sup = G[y];
        } /* for k */

        sup = mcmin(sup, F[x]);
        if (G[x] != sup)
        {  /* changement: on enregistre x ainsi que ses voisins */
          nbchang += 1;
          if (! IsSet(x, (iter + 1) % 2))
          {
            FifoPush(FIFO[(iter + 1) % 2], x);
            Set(x, (iter + 1) % 2);
  	  }
          for (k = 0; k <= 10; k += 2)
          {
            y = voisin6(x, k, rs, n, N);
            if ((y != -1) && (! IsSet(y, (iter + 1) % 2)))
            {
              FifoPush(FIFO[(iter + 1) % 2], y);
              Set(y, (iter + 1) % 2);
            }
          } /* for k */
        }
        H[x] = sup;
      } /* while ! FifoVide */
      temp = G; /* echange les roles de G et H */
      G = H;
      H = temp;
#ifdef VERBOSE
      printf("iteration %d, nbchang %d\n", iter, nbchang);
#endif
    } while (((niter == -1) || (iter < niter)) && (nbchang != 0));
  }
  else
  {
    fprintf(stderr, "%s: bad connexity\n", F_NAME);
    return 0;
  }

  /* remet le resultat dans g si necessaire */
  if (G != SSHORTDATA(g))
  {
    for (x = 0; x < N; x++)
      (SSHORTDATA(g))[x] = G[x];
    free(G);
  }
  else
    free(H);

  FifoTermine(FIFO[0]);
  FifoTermine(FIFO[1]);
  IndicsTermine();
  return 1;
} // lgeodilat3d_short(

/* ==================================== */
int32_t lgeodilat3d_long(
        struct xvimage *g,
        struct xvimage *f,
        int32_t connex,
        int32_t niter)
/* reconstruction de g sous f */
/* g : image marqueur */
/* f : image masque */
/* connex : 6 ou 18 ou 26 */
/* niter : nombre d'iterations (ou -1 pour saturation) */
/* resultat dans g */
/* ==================================== */
#undef F_NAME
#define F_NAME "lgeodilat3d_long"
{
  index_t nbchang, iter;
  index_t x;                       /* index muet de pixel */
  index_t y;                       /* index muet (generalement un voisin de x) */
  index_t k;                       /* index muet */
  index_t rs = rowsize(g);         /* taille ligne */
  index_t cs = colsize(g);         /* taille colonne */
  index_t d = depth(g);            /* nombre plans */
  index_t n = rs * cs;             /* taille plan */
  index_t N = n * d;               /* taille image */
  int32_t *G = SLONGDATA(g);      /* l'image marqueur */
  int32_t *F = SLONGDATA(f);      /* l'image masque */
  int32_t *H;                     /* image de travail */
  int32_t *temp;
  int32_t sup;
  Fifo * FIFO[2];

  if ((rowsize(f) != rs) || (colsize(f) != cs) || (depth(f) != d))
  {
    fprintf(stderr, "%s: incompatible sizes\n", F_NAME);
    return 0;
  }

  FIFO[0] = CreeFifoVide(N);
  FIFO[1] = CreeFifoVide(N);
  if ((FIFO[0] == NULL) || (FIFO[1] == NULL))
  {   fprintf(stderr,"%s : CreeFifoVide failed\n", F_NAME);
      return(0);
  }

  IndicsInit(N);

  for (x = 0; x < N; x++)      /* mise en fifo initiale de tous les points */    {
    FifoPush(FIFO[1], x);
    Set(x, 1);
  }

  H = (int32_t *)calloc(1,N*sizeof(int32_t));
  if (H == NULL)
  {   fprintf(stderr,"%s : malloc failed for H\n", F_NAME);
      return(0);
  }

  for (x = 0; x < N; x++)      /* force G à être <= F */
    if (G[x] > F[x]) G[x] = F[x];

  if (connex == 26)
  {
    iter = 0;
    do
    {
      iter += 1;
      nbchang = 0;
      while (! FifoVide(FIFO[iter % 2]))
      {
        x = FifoPop(FIFO[iter % 2]);
        UnSet(x, iter % 2);
        sup = G[x];
        for (k = 0; k < 26; k += 1)
        {
          y = voisin26(x, k, rs, n, N);
          if ((y != -1) && (G[y] > sup)) sup = G[y];
        } /* for k */

        sup = mcmin(sup, F[x]);
        if (G[x] != sup)
        {  /* changement: on enregistre x ainsi que ses voisins */
          nbchang += 1;
          if (! IsSet(x, (iter + 1) % 2))
          {
            FifoPush(FIFO[(iter + 1) % 2], x);
            Set(x, (iter + 1) % 2);
  	  }
          for (k = 0; k < 26; k += 1)
          {
            y = voisin26(x, k, rs, n, N);
            if ((y != -1) && (! IsSet(y, (iter + 1) % 2)))
            {
              FifoPush(FIFO[(iter + 1) % 2], y);
              Set(y, (iter + 1) % 2);
            }
          } /* for k */
        }
        H[x] = sup;
      } /* while ! FifoVide */
      temp = G; /* echange les roles de G et H */
      G = H;
      H = temp;
#ifdef VERBOSE
      printf("iteration %d, nbchang %d\n", iter, nbchang);
#endif
    } while (((niter == -1) || (iter < niter)) && (nbchang != 0));
  }
  else if (connex == 18)
  {
    iter = 0;
    do
    {
      iter += 1;
      nbchang = 0;
      while (! FifoVide(FIFO[iter % 2]))
      {
        x = FifoPop(FIFO[iter % 2]);
        UnSet(x, iter % 2);
        sup = G[x];
        for (k = 0; k < 18; k += 1)
        {
          y = voisin18(x, k, rs, n, N);
          if ((y != -1) && (G[y] > sup)) sup = G[y];
        } /* for k */

        sup = mcmin(sup, F[x]);
        if (G[x] != sup)
        {  /* changement: on enregistre x ainsi que ses voisins */
          nbchang += 1;
          if (! IsSet(x, (iter + 1) % 2))
          {
            FifoPush(FIFO[(iter + 1) % 2], x);
            Set(x, (iter + 1) % 2);
  	  }
          for (k = 0; k < 18; k += 1)
          {
            y = voisin18(x, k, rs, n, N);
            if ((y != -1) && (! IsSet(y, (iter + 1) % 2)))
            {
              FifoPush(FIFO[(iter + 1) % 2], y);
              Set(y, (iter + 1) % 2);
            }
          } /* for k */
        }
        H[x] = sup;
      } /* while ! FifoVide */
      temp = G; /* echange les roles de G et H */
      G = H;
      H = temp;
#ifdef VERBOSE
      printf("iteration %d, nbchang %d\n", iter, nbchang);
#endif
    } while (((niter == -1) || (iter < niter)) && (nbchang != 0));
  }
  else if (connex == 6)
  {
    iter = 0;
    do
    {
      iter += 1;
      nbchang = 0;
      while (! FifoVide(FIFO[iter % 2]))
      {
        x = FifoPop(FIFO[iter % 2]);
        UnSet(x, iter % 2);
        sup = G[x];
        for (k = 0; k <= 10; k += 2)
        {
          y = voisin6(x, k, rs, n, N);
          if ((y != -1) && (G[y] > sup)) sup = G[y];
        } /* for k */

        sup = mcmin(sup, F[x]);
        if (G[x] != sup)
        {  /* changement: on enregistre x ainsi que ses voisins */
          nbchang += 1;
          if (! IsSet(x, (iter + 1) % 2))
          {
            FifoPush(FIFO[(iter + 1) % 2], x);
            Set(x, (iter + 1) % 2);
  	  }
          for (k = 0; k <= 10; k += 2)
          {
            y = voisin6(x, k, rs, n, N);
            if ((y != -1) && (! IsSet(y, (iter + 1) % 2)))
            {
              FifoPush(FIFO[(iter + 1) % 2], y);
              Set(y, (iter + 1) % 2);
            }
          } /* for k */
        }
        H[x] = sup;
      } /* while ! FifoVide */
      temp = G; /* echange les roles de G et H */
      G = H;
      H = temp;
#ifdef VERBOSE
      printf("iteration %d, nbchang %d\n", iter, nbchang);
#endif
    } while (((niter == -1) || (iter < niter)) && (nbchang != 0));
  }
  else
  {
    fprintf(stderr, "%s: bad connexity\n", F_NAME);
    return 0;
  }

  /* remet le resultat dans g si necessaire */
  if (G != SLONGDATA(g))
  {
    for (x = 0; x < N; x++)
      (SLONGDATA(g))[x] = G[x];
    free(G);
  }
  else
    free(H);

  FifoTermine(FIFO[0]);
  FifoTermine(FIFO[1]);
  IndicsTermine();
  return 1;
} // lgeodilat3d_long(

/* ==================================== */
int32_t lreconsdilat3d(
        struct xvimage *g,
        struct xvimage *f,
        int32_t connex)
/* reconstruction de g sous f */
/* g : image marqueur */
/* f : image masque */
/* resultat dans g */
/* ==================================== */
{
  return lgeodilat3d(g, f, connex, -1);
}

/* ==================================== */
int32_t lgeoeros3d(
        struct xvimage *g,
        struct xvimage *f,
        int32_t connex,
        int32_t niter)
/* reconstruction de g sous f */
/* g : image marqueur */
/* f : image masque */
/* connex : 6 ou 18 ou 26 */
/* niter : nombre d'iterations (ou -1 pour saturation) */
/* resultat dans g */
/* ==================================== */
#undef F_NAME
#define F_NAME "lgeoeros3d"
{
  index_t nbchang, iter;
  index_t x;                       /* index muet de pixel */
  index_t y;                       /* index muet (generalement un voisin de x) */
  index_t k;                       /* index muet */
  index_t rs = rowsize(g);         /* taille ligne */
  index_t cs = colsize(g);         /* taille colonne */
  index_t d = depth(g);            /* nombre plans */
  index_t n = rs * cs;             /* taille plan */
  index_t N = n * d;               /* taille image */
  uint8_t *G = UCHARDATA(g);      /* l'image marqueur */
  uint8_t *F = UCHARDATA(f);      /* l'image masque */
  uint8_t *H;                     /* image de travail */
  uint8_t *temp;
  uint8_t inf;
  Fifo * FIFO[2];

  if ((rowsize(f) != rs) || (colsize(f) != cs) || (depth(f) != d))
  {
    fprintf(stderr, "%s: incompatible sizes\n", F_NAME);
    return 0;
  }

  FIFO[0] = CreeFifoVide(N);
  FIFO[1] = CreeFifoVide(N);
  if ((FIFO[0] == NULL) || (FIFO[1] == NULL))
  {   fprintf(stderr,"%s : CreeFifoVide failed\n", F_NAME);
      return(0);
  }

  IndicsInit(N);

  for (x = 0; x < N; x++)      /* mise en fifo initiale de tous les points */    {
    FifoPush(FIFO[1], x);
    Set(x, 1);
  }

  H = (uint8_t *)calloc(1,N*sizeof(char));
  if (H == NULL)
  {   fprintf(stderr,"%s : malloc failed for H\n", F_NAME);
      return(0);
  }

  for (x = 0; x < N; x++)      /* force G à être >= F */
    if (G[x] < F[x]) G[x] = F[x];

  if (connex == 26)
  {
    iter = 0;
    do
    {
      iter += 1;
      nbchang = 0;
      while (! FifoVide(FIFO[iter % 2]))
      {
        x = FifoPop(FIFO[iter % 2]);
        UnSet(x, iter % 2);
        inf = G[x];
        for (k = 0; k < 26; k += 1)
        {
          y = voisin26(x, k, rs, n, N);
          if ((y != -1) && (G[y] < inf)) inf = G[y];
        } /* for k */

        inf = mcmax(inf, F[x]);
        if (G[x] != inf)
        {  /* changement: on enregistre x ainsi que ses voisins */
          nbchang += 1;
          if (! IsSet(x, (iter + 1) % 2))
          {
            FifoPush(FIFO[(iter + 1) % 2], x);
            Set(x, (iter + 1) % 2);
  	  }
          for (k = 0; k < 26; k += 1)
          {
            y = voisin26(x, k, rs, n, N);
            if ((y != -1) && (! IsSet(y, (iter + 1) % 2)))
            {
              FifoPush(FIFO[(iter + 1) % 2], y);
              Set(y, (iter + 1) % 2);
            }
          } /* for k */
        }
        H[x] = inf;
      } /* while ! FifoVide */
      temp = G; /* echange les roles de G et H */
      G = H;
      H = temp;
#ifdef VERBOSE
      printf("iteration %d, nbchang %d\n", iter, nbchang);
#endif
    } while (((niter == -1) || (iter < niter)) && (nbchang != 0));
  }
  else if (connex == 18)
  {
    iter = 0;
    do
    {
      iter += 1;
      nbchang = 0;
      while (! FifoVide(FIFO[iter % 2]))
      {
        x = FifoPop(FIFO[iter % 2]);
        UnSet(x, iter % 2);
        inf = G[x];
        for (k = 0; k < 18; k += 1)
        {
          y = voisin18(x, k, rs, n, N);
          if ((y != -1) && (G[y] < inf)) inf = G[y];
        } /* for k */

        inf = mcmax(inf, F[x]);
        if (G[x] != inf)
        {  /* changement: on enregistre x ainsi que ses voisins */
          nbchang += 1;
          if (! IsSet(x, (iter + 1) % 2))
          {
            FifoPush(FIFO[(iter + 1) % 2], x);
            Set(x, (iter + 1) % 2);
  	  }
          for (k = 0; k < 18; k += 1)
          {
            y = voisin18(x, k, rs, n, N);
            if ((y != -1) && (! IsSet(y, (iter + 1) % 2)))
            {
              FifoPush(FIFO[(iter + 1) % 2], y);
              Set(y, (iter + 1) % 2);
            }
          } /* for k */
        }
        H[x] = inf;
      } /* while ! FifoVide */
      temp = G; /* echange les roles de G et H */
      G = H;
      H = temp;
#ifdef VERBOSE
      printf("iteration %d, nbchang %d\n", iter, nbchang);
#endif
    } while (((niter == -1) || (iter < niter)) && (nbchang != 0));
  }
  else if (connex == 6)
  {
    iter = 0;
    do
    {
      iter += 1;
      nbchang = 0;
      while (! FifoVide(FIFO[iter % 2]))
      {
        x = FifoPop(FIFO[iter % 2]);
        UnSet(x, iter % 2);
        inf = G[x];
        for (k = 0; k <= 10; k += 2)
        {
          y = voisin6(x, k, rs, n, N);
          if ((y != -1) && (G[y] < inf)) inf = G[y];
        } /* for k */

        inf = mcmax(inf, F[x]);
        if (G[x] != inf)
        {  /* changement: on enregistre x ainsi que ses voisins */
          nbchang += 1;
          if (! IsSet(x, (iter + 1) % 2))
          {
            FifoPush(FIFO[(iter + 1) % 2], x);
            Set(x, (iter + 1) % 2);
  	  }
          for (k = 0; k <= 10; k += 2)
          {
            y = voisin6(x, k, rs, n, N);
            if ((y != -1) && (! IsSet(y, (iter + 1) % 2)))
            {
              FifoPush(FIFO[(iter + 1) % 2], y);
              Set(y, (iter + 1) % 2);
            }
          } /* for k */
        }
        H[x] = inf;
      } /* while ! FifoVide */
      temp = G; /* echange les roles de G et H */
      G = H;
      H = temp;
#ifdef VERBOSE
      printf("iteration %d, nbchang %d\n", iter, nbchang);
#endif
    } while (((niter == -1) || (iter < niter)) && (nbchang != 0));
  }
  else
  {
    fprintf(stderr, "%s: bad connexity\n", F_NAME);
    return 0;
  }

  /* remet le resultat dans g si necessaire */
  if (G != UCHARDATA(g))
  {
    for (x = 0; x < N; x++)
      (UCHARDATA(g))[x] = G[x];
    free(G);
  }
  else
    free(H);

  FifoTermine(FIFO[0]);
  FifoTermine(FIFO[1]);
  IndicsTermine();
  return 1;
} // lgeoeros3d(

/* ==================================== */
int32_t lreconseros3d(
        struct xvimage *g,
        struct xvimage *f,
        int32_t connex)
/* reconstruction de g sous f */
/* g : image marqueur */
/* f : image masque */
/* resultat dans g */
/* ==================================== */
{
  return lgeoeros3d(g, f, connex, -1);
}

/* ==================================== */
int32_t lamont(
        struct xvimage *m,
        struct xvimage *f,
        int32_t connex,
        int32_t strict)
/* connex : 4, 8 (en 2D), 6, 18, 26 (en 3D) */
/* ==================================== */
#undef F_NAME
#define F_NAME "lamont"
{
  index_t i, j, k;                 /* index muet de pixel */
  index_t rs = rowsize(f);         /* taille ligne */
  index_t cs = colsize(f);         /* taille colonne */
  index_t ds = depth(f);           /* nb plans */
  index_t ps = rs * cs;            /* taille plan */
  index_t N = ps * ds;             /* taille image */
  int32_t *F = SLONGDATA(f);
  uint8_t *M = UCHARDATA(m);
  Fifo * FIFO;
  int32_t incr_vois;

  if ((rowsize(m) != rs) || (colsize(m) != cs) || (depth(m) != ds))
  {
    fprintf(stderr, "%s: incompatible sizes\n", F_NAME);
    return 0;
  }

  if ((datatype(m) != VFF_TYP_1_BYTE) || (datatype(f) != VFF_TYP_4_BYTE))
  {
    fprintf(stderr, "%s: incompatible types\n", F_NAME);
    return 0;
  }

  switch (connex)
  {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
  } /* switch (connex) */

  FIFO = CreeFifoVide(N);
  if (FIFO == NULL)
  {   fprintf(stderr,"%s : CreeFifoVide failed\n", F_NAME);
      return(0);
  }

  for (i = 0; i < N; i++) if (M[i]) FifoPush(FIFO, i);

  if ((connex == 4) || (connex == 8))
  {
    if (ds != 1)
    {
      fprintf(stderr,"%s : connexity 4 or 8 not defined for 3D\n", F_NAME);
      return(0);
    }
    if (strict)
    while (! FifoVide(FIFO))
    {
      i = FifoPop(FIFO);
      for (k = 0; k < 8; k += incr_vois)
      {
        j = voisin(i, k, rs, N);
        if ((j != -1) && !M[j] && (F[j] > F[i])) { FifoPush(FIFO, j); M[j] = 255; }
      } /* for k */
    } /* while ! FifoVide */
    else
    while (! FifoVide(FIFO))
    {
      i = FifoPop(FIFO);
      for (k = 0; k < 8; k += incr_vois)
      {
        j = voisin(i, k, rs, N);
//printf("i=%d ; j=%d ; Mj=%d ; Fj=%ld ; Fi=%ld\n", i, j, M[j], F[j], F[i]);
        if ((j != -1) && !M[j] && (F[j] >= F[i])) { FifoPush(FIFO, j); M[j] = 255; }
      } /* for k */
    } /* while ! FifoVide */
  }
  else if (connex == 6)
  {
    if (ds == 1)
    {   fprintf(stderr,"%s : connexity 6 not defined for 2D\n", F_NAME);
        return(0);
    }
    if (strict)
    while (! FifoVide(FIFO))
    {
      i = FifoPop(FIFO);
      for (k = 0; k <= 10; k += 2)
      {
        j = voisin6(i, k, rs, ps, N);
        if ((j != -1) && !M[j] && (F[j] > F[i])) { FifoPush(FIFO, j); M[j] = 255; }
      } /* for k */
    } /* while ! FifoVide */
    else
    while (! FifoVide(FIFO))
    {
      i = FifoPop(FIFO);
      for (k = 0; k <= 10; k += 2)
      {
        j = voisin6(i, k, rs, ps, N);
        if ((j != -1) && !M[j] && (F[j] >= F[i])) { FifoPush(FIFO, j); M[j] = 255; }
      } /* for k */
    } /* while ! FifoVide */
  }
  else if (connex == 18)
  {
    if (ds == 1)
    {   fprintf(stderr,"%s : connexity 18 not defined for 2D\n", F_NAME);
        return(0);
    }
    if (strict)
    while (! FifoVide(FIFO))
    {
      i = FifoPop(FIFO);
      for (k = 0; k < 18; k += 1)
      {
        j = voisin18(i, k, rs, ps, N);
        if ((j != -1) && !M[j] && (F[j] > F[i])) { FifoPush(FIFO, j); M[j] = 255; }

      } /* for k */
    } /* while ! FifoVide */
    else
    while (! FifoVide(FIFO))
    {
      i = FifoPop(FIFO);
      for (k = 0; k < 18; k += 1)
      {
        j = voisin18(i, k, rs, ps, N);
        if ((j != -1) && !M[j] && (F[j] >= F[i])) { FifoPush(FIFO, j); M[j] = 255; }

      } /* for k */
    } /* while ! FifoVide */
  }
  else if (connex == 26)
  {
    if (ds == 1)
    {   fprintf(stderr,"%s : connexity 26 not defined for 2D\n", F_NAME);
        return(0);
    }
    if (strict)
    while (! FifoVide(FIFO))
    {
      i = FifoPop(FIFO);
      for (k = 0; k < 26; k += 1)
      {
        j = voisin26(i, k, rs, ps, N);
        if ((j != -1) && !M[j] && (F[j] > F[i])) { FifoPush(FIFO, j); M[j] = 255; }
      } /* for k */
    } /* while ! FifoVide */
    else
    while (! FifoVide(FIFO))
    {
      i = FifoPop(FIFO);
      for (k = 0; k < 26; k += 1)
      {
        j = voisin26(i, k, rs, ps, N);
        if ((j != -1) && !M[j] && (F[j] >= F[i])) { FifoPush(FIFO, j); M[j] = 255; }
      } /* for k */
    } /* while ! FifoVide */
  }
  FifoTermine(FIFO);
  return 1;
} /* lamont() */
