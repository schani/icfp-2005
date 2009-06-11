/* Kottan the McGruff Controller */


#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "parser.h"
#include "parser_print.h"
#include "map.h"
#include "dijkstra.h"
#include "announcer.h"

#if 0
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
#endif

static node_line_t*
get_next_step_node (map_t *map, int frm_idx, int to_idx)
{
    if( frm_idx == to_idx)
	return node_by_index( map, to_idx );

    for (;;)
    {
	if (is_foot_edge(map, frm_idx, to_idx))
	    return node_by_index(map, to_idx);

	to_idx = get_prev(map, CHOOSE_FOOT, frm_idx)[to_idx];
    }
}

static player_line_t *
cop_with_name( player_line_t *playerlist, const char *name )
{
    assert( playerlist );
    assert( name );

    for (; playerlist != NULL; playerlist = playerlist->next)
	if (!strcmp( name, playerlist->bot )) {
	    assert( playerlist->type != PTYPE_ROBBER );

	    return playerlist;
	}

    assert( 0 );
}

static int
register_cops( player_line_t *players, char **cops, const char *name,
	       player_line_t **coplist )
{
    int i, result = -1;

    assert( players );
    assert( cops );
    assert( name );
    assert( coplist );

    for (i=0; i<NUM_COPS; i++) {
	coplist[i] = cop_with_name( players, cops[i]);
	if (!strcmp( cops[i], name))
	    result = i;
    }

    assert( result != -1 );

    return result;
}

int
main (void)
{
    world_skeleton_t *s;
    map_t *map = 0;
    announcer_t a;

    /* kottan */
    int i = 0;
    int goal = 0;
    node_line_t *bank_nodes[6], *tmp_node;

    a = init_announcer( stdout );
    announce_reg( a, "Kottan", PTYPE_COP_FOOT);

    s = parse_world_skeleton(stdin);

    //parser_print_world_skeleton(s);

    for (i=0, tmp_node = s->nodes; i<NUM_BANKS; tmp_node = tmp_node->next) {
	assert( tmp_node );

	if (tmp_node->tag == NODE_TAG_BANK)
	    bank_nodes[i++] = tmp_node;
    }

    map = build_map(s);


    for (;;)
    {
	world_message_t *m;

	/* kottan */
	player_line_t *cops[NUM_COPS];
	node_line_t *cop_next_nodes[NUM_COPS];
	int cop_jobs[NUM_COPS];
	int my_cop = -1;
	int i;
	cop_plan_msg_t plans[NUM_COPS];

	m = parse_world_message(s);

	my_cop = register_cops( m->players, s->cops, s->name, cops );
	assert( my_cop != -1 );

	//parser_print_world_message(m);

	if (!m->game_running)
	    break;

	/* game is still running, compute a move */

	/* check it out man
	 */
	if (goal == 0) { /* no goal, select banks */
	    for (i=0; i<NUM_COPS; i++)
		cop_jobs[i] = i;
	    goal++;
	}
	if (goal == 1) { /* proceed to banks */
	    for (i=0; i<NUM_COPS; i++)
		cop_next_nodes[i] = 
		    get_next_step_node( map, cops[i]->node->index,
					bank_nodes[cop_jobs[i]]->index );
	} else { /* stand bled herum */
	    cop_next_nodes[i] = cops[i]->node;
	}


	

	/* do it
	 */
	announce_inform( a, NULL );
	

	parse_inform_messages( s );

	for (i = 0; i<NUM_COPS; i++) {
	    plans[i].bot = cops[i]->bot;
	    plans[i].node = cop_next_nodes[i];
	    plans[i].type = PTYPE_COP_FOOT;
	    plans[i].world = m->world + 1;
	    plans[i].next = &plans[i+1];
	}
	plans[NUM_COPS-1].next = NULL;

	announce_plan( a, plans );

	parse_plan_messages( s );

	/* selbstvoter */
	{
	    int j;
	    cop_vote_msg_t vote [NUM_COPS];

	    vote[0].bot = s->name;
	    vote[0].next = &vote[1];

	    for (i=0, j=1; i<NUM_COPS; i++) {
		if (i == my_cop)
		    continue;
		vote[j].bot = cops[i]->bot;
		vote[j].next = &vote[j+1];
		j++;
	    }
	    vote[NUM_COPS-1].next = NULL;

	    announce_vote( a, vote );
	}

	parse_vote_tally( s );

	announce_move( a, s, m, cop_next_nodes[my_cop]->loc, PTYPE_COP_FOOT );

	free_world_message(m);
    }



    

    
/* this takes 0.380s*/
/*     for(i=0; i<map->num_nodes; i++) */
/* 	    for(j=0; j<map->num_nodes; j++) */
/* 		    get_dist(map, CHOOSE_FOOT, i, j); */
    
    /*
    m = parse_world_message(s);

    parser_print_world_message(m);
    */

    return 0;
}
