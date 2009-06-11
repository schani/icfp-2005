#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include <time.h>

#include "dprintf.h"
#include "coplib.h"
#include "opening_book.h"
#include "dijkstra.h"



void help_and_exit (char *argv0)
{
	dprintf ("usage: %s <copname>\n", argv0);
	dprintf ("where copname is one of karin, zak, rult, im, hirn.\n");
	dprintf (" (c) joe object 2005\n");
	exit (1);
}


node_line_t *find_player (world_skeleton_t *w, world_message_t *wm, char *name)
{
	player_line_t *p;

	for (p=wm->players; p; p=p->next) {
		if (strcmp (p->bot, name) == 0) {
			return p->node;
		}
	}
	return NULL;
}


int find_this_cop (world_skeleton_t *w, world_message_t *wm)
{
	player_line_t *p;
	int i;

	i = 0;
	for (p=wm->players; p; p=p->next) {
		if (strcmp (p->bot, w->name) == 0) {
			return i;
		}
		if (p->type != PTYPE_ROBBER) i++;
	}
	assert (0);
}

node_line_t *check_bank_robbery (world_skeleton_t *w, world_message_t *wm)
{
	return find_player (w, wm, w->robbers[0]);
}


char *nn (cop_knowledge_t *c, int index)
{
	node_line_t *n;

        n = node_by_index (c->map, index);
        return n->loc;
}


/* compute targets around robber
   do this when robber pos is known exactly.

   moves = min dist (robber, cop) * 0.75 
   forall nodes where dist (node, robber) = moves
	select 5 targets such that 
           sum sqr (dist (targetn, targetn+1)) is minimal
             and dist (targetn, targetn+1) >= smellrange (t1) + sr t2

   foreach target 
        minimize sum (target, cop)

   returns number of moves for robber to reach target.
*/

int assign_cops (cop_knowledge_t *c, int cop, int assignment[NUM_COPS], int *min_dist, int best_assignment[NUM_COPS], int cop_pos[NUM_COPS], int cop_vehicles[NUM_COPS], int targets[NUM_COPS])
{
	int i, j;
	int new_assignment[NUM_COPS];
	int dist; 

	if (cop == NUM_COPS) {
		dist = 0;

		for (i=0; i<NUM_COPS; i++) {
/* ???? also handle CAR !!! */
			dist+=get_dist (c->map, CHOOSE_FOOT, cop_pos[i], targets[assignment[i]]);
		}
// dprintf ("dist: %d best_dist: %d\n", dist, *min_dist);
		if (dist < *min_dist) {
			for (i=0; i<NUM_COPS; i++) {
// dprintf ("%d ", assignment[i]);
				best_assignment[i]=assignment[i];
				*min_dist = dist;
			}
// dprintf ("dist %d\n", dist);
		}
			
		return 0;
	}

	for (i=0; i<NUM_COPS; i++) {
		new_assignment[i]=assignment[i];
	}

	for (i=0; i<NUM_COPS; i++) {
		if (assignment[i] == -1) {
			new_assignment[i] = cop; 
			assign_cops (c, cop+1, new_assignment, min_dist, best_assignment, cop_pos, cop_vehicles, targets);
			new_assignment[i] = -1;
		}
	}

	return 0;
}

int compute_targets (cop_knowledge_t *c, world_message_t *wm, int targets[NUM_COPS])
{
	int *circle;
	int robber_pos; 

	int min_dist, dist;
	int i, j; 
	int radius;
	int ccnt; 
	int r; 
	int cop_assignment[NUM_COPS], best_assignment[NUM_COPS], cop_pos[NUM_COPS], cop_vehicles[NUM_COPS];
	int best;

	player_line_t *p, *min_cop;

        if (c->prp_cnt[c->turn-1] > 4) return 0;

        robber_pos = -1;
        for (i=0; i<c->map->num_nodes; i++) {
                if (isset_prp (c, i)) {
                        robber_pos = i;
                }
        }
        if (robber_pos == -1) {
                dprintf ("ALERT didn't find robber_pos\n");
                return 0;
        }
 dprintf ("%d robberpositions, selected %s\n", c->prp_cnt[c->turn-1], nn (c, robber_pos));

        min_dist = 99999;
        min_cop = NULL;
        for (p=wm->players; p; p=p->next) {
                if (p->type != PTYPE_ROBBER) {
                        if ((dist=get_dist (c->map, CHOOSE_FOOT, robber_pos, p->node->index)) < min_dist) {
                                min_dist = dist;
                                min_cop = p;
                        }
                }
        }
        if (!min_cop) {
// dprintf ("ALERT found no min_cop!\n");
                return 0;
        }

dprintf ("min_dist %d cop %s at %s\n", min_dist, min_cop->bot, min_cop->node->loc);
        radius = (min_dist + 1) / 2;
 dprintf ("radius %d\n", radius);

        circle = alloc_init_range(c);
	ccnt = 0;

/* compute circle */
        for (i=0; i<c->map->num_nodes; i++) {
                if (get_dist (c->map, CHOOSE_FOOT, i, robber_pos) == radius) {
                        set (circle, i);
                        ccnt++;
                }
        }

/* remove nodes that are too close to other nodes */
        for (i=0; i<c->map->num_nodes; i++) {
		if (isset (circle, i)) {
			for (j=i+1; j<c->map->num_nodes; j++) {
				if (isset (circle, j)) {
					if (get_dist (c->map, CHOOSE_FOOT, i, j) < 3) {
 dprintf ("Kicking out %s (too close to %s)\n", nn (c, j), nn (c, i));
						unset (circle, j);
						ccnt--;
						if (ccnt <= NUM_COPS) goto out;
					}
				}
			}
		}
	}	
out:

/*
dprintf ("alsdkj\n");

{ int c2=0;
 for (i=0; i<c->map->num_nodes; i++) {
	if (isset (circle, i)) { c2++; }
  }
dprintf ("c2: %d ccnt: %d\n", c2, ccnt);
 assert (ccnt == c2);
}
*/

/* select five nodes randomly. */
	while (ccnt > 5) {
		r = random () % ccnt;
// dprintf ("alsdkjdkj %d %d\n", ccnt, r);
        	for (i=0; i<c->map->num_nodes; i++) {
			if (isset (circle, i)) {
//dprintf ("isset %d\n", i);
				if (!r) {
 //dprintf ("Kicking out %s (random choice)\n", nn (c, i));
					unset (circle, i);
					ccnt--;
					break;
				}
				r--;
			}
		}
	}

//dprintf ("alsdj\n");
	j=0;
       	for (i=0; i<c->map->num_nodes; i++) {
		if (isset (circle, i)) {
// dprintf ("Selected %s as target.\n", nn (c, i));
			targets[j] = i;
			j++;
		}
	}
	assert (j);
	for (;j<NUM_COPS;j++) targets[j] = robber_pos;
/* assign cops */

	for (i=0; i<NUM_COPS; i++) {
		cop_assignment[i] = -1;
	}
	i=0;
	for (p=wm->players; p; p=p->next) {
                if (p->type != PTYPE_ROBBER) {
			cop_pos[i] = p->node->index;
			if (p->node->tag == NODE_TAG_HQ) {
				cop_vehicles[i] = -1;
			} else {
				cop_vehicles[i] = (p->type == PTYPE_COP_FOOT ? CHOOSE_FOOT : CHOOSE_CAR);
			}
			i++;
		}
	}
	assert (i==NUM_COPS);

	best = 999999;
	assign_cops (c, 0, cop_assignment, &best, best_assignment, cop_pos, cop_vehicles, targets);
	for (i=0; i<NUM_COPS; i++) {
//		dprintf ("Assigned cop at %s to target at %s.\n", nn (c, cop_pos[i]), nn (c, targets[best_assignment[i]]));
	}
	
	return radius*2;
}


char *minimal_move (cop_knowledge_t *c, int on_foot, int mypos)
	/* Minimze distance to known positions. */
{
	int i, j;
	int best, best_score; 
	int can_move;
	int score;

	best = -1;
	best_score = 99999;

	for (i=0; i<c->map->num_nodes; i++) {
		if (on_foot) {
			can_move = c->map->foot_adj[mypos][i];
		} else {
			can_move = c->map->car_adj[mypos][i];
		}

		if (can_move) {
// dprintf ("%s considering %s ..\n", c->world->name, nn (i));
			score = 0;
			for (j=0; j<c->map->num_nodes; j++) {
				if (isset_prp (c, j)) {
					int dist; 
					dist = get_dist (c->map, (on_foot ? CHOOSE_FOOT : CHOOSE_CAR) , i, j); 
// dprintf ("dist (%s -> %s): %d\n", nn(c, i), nn(c, j), dist);
					score += dist;
				}
			}
			if (score < best_score) {
				best_score = score; 
				best = i;
			}
		}
	}

	return nn (c, best);
}

	
int do_move_randomly (cop_knowledge_t *c, int on_foot, int mypos, world_message_t *wm) 
{
	player_line_t *p;
	int cnt; 

	if (c->prp_cnt[c->turn-1] > 50) return 1;

	cnt = 0;
	for (p = wm->players; p; p=p->next) {
		if (p->node->index == mypos) cnt++;
	}
	if (cnt>1) return 1;

	return 0;
}

char *random_move (cop_knowledge_t *c, int on_foot, int mypos)
{
	int i, pm, r, can_move; 

	pm = 0;
        for (i=0; i<c->map->num_nodes; i++) {
                if (on_foot) {
                        can_move = c->map->foot_adj[mypos][i];
                } else {
                        can_move = c->map->car_adj[mypos][i];
                }
		if (can_move) pm++;
	}
	r = random() % pm; 

	pm = 0;
        for (i=0; i<c->map->num_nodes; i++) {
                if (on_foot) {
                        can_move = c->map->foot_adj[mypos][i];
                } else {
                        can_move = c->map->car_adj[mypos][i];
                }
                if (can_move) { 
			if (pm == r) {
// dprintf ("%s picked random move %s\n", c->world->name, nn(c, i));
				return nn(c, i);
			}
			pm++;
		}
        }
assert (0);
}

char *move_towards (cop_knowledge_t *c, int on_foot, int mypos, int target)
{
	int best, best_dist;
	int i, can_move, dist;

	if (mypos == target) return nn (c, target);

	best_dist = 999999;
	best = -1;
        for (i=0; i<c->map->num_nodes; i++) {
                if (on_foot) {
                        can_move = c->map->foot_adj[mypos][i];
                } else {
                        can_move = c->map->car_adj[mypos][i];
                }
                if (can_move) {
			if ((dist = get_dist (c->map, on_foot?CHOOSE_FOOT:CHOOSE_CAR, i, target)) < best_dist) {
				best_dist = dist; 
				best = i;
			}
		}
        }
	assert (best != -1);
	
dprintf ("%s moving towards %s from %s via %s.\n", c->world->name, nn (c, target), nn (c, mypos), nn (c, best));
	return nn (c, best);
}

int main (int argc, char ** argv) 
{
	int copnr, i; 
	char *cc;
	int mov;
	int mypos;
	int on_foot;
	int use_targets;

	world_message_t *wm;
	cop_inform_msg_t *cim;
	vote_tally_t *vt;
	cop_plan_msg_t *pm;
	node_line_t *n;
	evidence_line_t *ev;
	
	cop_knowledge_t *c; 
	
	int targets[NUM_COPS];
	int my_copno;
	int t, target_expires; 

	for (i=0; i<NUM_COPS; i++) {
		targets[i] = -1;
	}
use_targets = 1;

	if (argc != 2) help_and_exit (*argv);
	if ((cc=strchr ("kzrih", argv[1][0])) == NULL) help_and_exit (*argv);
	copnr = cc-"kzrih";
		 /* copnr not neccesarry the one on server or in parser .. */
	// on_foot = copnr < 2;
//	on_foot = copnr < 3;
	on_foot = 1;

	srandom (time (NULL)+copnr);

	printf ("reg: %s cop-%s\n", argv[1], on_foot?"foot":"car");
	fflush (stdout);

	c=cl_init (stdin);
	cl_dump_prp (stderr, c);	
	
	mov = 0;
	my_copno = -1;
	target_expires = 0;
	while (1) {
		wm = parse_world_message (c->world);

		cl_compute_robber_moves (c);	
		if ((n = check_bank_robbery (c->world, wm)) != NULL) {
			cl_robber_robbed_bank (c, n->index);
		} else {
			cl_robber_didnt_rob_bank (c);
		}

		n = find_player (c->world, wm, c->world->name);
		my_copno = find_this_cop (c->world, wm);
dprintf ("%s (copno %d) at %s\n", c->world->name, my_copno, n->loc);
		mypos = n->index;
		cl_smell_robber (c, n->index, wm->smell, on_foot?PTYPE_COP_FOOT:PTYPE_COP_CAR);
		for (ev = wm->evidences; ev; ev=ev->next) {
dprintf (" *************** %s found evidence at %s, %d turns ago (thisworld %d evworld %d. *************\n", c->world->name, nn (c, mypos),  (wm->world - ev->world) / 2 + 1, wm->world, ev->world);
			cl_found_evidence (c, (wm->world - ev->world) / 2 + 1, mypos, wm);
		}

		printf ("inf\\\n");
		cl_inform_others (c);
		printf ("inf: %s %s %s %d 1\n", c->world->name, n->loc, on_foot ? "cop-foot" : "cop-car", c->turn); // ?? turn ??
		printf ("inf/\n");
		fflush (stdout);

		cim = parse_inform_messages (c->world);
		cl_evaluate_information (c, cim);

		printf ("plan\\\n");
		printf ("plan/\n");
		fflush (stdout);

		pm = parse_plan_messages (c->world);

		printf ("vote\\\n");
		for (i=0;i<5;i++) {
			printf ("vote: %s\n", c->world->cops[i]);
		}
		printf ("vote/\n");
		fflush (stdout);

		vt = parse_vote_tally (c->world);

if (copnr == 1) { 
	cl_dump_prp (stderr, c);
	cl_draw_knowledge_map (stderr, c, wm);
}

		t=compute_targets (c, wm, targets);
dprintf ("%s te: %d t: %d\n", c->world->name, target_expires, t);
		if (t) {
			target_expires = t;
		} else {
			if (target_expires > 0) {
				target_expires--;
if (target_expires == 0) dprintf ("%s TARGET EXPIRED.\n", c->world->name);
			}
		}

	/*	if (mov < 5) {
dprintf ("%s OPENING MOVE.\n", c->world->name);
			printf ("mov: %s cop-%s\n", opening_move (mov, copnr), on_foot?"foot":"car");
		} else */
		if (use_targets && (targets[my_copno] != -1) && (target_expires>0)) {
dprintf ("%s TARGET MOVE.\n", c->world->name);
			printf ("mov: %s cop-%s\n", move_towards (c, on_foot, mypos, targets[my_copno]), on_foot?"foot":"car");
		} else if (do_move_randomly (c, on_foot, mypos, wm)) {
dprintf ("%s RANDOM MOVE.\n", c->world->name);
			printf ("mov: %s cop-%s\n", random_move (c, on_foot, mypos), on_foot?"foot":"car");
		} else {
dprintf ("%s MINIMAL MOVE.\n", c->world->name);
			printf ("mov: %s cop-%s\n", minimal_move (c, on_foot, mypos), on_foot?"foot":"car");
		}

		fflush (stdout);

		mov++;
	}
	
	return 0;
}

	
	
