/*
 * 
 *                This source code is part of
 * 
 *                 G   R   O   M   A   C   S
 * 
 *          GROningen MAchine for Chemical Simulations
 * 
 *                        VERSION 3.2.0
 * Written by David van der Spoel, Erik Lindahl, Berk Hess, and others.
 * Copyright (c) 1991-2000, University of Groningen, The Netherlands.
 * Copyright (c) 2001-2004, The GROMACS development team,
 * check out http://www.gromacs.org for more information.

 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * If you want to redistribute modifications, please consider that
 * scientific software is very special. Version control is crucial -
 * bugs must be traceable. We will be happy to consider code for
 * inclusion in the official distribution, but derived work must not
 * be called official GROMACS. Details are found in the README & COPYING
 * files - if they are missing, get the official version at www.gromacs.org.
 * 
 * To help us fund GROMACS development, we humbly ask that you cite
 * the papers on the package - you can find them in the top README file.
 * 
 * For more info, check our website at http://www.gromacs.org
 * 
 * And Hey:
 * Gromacs Runs On Most of All Computer Systems
 */

#ifndef _pme_h
#define _pme_h

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include "typedefs.h"
#include "gmxcomplex.h"
#include "fftgrid.h"
#include "gmx_wallcycle.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef real *splinevec[DIM];

enum { GMX_SUM_QGRID_FORWARD, GMX_SUM_QGRID_BACKWARD };

extern int pme_inconvenient_nnodes(int nkx,int nky,int nnodes);
/* Checks for FFT + solve_pme load imbalance, returns:
 * 0 when no or negligible load imbalance is expected
 * 1 when a slight load imbalance is expected
 * 2 when using less PME nodes is expected to be faster
 */

extern int gmx_pme_init(gmx_pme_t *pmedata,t_commrec *cr,
			int nnodes_major,int nnodes_minor,
			t_inputrec *ir,int homenr,
			bool bFreeEnergy, bool bReproducible);
			
extern int gmx_pme_destroy(FILE *log,gmx_pme_t *pmedata);
/* Initialize and destroy the pme data structures resepectively.
 * Return value 0 indicates all well, non zero is an error code.
 */

#define GMX_PME_SPREAD_Q      (1<<0)
#define GMX_PME_SOLVE         (1<<1)
#define GMX_PME_CALC_F        (1<<2)
#define GMX_PME_DO_ALL  (GMX_PME_SPREAD_Q | GMX_PME_SOLVE | GMX_PME_CALC_F)

extern int gmx_pme_do(gmx_pme_t pme,
		      int start,       int homenr,
		      rvec x[],        rvec f[],
		      real chargeA[],  real chargeB[],
		      matrix box,      t_commrec *cr,
		      int  maxshift0,  int maxshift1,
		      t_nrnb *nrnb,
		      matrix lrvir,    real ewaldcoeff,
		      real *energy,    real lambda,    
		      real *dvdlambda, int flags);
/* Do a PME calculation for the long range electrostatics. 
 * flags, defined above, determine which parts of the calculation are performed.
 * Return value 0 indicates all well, non zero is an error code.
 */

extern int gmx_pmeonly(gmx_pme_t pme,
                       t_commrec *cr,     t_nrnb *mynrnb,
		       gmx_wallcycle_t wcycle,
		       real ewaldcoeff,   bool bGatherOnly,
		       t_inputrec *ir);
/* Called on the nodes that do PME exclusively (as slaves) 
 */

extern void gmx_sum_qgrid(gmx_pme_t pme,t_commrec *cr,t_fftgrid *grid,
			  int direction);

extern void gmx_pme_calc_energy(gmx_pme_t pme,int n,rvec *x,real *q,real *V);
/* Calculate the PME grid energy V for n charges with a potential
 * in the pme struct determined before with a call to gmx_pme_do
 * with at least GMX_PME_SPREAD_Q and GMX_PME_SOLVE specified.
 * Note that the charges are not spread on the grid in the pme struct.
 * Currently does not work in parallel or with free energy.
 */

/* The following three routines are for PME/PP node splitting in pme_pp.c */

/* Abstract type for PME <-> PP communication */
typedef struct gmx_pme_pp *gmx_pme_pp_t;

extern gmx_pme_pp_t gmx_pme_pp_init(t_commrec *cr);
/* Initialize the PME-only side of the PME <-> PP communication */

extern void gmx_pme_send_q(t_commrec *cr,
			   bool bFreeEnergy, real *chargeA, real *chargeB,
			   int maxshift0, int maxshift1);
/* Send the charges and maxshift to out PME-only node. */

extern void gmx_pme_send_x(t_commrec *cr, matrix box, rvec *x,
			   bool bFreeEnergy, real lambda, gmx_large_int_t step);
/* Send the coordinates to our PME-only node and request a PME calculation */

extern void gmx_pme_finish(t_commrec *cr);
/* Tell our PME-only node to finish */

extern void gmx_pme_receive_f(t_commrec *cr,
			      rvec f[], matrix vir, 
			      real *energy, real *dvdlambda,
			      float *pme_cycles);
/* PP nodes receive the long range forces from the PME nodes */

extern int gmx_pme_recv_q_x(gmx_pme_pp_t pme_pp,
			    real **chargeA, real **chargeB,
			    matrix box, rvec **x,rvec **f,
			    int *maxshift0,int *maxshift1,
			    bool *bFreeEnergy,real *lambda,
			    gmx_large_int_t *step);
/* Receive charges and/or coordinates from the PP-only nodes.
 * Returns the number of atoms, or -1 when the run is finished.
 */

extern void gmx_pme_send_force_vir_ener(gmx_pme_pp_t pme_pp,
					rvec *f, matrix vir,
					real energy, real dvdlambda,
					float cycles,
					bool bGotTermSignal,
					bool bGotUsr1Signal);
/* Send the PME mesh force, virial and energy to the PP-only nodes */

#ifdef __cplusplus
}
#endif

#endif
