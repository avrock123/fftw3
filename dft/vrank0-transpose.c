/*
 * Copyright (c) 2002 Matteo Frigo
 * Copyright (c) 2002 Steven G. Johnson
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* $Id: vrank0-transpose.c,v 1.1 2002-06-08 19:51:46 athena Exp $ */

/* rank-0, vector-rank-0, square transposition  */

#include "dft.h"

/* transposition routine. TODO: optimize? */
static void t(R *rA, R *iA, uint n, int is, int js)
{
     uint i, j;

     for (i = 1; i < n; ++i) {
	  for (j = 0; j < i; ++j) {
	       R ar, ai, br, bi;

	       ar = rA[i * is + j * js];
	       ai = iA[i * is + j * js];
	       br = rA[j * is + i * js];
	       bi = iA[j * is + i * js];

	       rA[j * is + i * js] = ar;
	       iA[j * is + i * js] = ai;
	       rA[i * is + j * js] = br;
	       iA[i * is + j * js] = bi;
	  }
     }
}

typedef solver S;

typedef struct {
     plan_dft super;
     uint n;
     int s0, s1;
} P;

static void apply(plan *ego_, R *ri, R *ii, R *ro, R *io)
{
     P *ego = (P *) ego_;
     UNUSED(ro);
     UNUSED(io);
     t(ri, ii, ego->n, ego->s0, ego->s1);
}

static int applicable(const problem *p_)
{
     if (DFTP(p_)) {
	  const problem_dft *p = (const problem_dft *)p_;
	  return (1
		  && p->ri == p->ro 
		  && p->sz.rnk == 0 
		  && p->vecsz.rnk == 2 
		  && p->vecsz.dims[0].n == p->vecsz.dims[1].n 
		  && p->vecsz.dims[0].is == p->vecsz.dims[1].os 
		  && p->vecsz.dims[0].os == p->vecsz.dims[1].is
	       );
     }
     return 0;
}

static int score(const solver *ego, const problem *p)
{
     UNUSED(ego);
     return (applicable(p)) ? GOOD : BAD;
}

static void destroy(plan *ego)
{
     fftw_free(ego);
}

static void print(plan *ego_, plan_printf prntf)
{
     P *ego = (P *) ego_;
     prntf("(DFT-TRANSPOSE-%u)", ego->n);
}

static plan *mkplan(const solver *ego, const problem *p_, planner *plnr)
{
     const problem_dft *p;
     P *pln;

     static const plan_adt padt = { 
	  fftw_dft_solve, fftw_null_awake, print, destroy 
     };

     UNUSED(plnr); UNUSED(ego);

     if (!applicable(p_))
	  return (plan *) 0;
     p = (const problem_dft *) p_;

     pln = MKPLAN_DFT(P, &padt, apply);
     pln->n = p->vecsz.dims[0].n;
     pln->s0 = p->vecsz.dims[0].is;
     pln->s1 = p->vecsz.dims[0].os;

     pln->super.super.cost = 1.0;	/* FIXME? */
     pln->super.super.flops = fftw_flops_zero;
     return &(pln->super.super);
}

static solver *mksolver(void)
{
     static const solver_adt sadt = { mkplan, score };
     return MKSOLVER(S, &sadt);
}

void fftw_dft_vrank0_transpose_register(planner *p)
{
     p->adt->register_solver(p, mksolver());
}
