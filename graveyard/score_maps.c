#include "marple.h"

#define COPWORTH      1042
#define THE_PROP      (1)
#define SCALE         8
#define SCORE_MAGIC1  256
#define SCORE_MAGIC2  423
#define RAND_MOD      1000
#define IM_OARSCH     0
#define IGNORE_FAR_AWAY_KAcK (9*9)
#define DYNAMIC_SCHEISS IGNORE_FAR_AWAY_KAcK



node_line_t* player_node (brain_t* brain, char *name, ptype_t *type)
{
    node_line_t *node = 0;
    player_line_t *pl;


    for (pl = brain->curr_world_msg->players; pl != 0; pl = pl->next)
    {
	if (strcmp(pl->bot, name) == 0)
	{
	    node = pl->node;
	    if (type != 0)
		*type = pl->type;
	    break;
	}
    }
    assert(node != 0);

    return node;
}


static int* calc_bank_map(brain_t* brain)
{
	int* bank_gravity = calloc(sizeof(int), brain->map->num_nodes);
	bank_value_line_t** nearest_bank = calloc(sizeof(bank_value_line_t **), brain->map->num_nodes);
	bank_value_line_t* bv;
	int i;
	
	for(i=0; i<brain->map->num_nodes; i++)
	{
		bank_gravity[i] = INFTY;

		for (bv = brain->curr_world_msg->bank_values; bv != 0; bv = bv->next)
		{
			int bank_dist = get_dist(brain->map, CHOOSE_FOOT, i, bv->node->index);
			
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


static int* calc_cop_map(brain_t* brain)
{
	int* gravity = calloc(sizeof(int), brain->map->num_nodes);
	int i;
	int k;
	
	for(i=0; i<brain->map->num_nodes; i++)
	{
		gravity[i] = 0;

		for (k = 0; k < NUM_COPS; ++k)
		{
			ptype_t cop_type;
			node_line_t *cop_node;
			int cop_dist;
			
			cop_node = player_node(brain, brain->skel->cops[k], &cop_type);
			//cop_dist = get_combined_dist(map, (cop_type == PTYPE_COP_FOOT) ? CHOOSE_FOOT : CHOOSE_CAR,
			// cop_node->index, i);
			// ALERT
			cop_dist = get_dist(brain->map, (cop_type == PTYPE_COP_FOOT) ? CHOOSE_FOOT : CHOOSE_CAR,
						     cop_node->index, i);

			cop_dist++;
			
			gravity[i] += COPWORTH / (cop_dist*cop_dist) / SCALE;
		}
	}

	return gravity;
}


int ein_cop_is_here(brain_t* brain, int pos)
{
	player_line_t* p = brain->curr_world_msg->players;
	
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

int eye_was_here(brain_t* brain, int pos)
{
	return(pos==brain->my_prev_pos);
}


static char* calc_possible_map(brain_t* brain)
{
	char* poss_map = calloc(sizeof(int), brain->map->num_nodes);
	int i;
	int count = 0;

	fprintf(stderr, "===================OARSCH=====================\n");
	for(i=0; i<brain->num_last; i++)
	{
		//int* dists = get_all_dists(map, CHOOSE_FOOT, brain->last_pos[i]);
		int j;

		// assert(sddhd);


		for(j=0; j<brain->map->num_nodes; j++)
		{
			if(ein_cop_is_here(brain, j))
			{
				poss_map[j] = 0;
			}
			else 
			{
				int dist = get_dist(brain->map, CHOOSE_FOOT, j, brain->last_pos[i]);
				if(dist < brain->rounds_ago)
				{
					count++;
					// ALERT
					poss_map[j] = THE_PROP; // * (SCORE_MAGIC2/(1+dist));
				}
			}
		}
	}
	

	return poss_map;
}


int* calc_score_map(brain_t* brain)
{
	int* score_map = calloc(sizeof(int), brain->map->num_nodes);
	int* bank_map  = calc_bank_map(brain);
	int* cop_map   = calc_cop_map(brain);
	char* poss_map = calc_possible_map(brain);

	int i;
	int away;
	
	for(i=0; i<brain->map->num_nodes; i++)
	{
		//away = get_combined_dist(map, CHOOSE_FROM_PTYPE(brain->my_ptype),
		// brain->my_pos, i);
		//ALERT
		away = get_dist(brain->map, CHOOSE_FROM_PTYPE(brain->my_ptype),
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
