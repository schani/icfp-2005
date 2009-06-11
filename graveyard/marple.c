
char *strdup(const char *s);

#include "marple.h"


int copbot_main_loop (char* name, brain_t* state, bot_calls_t* bot_calls);

extern node_line_t* player_node (brain_t* brain, char *name, ptype_t *type);
extern int* calc_score_map(brain_t* brain);

int sharing_cop_position_p(brain_t* brain)
{
	player_line_t* p = brain->curr_world_msg->players;

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



brain_t* alloc_brain()
{
	brain_t* brain = (brain_t*)malloc(sizeof(brain_t*));

	return brain;
}

brain_t* update_world_brain(brain_t* state, world_message_t* m)
{
	LOCAL_BRAINSTATE;
	brain->round = m->world;
	brain->old_world_msg[m->world] = m;
	brain->curr_world_msg = m;
	
	brain->my_prev_pos = brain->my_pos;
	
	brain->my_pos = player_node(brain, brain->name, &brain->my_ptype)->index;	
	
	return brain;
}


brain_t* init_brain(brain_t* state, map_t* map, world_skeleton_t* s)
{
	LOCAL_BRAINSTATE;

	brain->map = map;
	brain->skel = s;
	brain->name = strdup(s->name);

	fprintf(stderr, "THE NAME IS %s @ %p\n", brain->name, brain->name);
	return brain;
}


static cop_vote_msg_t* make_stupid_votes(brain_t* state)
{
	LOCAL_BRAINSTATE;
	
	cop_vote_msg_t* vote = calloc(sizeof(cop_vote_msg_t), NUM_COPS);
	int i;
	
	for(i=0; i<NUM_COPS; i++)
	{
		vote[i].next = vote+i+1;
		vote[i].bot  = brain->skel->cops[i];
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

// returns most likely pos
int get_dest(brain_t* brain)
{
	int* scores = calc_score_map(brain);
	int i;
	int bestscore = -INFTY;
	int bestdst   = -1;


	for(i=0; i<brain->map->num_nodes; i++)
	{
		if(bestscore < scores[i])
		{
			bestscore = scores[i];
			bestdst = i;
		}
	}
	
	free(scores);
	return bestdst;
}


int calc_next_move(brain_t* state)
{
	LOCAL_BRAINSTATE;

	int bestdst = get_dest(brain);
	
	if(sharing_cop_position_p(brain))
	{
		bestdst = (rand()%(brain->map->num_nodes));
	}


	//path = get_combined_prev(map, CHOOSE_FROM_PTYPE(brain->my_ptype), brain->my_pos);
	// ALERT
	int* path = get_prev(brain->map, CHOOSE_FROM_PTYPE(brain->my_ptype), brain->my_pos);
	
	if(get_combined_switchp(brain->map, CHOOSE_FROM_PTYPE(brain->my_ptype), brain->my_pos))
	{
		// ALERT
		//switch_transport(brain, map);
	}
	
	fprintf(stderr, "my destination is %d %s\n\n", bestdst, node_by_index(brain->map, bestdst)->loc);

	while(path[bestdst]!=brain->my_pos)
	{
		bestdst = path[bestdst];
	}
	
	brain->my_ptype = PTYPE_COP_FOOT;
	return bestdst;
}

ptype_t calc_next_ptype(brain_t* state)
{
	LOCAL_BRAINSTATE;
	return brain->my_ptype;
}


int main(int argc, char** argv)
{
	brain_t* brain = alloc_brain();
	bot_calls_t bot_calls;
	
	char* name = "mrsmarple";
	
	if(argc>1)
	{
		srand(argv[1][0]);
	}

	STOP;

	bot_calls.init_state = init_brain;
	bot_calls.update_world = update_world_brain;
	bot_calls.create_votes = make_stupid_votes;
	bot_calls.get_next_position = calc_next_move;
	bot_calls.get_next_ptype = calc_next_ptype;

	copbot_main_loop(name, brain, &bot_calls);
	return 0;
}


int copbot_main_loop (char* name, brain_t* state, bot_calls_t* bot)
{
    world_skeleton_t *s;
    map_t *map;
    announcer_t a;

    assert(0!=bot->init_state);
    assert(0!=bot->update_world);
    assert(0!=bot->create_votes);
    assert(0!=bot->get_next_position);
    assert(0!=bot->get_next_ptype);

    a = init_announcer(stdout);
    announce_reg(a, name, PTYPE_COP_FOOT);

    s = parse_world_skeleton(stdin);

    //parser_print_world_skeleton(s);

    map = build_map(s);

    bot->init_state(state, map, s);

    STOP;

    for(;;)
    {
	world_message_t *m;
	cop_inform_msg_t* inform;
	cop_plan_msg_t* plans;
	cop_vote_msg_t* votes;
	vote_tally_t* tally;

	m = parse_world_message(s);

	if(!m->game_running)
		break; 

	bot->update_world(state, m);
	
	// FIXME code inform msg.

	announce_inform(a, NULL);

	
	inform = parse_inform_messages(s);

	// FIXME code inform msg.
	
	free_inform_messages(inform);

	fprintf(stderr, "announcing plan\n");
	announce_plan(a, NULL);
	plans = parse_plan_messages(s);
	
	// FIXME code plan msg.

	free_plan_messages(plans);
	
	votes = bot->create_votes(state);

	announce_vote(a, votes);
	free(votes);

	tally = parse_vote_tally(s);

	// FIXME code tally msg.
	
	free_vote_tally(tally);

	int next_position = bot->get_next_position(state);
	ptype_t next_ptype = bot->get_next_ptype(state);
	
	fprintf(stderr, "announcing move\n");
	announce_move(a, s, m, node_by_index(map, next_position)->loc, next_ptype);
    }
	

    return 0;
}
