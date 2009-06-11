
/* 
 * reinkommt Resi Berghammer, die oide vom Bullen 
 * enters Resi Mountainhammer, the oldie of the Boole
 * 
 * 
 * 
 */


//
//    THIS CODE IS PRESENTED IN 
//  
//     CCCCC   III   N     N EEEEEEE M     M    A     SSSSS   CCCCC  OOOOOOO PPPPPP  EEEEEEE
//    C     C   I    NN    N E       MM   MM   A A   S     S C     C O     O P     P E
//    C         I    N N   N E       M M M M  A   A  S       C       O     O P     P E
//    C         I    N  N  N EEEEE   M  M  M A     A  SSSSS  C       O     O PPPPPP  EEEEE
//    C         I    N   N N E       M     M AAAAAAA       S C       O     O P       E
//    C     C   I    N    NN E       M     M A     A S     S C     C O     O P       E
//     CCCCC   III   N     N EEEEEEE M     M A     A  SSSSS   CCCCC  OOOOOOO P       EEEEEEE
//    
//
//
//
//
//
//        #     # #     # #     # #     # #     #  #####   #####  #     #
//        #  #  # #     # #     # #     # #     # #     # #     # #     #
//        #  #  # #     # #     # #     # #     # #       #       #     #
//        #  #  # #     # #     # #     # #     #  #####  #       #######
//        #  #  # #     # #     # #     # #     #       # #       #     #
//        #  #  # #     # #     # #     # #     # #     # #     # #     #
//         ## ##   #####   #####   #####   #####   #####   #####  #     #
// 
//                                     T.H.X. (C) Lucas Films
//
//
//
//


#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "dprintf.h"
#include "parser.h"
#include "parser_print.h"
#include "map.h"
#undef  INFINITY
#include "dijkstra.h"
#include "announcer.h"
#include "resi.h"

// FIXME: somewhere I refer to a the robber and cop start directly this can be done via info in
// skel. (Alamaise: robber-start is a nodetype, like Bank and HQ)

// AKTENNOTIZ: If the robber must not to the bank in a world you must not assume he could be there.
// AKTENNOTIZ: after smelling, remove smell roots the bot moved towards
// SDHD

#define INFTY         (INFINITY/2)
#define COPWORTH      1542
#define THE_PROP      (1)
#define SCALE         8
#define SCORE_MAGIC1  32
#define SCORE_MAGIC2  423
#define RAND_MOD      1000
#define IM_OARSCH     0
#define BANK_CONSTANT 500

#define CHOOSE_FROM_PTYPE(ptype)  ((ptype == PTYPE_COP_FOOT) ? CHOOSE_FOOT : CHOOSE_CAR)

// ALERT MEMORY PROBLEM EVASION IS NOT PERFECT YET
#define free(x) do{}while(0)

int dump_scores = 0;

static void spuel_runter(int* valuesmap, map_t* map)
{
	int i;
	for(i=0; i<map->num_nodes; i++)
	{
		dprintf("----- % 5d   %s \n", valuesmap[i], node_by_index(map, i)->loc);
	}
}


static void shout_location_count(char* prefix, brain_t* brain, int* scores)
{
	int i=0;
	int count = 0;
	
	for(i=0; i<brain->num_nodes; i++)
	{
		if(scores[i] > 0)
			count++;
	}
		
	dprintf("%s\t%s has %d possible fields\n", prefix, brain->name, count);
}


static int ada_kurds_here_p(world_skeleton_t* s, world_message_t* m, map_t* map, brain_t* brain, int pos)
{
	player_line_t* p = m->players;
	while(p!=0)
	{
		if((p->node->index == pos) && (0!=strcmp(brain->name, p->bot)))
		{
			return 1;
		}
		p = p->next;
	}
	return 0;
}

static int find_robber_location(world_message_t* m)
{
	player_line_t* p = m->players;

	while(p!=NULL)
	{
		if(p->type == PTYPE_ROBBER)
		{
			return p->node->index;
		}
		p = p->next;
	}
	return -1;
}



static int* calc_bank_map(world_skeleton_t *s, world_message_t *m, map_t *map)
{
	int* bank_gravity = calloc(sizeof(int), map->num_nodes);
	bank_value_line_t** nearest_bank = calloc(sizeof(bank_value_line_t **), map->num_nodes);
	bank_value_line_t* bv;
	int i;
	
	for(i=0; i<map->num_nodes; i++)
	{
		bank_gravity[i] = INFTY;

		for (bv = m->bank_values; bv != 0; bv = bv->next)
		{
			int bank_dist = get_dist(map, CHOOSE_FOOT, i, bv->node->index);
			
			if(bank_dist < bank_gravity[i])
			{
				bank_gravity[i] = bank_dist;
				nearest_bank[i] = bv;
			}
		}
		
		bank_gravity[i]++;
		
		int tmp = bank_gravity[i]*bank_gravity[i];
		bank_gravity[i] = ((BANK_CONSTANT+nearest_bank[i]->value/2) / tmp) / SCALE;
	}

	return bank_gravity;
}

static node_line_t*
player_node (world_message_t *m, map_t *map, char *name, ptype_t *type)
{
    node_line_t *node = 0;
    player_line_t *pl;

    for (pl = m->players; pl != 0; pl = pl->next)
	if (strcmp(pl->bot, name) == 0)
	{
	    node = pl->node;
	    if (type != 0)
		*type = pl->type;
	    break;
	}
    assert(node != 0);

    return node;
}

static int* calc_cop_map(world_skeleton_t *s, world_message_t *m, map_t *map)
{
	int* gravity = calloc(sizeof(int), map->num_nodes);
	int i;
	int k;
	
	for(i=0; i<map->num_nodes; i++)
	{
		gravity[i] = 0;
		
		for (k = 0; k < NUM_COPS; ++k)
		{
			ptype_t cop_type;
			node_line_t *cop_node;
			int cop_dist;
			
			cop_node = player_node(m, map, s->cops[k], &cop_type);
			cop_dist = get_combined_dist(map, (cop_type == PTYPE_COP_FOOT) ? CHOOSE_FOOT : CHOOSE_CAR,
						     cop_node->index, i);
			// ALERT
			//cop_dist = get_dist(map, (cop_type == PTYPE_COP_FOOT) ? CHOOSE_FOOT : CHOOSE_CAR,
			//		     cop_node->index, i);

			cop_dist++;
			
			gravity[i] += COPWORTH / (cop_dist) / SCALE;
		}
	}

	return gravity;
}

/* static int ein_cop_is_here(world_skeleton_t *s, world_message_t *m, map_t *map, int pos) */
/* { */
/* 	player_line_t* p = m->players; */
	
/* 	while(p!=IM_OARSCH) */
/* 	{ */
/* 		if(p->type==PTYPE_ROBBER) */
/* 		{ */
/* 			return 1000; */
/* 		} */
/* 		if(p->node->index == pos) */
/* 		{ */
/* 			return 1; */
/* 		} */
/* 		p = p->next; */
/* 	} */
/* 	return 0; */
/* } */

int eye_was_here(world_skeleton_t *s, world_message_t *m, map_t *map, int pos, brain_t* brain)
{
	return(pos==brain->my_prev_pos);
}



static int* calc_score_map(world_skeleton_t *s, world_message_t *m, map_t *map, brain_t* brain)
{
	int* score_map = calloc(sizeof(int), map->num_nodes);
	int* bank_map  = calc_bank_map(s, m, map);
	int* cop_map   = calc_cop_map(s, m, map);
		
	int i;
	int away;

	
	
	for(i=0; i<map->num_nodes; i++)
	{
		away = get_combined_dist(map, CHOOSE_FROM_PTYPE(brain->my_ptype),
		 brain->my_pos, i);
		//ALERT
		//away = get_dist(map, CHOOSE_FROM_PTYPE(brain->my_ptype),
		//			 brain->my_pos, i);
		
		score_map[i] = (SCORE_MAGIC1+brain->mr_proper[i])*brain->mr_proper[i];
		// score_map[i] *= brain->mr_proper[i];
		
		score_map[i] = (score_map[i] * bank_map[i]) / (1+cop_map[i]); // (away+1);
	}
	
	free(bank_map);
	free(cop_map);

	if(dump_scores)
	{
		spuel_runter(brain->mr_proper, map);
	}

	return score_map;
}

// returns most likely pos
static int get_dest(world_skeleton_t *s, world_message_t *m, map_t *map, brain_t* brain)
{
	int* scores = calc_score_map(s, m, map, brain);
	int i;
	int bestscore = -INFTY;
	int bestdst   = -1;
	int count = 0;


	for(i=0; i<map->num_nodes; i++)
	{
		if(bestscore < scores[i])
		{
			bestscore = scores[i];
			bestdst = i;
		}
		
		if(scores[i] > 0)
			count++;
	}

	dprintf("THE BEST MOVE IS TOWARDS %d with SCORE %d\n", bestdst, bestscore);
		
	return bestdst;
}


static int get_hq(map_t* map)
{
	return node_by_loc(map, "55-and-woodlawn")->index;
}

static void switch_transport(brain_t* brain, map_t* map)
{
	if(brain->my_pos == get_hq(map))
	{
		brain->my_ptype = (brain->my_ptype==PTYPE_COP_FOOT)?PTYPE_COP_CAR:PTYPE_COP_FOOT;
	}
}

cop_vote_msg_t* reserl_make_stupid_votes(world_skeleton_t* s)
{
	cop_vote_msg_t* vote = calloc(sizeof(cop_vote_msg_t), NUM_COPS);
	int i;
	
	for(i=0; i<NUM_COPS; i++)
	{
		vote[i].next = vote+i+1;
		vote[i].bot  = s->cops[i];
	}

	vote[NUM_COPS-1].next = 0;

	for(i=1; i<NUM_COPS; i++)
	{
		if(strcmp(vote[i].bot, s->name)==0)
		{
			vote[i].bot = vote[0].bot;
			vote[0].bot = s->name;
			return vote;
		}
	}
	return vote;
}

brain_t* reserl_create_brain(world_skeleton_t* s, map_t* map)
{
    brain_t* brain = calloc(sizeof(brain_t), 1);

    brain->rounds_ago = 0;
    brain->my_ptype = PTYPE_COP_FOOT;
    brain->my_pos = get_hq(map);
    brain->my_prev_pos = get_hq(map);
    brain->name = s->name;
    brain->map = map; 
    brain->num_nodes = map->num_nodes;
    brain->mr_proper = calloc(sizeof(int), map->num_nodes);
    brain->mr_proper[node_by_loc(map, "54-and-ridgewood")->index] = 1;
    brain->skel = s;
    return brain;
}



static void increase_mr_proper(brain_t* brain)
{
	int* mr_proper_add = calloc(sizeof(int), brain->map->num_nodes);
	


	int i, j;
	for(i=0; i<brain->map->num_nodes; i++)
	{
		if(brain->mr_proper[i] > 0)
		{
			// ALERT FLAT PROPERTY
			mr_proper_add[i] = 1;
			for(j=0; j<brain->map->num_nodes; j++)
			{
				if(brain->map->foot_adj[j][i])
				{
					// ALERT FLAT PROPERTY
					mr_proper_add[j] = 1;
				}
			}
		}
	}


	for(i=0; i<brain->map->num_nodes; i++)
	{
		// ALERT FLAT PROPERTY
		brain->mr_proper[i] |= mr_proper_add[i];
	}
	
	
	free(mr_proper_add);
}

static int get_smell_dist(ptype_t pt)
{
	switch(pt)
	{
	case PTYPE_COP_FOOT: 
		return 2;
	case PTYPE_COP_CAR:
		return 1;
	default:
		assert(0);
	}
}


static int* calc_stinker_map(world_skeleton_t* s, world_message_t* m, map_t* map, brain_t* brain, int weite)
{
	int* sm = calloc(sizeof(int), map->num_nodes);
	int i = 0;
	
	for(i=0; i<map->num_nodes; i++)
	{
		sm[i] = (get_dist(map, CHOOSE_FROM_PTYPE(brain->my_ptype), brain->my_pos, i)==weite);
	}
	
	return sm;
}


static void tell_mr_proper(world_message_t *m, brain_t* brain)
{
	int i;
	int robber_loc = find_robber_location(m);

	
	if(robber_loc > -1)
	{
		dprintf(" SIEG ! i weiss wo er ist der alte bot\n");
		dprintf(" SIEG ! i weiss wo er ist der alte bot\n");
		dprintf(" SIEG ! i weiss wo er ist der alte bot\n");
		dprintf(" SIEG ! i weiss wo er ist der alte bot\n");
		dprintf(" SIEG ! i weiss wo er ist der alte bot\n");
		dprintf(" SIEG ! i weiss wo er ist der alte bot\n");


		memset(brain->mr_proper, 0, sizeof(int)*brain->map->num_nodes);
		brain->mr_proper[robber_loc] = 1;
		return;
	}
	

	increase_mr_proper(brain);

	for (i = 0; i < NUM_COPS; ++i)
	{
		ptype_t cop_type;
		node_line_t *cop_node;
		
		cop_node = player_node(m, brain->map, brain->skel->cops[i], &cop_type);
		brain->mr_proper[cop_node->index] = 0;
	}


	if(m->smell > 0)
	{
		int* stinke = calc_stinker_map(brain->skel, m, brain->map, brain, m->smell);
		for(i=0; i<brain->map->num_nodes; i++)
		{
			brain->mr_proper[i] *= (1-stinke[i]);
		}
		free(stinke);
	}
	else
	{
		for(i=0; i<brain->map->num_nodes; i++)
		{
			int dist = get_dist(brain->map, (brain->my_ptype == PTYPE_COP_FOOT) ? CHOOSE_FOOT : CHOOSE_CAR,
					    brain->my_pos, i);
			if(dist <= get_smell_dist(brain->my_ptype))
				brain->mr_proper[i] = 0;
		}
	}
	
	


	evidence_line_t* e = m->evidences;
	
	while(e != NULL)
	{
		int evidence_age = m->world - e->world;
	
		// ASSUME robber goes by foot
		int* dist_map = get_all_dists(brain->map, CHOOSE_FOOT, brain->my_pos);

		for(i=0; i<brain->map->num_nodes; i++)
		{
			if(2*dist_map[i] > evidence_age)
			{
				brain->mr_proper[i] = 0;
			}
		}
		
		free(dist_map);
		e = e->next;
	}

	shout_location_count("AFTER TELL ", brain, brain->mr_proper);
}



static void update_own_pos(world_message_t* m, brain_t* brain)
{
	brain->my_prev_pos = brain->my_pos;
	brain->my_pos = player_node(m, brain->map, brain->name, &brain->my_ptype)->index;
}


void reserl_update_brain(world_message_t* m, brain_t* brain)
{
	brain->rounds_ago += 2;
	
	update_own_pos(m, brain);

	tell_mr_proper(m, brain);
}




int reserl_get_move(world_message_t* m, brain_t* brain)
{
	int bestdst;
	int* path;
	world_skeleton_t* s = brain->skel;
	map_t* map = brain->map;

	bestdst = get_dest(s, m, map, brain);
	
	if(ada_kurds_here_p(s, m, map, brain, brain->my_pos))
	{
		// FIXME we should do some schlau stuff here instead of just being drunk.
		if(rand()%3<1)
		{ 
			bestdst = (rand()%map->num_nodes);
		}
	}

	path = get_prev(map, CHOOSE_FROM_PTYPE(brain->my_ptype), bestdst);
	// ALERT
	//path = get_prev(map, CHOOSE_FROM_PTYPE(brain->my_ptype), brain->my_pos);
	
	if(get_combined_switchp(map, CHOOSE_FROM_PTYPE(brain->my_ptype), bestdst)[brain->my_pos])
	{
		dprintf("  ####   #    #    ##    #    #   ####   ###### \n");
		dprintf(" #    #  #    #   #  #   ##   #  #    #  #      \n");
		dprintf(" #       ######  #    #  # #  #  #       #####  \n");
		dprintf(" #       #    #  ######  #  # #  #  ###  #      \n");
		dprintf(" #    #  #    #  #    #  #   ##  #    #  #      \n");
		dprintf("  ####   #    #  #    #  #    #   ####   ###### \n");

		// ALERT
		switch_transport(brain, map);
		path = get_prev(map, CHOOSE_FROM_PTYPE(brain->my_ptype), bestdst);
	}
	
	dprintf("%s my destination is %d %s\n\n", brain->name, bestdst, node_by_index(map, bestdst)->loc);

	//while(path[bestdst]!=brain->my_pos)
	//{
	//bestdst = path[bestdst];
        //}

	bestdst = path[brain->my_pos];

	if(ada_kurds_here_p(s, m, map, brain, bestdst))
	{
		// FIXME we should do some schlau stuff here instead of just being drunk.
		if(rand()%3<1)
		{
			bestdst = (rand()%map->num_nodes);
		}
		
		path = get_prev(map, CHOOSE_FROM_PTYPE(brain->my_ptype), bestdst);
		
		//while(path[bestdst]!=brain->my_pos)
		//{
		//bestdst = path[bestdst];
		// }
		bestdst = path[brain->my_pos];
	}

/* 	if(bestdst==-1) */
/* 	{ */
/* 		bestdst = brain->my_pos; */
/* 	} */

	dprintf("short announce %d \n", bestdst);
	dprintf("announcing move to %s using %s \n", node_by_index(map, bestdst)->loc, (brain->my_ptype==PTYPE_COP_CAR?"my cool car":"my swollen feet"));

	return bestdst;
}

