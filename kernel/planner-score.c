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

/* $Id: planner-score.c,v 1.5 2002-06-11 18:06:06 athena Exp $ */
#include "ifftw.h"

static plan *mkplan(planner *ego, problem *p)
{
     plan *best = 0;
     int best_score;
     int cnt = 0; /* count how many solvers have the highest score */

     best_score = BAD;
     FORALL_SOLVERS(ego, s, {
	  int sc = s->adt->score(s, p);
	  if (sc == best_score)
	       ++cnt;
	  else if (sc > best_score) {
	       best_score = sc;
	       cnt = 1;
	  }
     });

     for (; best_score > BAD; --best_score) {
          FORALL_SOLVERS(ego, s, {
	       if (s->adt->score(s, p) == best_score) {
		    plan *pln = s->adt->mkplan(s, p, ego);

		    if (pln) {
			 X(plan_use)(pln);
			 ego->hook(pln, p);

			 if (cnt > 1) {
			      ego->nplan++;
			      pln->pcost = X(evaluate_plan)(ego, pln, p);
			 } else {
			      /* no need to time this unique plan */
			      A(!best);
			      pln->pcost = 0;
			 }

			 if (best) {
			      if (pln->pcost < best->pcost) {
				   X(plan_destroy)(best);
				   best = pln;
			      } else {
				   X(plan_destroy)(pln);
			      }
			 } else {
			      best = pln;
			 }
		    }
	       }
	  });
          if (best)
               break;
     };

     return best;
}

/* constructor */
planner *X(mkplanner_score)(int estimatep)
{
     return X(mkplanner)(sizeof(planner), mkplan, 0, estimatep);
}
