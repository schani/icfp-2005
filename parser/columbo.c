#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "dprintf.h"


#include "parser.h"
#include "parser_print.h"
#include "map.h"
#undef  INFINITY
#include "dijkstra.h"
#include "announcer.h"

// FIXME: somewhere I refer to a the robber and cop start directly this can be done via info in
// skel. (Alamaise: robber-start is a nodetype, like Bank and HQ)

// AKTENNOTIZ: If the robber must not to the bank in a world you must assume he could be there.
// AKTENNOTIZ: after smelling, remove smell roots the bot moved towards

#define INFTY         (INFINITY/2)
#define COPWORTH      1042
#define THE_PROP      (1)
#define SCALE         8
#define SCORE_MAGIC1  256
#define SCORE_MAGIC2  423
#define RAND_MOD      1000
#define IM_OARSCH     0
#define IGNORE_FAR_AWAY_KAcK (9*9)
#define DYNAMIC_SCHEISS IGNORE_FAR_AWAY_KAcK



#define CHOOSE_FROM_PTYPE(ptype)  ((ptype == PTYPE_COP_FOOT) ? CHOOSE_FOOT : CHOOSE_CAR)

typedef struct {
	int rounds_ago;
	ptype_t my_ptype;
	int my_pos;
	int my_prev_pos;
	int num_last; 
	char* name;
	int dynamic_cutoff;
	int last_pos[100]; 
} brain_t;

static int calc_dynamic_cutoff(world_skeleton_t* s, world_message_t* m, map_t* map, brain_t* brain)
{
	brain->dynamic_cutoff = DYNAMIC_SCHEISS/(1+sqrt(brain->rounds_ago));
	return brain->dynamic_cutoff;
}


int ada_kurds_here_p(world_skeleton_t* s, world_message_t* m, map_t* map, brain_t* brain)
{
	player_line_t* p = m->players;
	while(p!=0)
	{
		if((p->node->index == brain->my_pos) && (0!=strcmp(brain->name, p->bot)))
		{
			return 1;
		}
		p = p->next;
	}
	return 0;
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
		bank_gravity[i] = (nearest_bank[i]->value / tmp) / SCALE;
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
			//cop_dist = get_combined_dist(map, (cop_type == PTYPE_COP_FOOT) ? CHOOSE_FOOT : CHOOSE_CAR,
			// cop_node->index, i);
			// ALERT
			cop_dist = get_dist(map, (cop_type == PTYPE_COP_FOOT) ? CHOOSE_FOOT : CHOOSE_CAR,
						     cop_node->index, i);

			cop_dist++;
			
			gravity[i] += COPWORTH / (cop_dist*cop_dist) / SCALE;
		}
	}

	return gravity;
}

int ein_cop_is_here(world_skeleton_t *s, world_message_t *m, map_t *map, int pos)
{
	player_line_t* p = m->players;
	
	while(p!=IM_OARSCH)
	{
		if(p->type==PTYPE_ROBBER)
		{
			return 1000;
		}
		if(p->node->index == pos)
		{
			return 1;
		}
		p = p->next;
	}
	return 0;
}

int eye_was_here(world_skeleton_t *s, world_message_t *m, map_t *map, int pos, brain_t* brain)
{
	return(pos==brain->my_prev_pos);
}


static char* calc_possible_map(world_skeleton_t *s, world_message_t *m, map_t *map, brain_t* brain)
{
	char* poss_map = calloc(sizeof(int), map->num_nodes);
	int i;
	int count = 0;

	for(i=0; i<brain->num_last; i++)
	{
		//int* dists = get_all_dists(map, CHOOSE_FOOT, brain->last_pos[i]);
		int j;

		// assert(sddhd);


		for(j=0; j<map->num_nodes; j++)
		{
			if(ein_cop_is_here(s, m, map, j))
			{
				poss_map[j] = 0;
			}
			else 
			{
				int dist = get_dist(map, CHOOSE_FOOT, j, brain->last_pos[i]);
				if(dist < brain->rounds_ago)
				{
					count++;
					// ALERT
					poss_map[j] = THE_PROP * (SCORE_MAGIC2/(1+dist));
				}
			}
		}
	}
	
	// PASS 2
	count=0;
	int dynamic = calc_dynamic_cutoff(s, m, map, brain);
	for(i=0; i<map->num_nodes; i++)
	{
		if(poss_map[i] < dynamic)
		{
			poss_map[i] = 0;
		}
		else
		{
			count++;
		}
	}


	return poss_map;
}


static int* calc_score_map(world_skeleton_t *s, world_message_t *m, map_t *map, brain_t* brain)
{
	int* score_map = calloc(sizeof(int), map->num_nodes);
	int* bank_map  = calc_bank_map(s, m, map);
	int* cop_map   = calc_cop_map(s, m, map);
	char* poss_map = calc_possible_map(s, m, map, brain);
		
	int i;
	int away;

	
	
	for(i=0; i<map->num_nodes; i++)
	{
		//away = get_combined_dist(map, CHOOSE_FROM_PTYPE(brain->my_ptype),
		// brain->my_pos, i);
		//ALERT
		away = get_dist(map, CHOOSE_FROM_PTYPE(brain->my_ptype),
					 brain->my_pos, i);
		
		score_map[i] = (SCORE_MAGIC1*poss_map[i]);
		score_map[i] *= poss_map[i];
		
		score_map[i] = (score_map[i]*bank_map[i]/(1+cop_map[i])) / (away+1);
	}

	free(bank_map);
	free(cop_map);
	free(poss_map);
	return score_map;
}

// returns most likely pos
int get_dest(world_skeleton_t *s, world_message_t *m, map_t *map, brain_t* brain)
{
	int* scores = calc_score_map(s, m, map, brain);
	int i;
	int bestscore = -INFTY;
	int bestdst   = -1;


	for(i=0; i<map->num_nodes; i++)
	{
		if(bestscore < scores[i])
		{
			bestscore = scores[i];
			bestdst = i;
		}
	}
	
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

static cop_vote_msg_t* make_stupid_votes(world_skeleton_t* s, brain_t* brain)
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
		if(strcmp(vote[i].bot, brain->name)==0)
		{
			vote[i].bot = vote[0].bot;
			vote[0].bot = brain->name;
			return vote;
		}
	}
	return vote;
}

brain_t* create_brain(char* name, map_t* map)
{
    brain_t* brain = calloc(sizeof(brain_t), 1);

    brain->num_last = 1;
    brain->rounds_ago = 0;
    brain->my_ptype = PTYPE_COP_FOOT;
    brain->my_pos = get_hq(map);
    brain->my_prev_pos = get_hq(map);
    brain->last_pos[0] = node_by_loc(map, "54-and-ridgewood")->index;
    brain->name = name;
    return brain;
}


int find_robber_location(world_message_t* m)
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

void update_kurden_pos(brain_t* b, world_message_t* m, int loc, int age)
{
	b->last_pos[0] = loc;
	b->rounds_ago = 1;
	b->num_last = 1;
}

void update_haeusl_evidence(world_skeleton_t *s, world_message_t *m, map_t *map, brain_t* brain)
{
	evidence_line_t* e = m->evidences;
	
	while(e != NULL)
	{
		int evidence_age = m->world - e->world;

		if(evidence_age < brain->rounds_ago)
		{
			update_kurden_pos(brain, m, e->node->index, evidence_age);
		}
		
		e = e->next;
	} 
}

int* calc_stinker_map(world_skeleton_t* s, world_message_t* m, map_t* map, brain_t* brain, int weite)
{
	int* sm = calloc(sizeof(int), map->num_nodes);
	int i = 0;
	
	for(i=0; i<map->num_nodes; i++)
	{
		sm[i] = (get_dist(map, CHOOSE_FROM_PTYPE(brain->my_ptype), brain->my_pos, i)==weite);
	}
	
	return sm;
}

void update_stinker_positions(world_skeleton_t* s, world_message_t* m, map_t* map, brain_t* brain)
{
	char* poss = calc_possible_map(s, m, map, brain);
	int* stink = calc_stinker_map(s, m, map, brain, m->smell);
	int i = 0;
	int sc = 0;


	for(i=0; i<map->num_nodes; i++)
	{
		stink[i] *= poss[i];
	}
	
	for(i=0; i<map->num_nodes; i++)
	{
		if(stink[i])
		{
			sc++;
			brain->last_pos[sc] = i;
		}
	}
	brain->num_last = sc;
	
	free(stink);
	free(poss);
}

void update_own_pos(world_message_t* m, map_t* map, brain_t* brain)
{
	brain->my_prev_pos = brain->my_pos;
	brain->my_pos = player_node(m, map, brain->name, &brain->my_ptype)->index;
}


void update_brain(world_skeleton_t* s, world_message_t* m, map_t* map, brain_t* brain)
{
	brain->rounds_ago += 2;
	
	int robber_loc = find_robber_location(m);
	update_own_pos(m, map, brain);
	if(robber_loc > -1)
	{
		update_kurden_pos(brain, m, robber_loc, 0);
	}
	update_haeusl_evidence(s, m, map, brain);
	if(m->smell>0)
	{
		update_stinker_positions(s, m, map, brain);
	}

}



int main (int argc, char** argv)
{
    world_skeleton_t *s;
    map_t *map;
    announcer_t a;

    char* name = "mrscolumbo";

    brain_t* brain;

    if(argc>1)
    {
	    srand(argv[1][0]);
	    
    }

    a = init_announcer(stdout);
    announce_reg(a, name, PTYPE_COP_FOOT);

    s = parse_world_skeleton(stdin);

    //parser_print_world_skeleton(s);

    map = build_map(s);

    dprintf("NAME: %s\n", s->name);

    brain = create_brain(s->name, map);
    /*
    printf("dist: %d\n", get_dist(map, CHOOSE_FOOT,
				  node_index(map, "55-and-woodlawn"),
				  node_index(map, "54-and-ridgewood")));
    */


    for(;;)
    {
	world_message_t *m;
	node_line_t *my_node;
	cop_inform_msg_t* inform;
	cop_plan_msg_t* plans;
	cop_vote_msg_t* votes;
	vote_tally_t* tally;
	int bestdst;
	int* path;

	m = parse_world_message(s);
	
	announce_inform(a, NULL);

	if(!m->game_running)
		break; 

	my_node = player_node(m, map, s->name, 0); 

	update_brain(s, m, map, brain);
	
	inform = parse_inform_messages(s);
	free_inform_messages(inform);

	announce_plan(a, NULL);
	plans = parse_plan_messages(s);

	free_plan_messages(plans);
	
	votes = make_stupid_votes(s, brain);
	announce_vote(a, votes);
	free(votes);

	tally = parse_vote_tally(s);
	free_vote_tally(tally);


	bestdst = get_dest(s, m, map, brain);
	
	if(ada_kurds_here_p(s, m, map, brain))
	{
		bestdst = (rand()%(map->num_nodes));
	}


	//path = get_combined_prev(map, CHOOSE_FROM_PTYPE(brain->my_ptype), brain->my_pos);
	// ALERT
	path = get_prev(map, CHOOSE_FROM_PTYPE(brain->my_ptype), brain->my_pos);
	
	if(get_combined_switchp(map, CHOOSE_FROM_PTYPE(brain->my_ptype), brain->my_pos))
	{
		// ALERT
		//switch_transport(brain, map);
	}
	

	while(path[bestdst]!=brain->my_pos)
	{
		bestdst = path[bestdst];
	}

	announce_move(a, s, m, node_by_index(map, bestdst)->loc, brain->my_ptype);
    }
	

    return 0;
}

