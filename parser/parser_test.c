#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>

#include "parser.h"
#include "parser_print.h"
#include "map.h"
#include "dijkstra.h"
#include "announcer.h"
#include "dprintf.h"

#define MODE_LOOT           1
#define MODE_EVADE          2

#define FOR_ALL_ROBBER_MOVES(my_node, map, node)       { int __num_nodes; \
							 node_line_t **__nodes = get_valid_robber_moves((map), (my_node), &__num_nodes); \
							 for (int __i = 0; __i < __num_nodes; ++__i) { \
							     node_line_t *node = __nodes[__i]; {
#define END_FOR_ALL_ROBBER_MOVES                       } } }

typedef struct
{
    node_line_t *node;
    int time_to_refuel;
} bank_refuel_timer_t;

typedef struct
{
    node_line_t *node;
    int cul_de_sac_penalty;
    int smell_penalty;
    int cop_penalty;
    int bank_refuel_penalty;
    int bank_loot_bonus;
    int bank_evade_penalty;
    int degree_penalty;
    int bank_distance_bonus;
} position_score_t;

static bank_refuel_timer_t bank_refuels[MAX_BANK_REFUELS];

static int best_position_score;

static int mode = MODE_EVADE;
static int cop_contact_ago = 0;

static volatile int timeout;

static void
advance_bank_refuels (void)
{
    int i;

    for (i = 0; i < MAX_BANK_REFUELS; ++i)
    {
	if (bank_refuels[i].time_to_refuel > 0)
	    --bank_refuels[i].time_to_refuel;
	if (bank_refuels[i].time_to_refuel == 0)
	    bank_refuels[i].node = 0;
    }
}

static void
register_bank_refuel (node_line_t *node, int time_to_refuel)
{
    int i;

    assert(node != 0 && time_to_refuel > 0);

    for (i = 0; i < MAX_BANK_REFUELS; ++i)
    	if (bank_refuels[i].node == 0)
	{
	    bank_refuels[i].node = node;
	    bank_refuels[i].time_to_refuel = time_to_refuel;

	    return;
	}

    assert(0);
}

static int
get_bank_refuel (node_line_t *node)
{
    int i;

    for (i = 0; i < MAX_BANK_REFUELS; ++i)
	if (bank_refuels[i].node == node)
	    return bank_refuels[i].time_to_refuel;

    return 0;
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

/*
static node_line_t*
get_next_step_node (map_t *map, int frm_idx, int to_idx)
{
    for (;;)
    {
	if (is_foot_edge(map, frm_idx, to_idx))
	    return node_by_index(map, to_idx);

	to_idx = get_prev(map, CHOOSE_FOOT, frm_idx)[to_idx];
    }
}
*/

static int
opt_invert_choose (int choose, int invert)
{
    if (choose == CHOOSE_FOOT)
    {
	if (invert)
	    return CHOOSE_CAR;
	else
	    return CHOOSE_FOOT;
    }
    else
    {
	if (invert)
	    return CHOOSE_FOOT;
	else
	    return CHOOSE_CAR;
    }
}

static int
cop_penalty (int dist)
{
    if (dist <= -2)
	return COP_MINUS_2_PENALTY;
    if (dist == -1)
	return COP_MINUS_1_PENALTY;
    else if (dist == 0)
	return COP_0_PENALTY;
    else if (dist == 1)
	return COP_1_PENALTY;
    else if (dist == 2)
	return COP_2_PENALTY;
    else if (dist == 3)
	return COP_3_PENALTY;
    else
	return COP_4_PENALTY;
}

static int
calc_best_position_score (void)
{
    int loot_score, distance_score;

    loot_score = BANK_VALUE_SCALE * TOTAL_BANK_MONEY / 5;
    distance_score = BANK_DIST_VALUE_SCALE * TOTAL_BANK_MONEY / 5;

    return loot_score + distance_score;
}

static int
total_pos_score (position_score_t *ps)
{
    return -ps->cul_de_sac_penalty - ps->smell_penalty - ps->cop_penalty - ps->bank_refuel_penalty
	+ ps->bank_loot_bonus - ps->bank_evade_penalty - ps->degree_penalty + ps->bank_distance_bonus;
}

/* future is 0 for present, 1 for cops having moved once, 2 for cops
   having moved twice, ... */
static int
position_score (world_skeleton_t *s, world_message_t *m, map_t *map, node_line_t *my_node,
		cop_info_t *cop_infos, int future, int is_end_point, position_score_t *ps)
{
    int score;
    int i;
    bank_value_line_t *bv;
    int bank_sum;
    int degree;

    assert(future > 0);

    memset(ps, 0, sizeof(position_score_t));

    ps->node = my_node;

    // calculate bank sum
    bank_sum = 0;
    for (bv = m->bank_values; bv != 0; bv = bv->next)
	bank_sum += bv->value;

    // cul de sac penalty
    if (is_cul_de_sac(map, my_node))
	ps->cul_de_sac_penalty = CUL_DE_SAC_PENALTY / (future * future);

    // smell penalty
    for (i = 0; i < NUM_COPS; ++i)
    {
	ptype_t cop_type = cop_infos[i].type;
	node_line_t *cop_node = cop_infos[i].node;
	cost_t *dists;
	char *switchps;
	int cop_choose = ptype_choose(cop_type);
	int idx;
	int smell = 0;

	dists = get_all_combined_dists(map, cop_choose, cop_node->index);
	switchps = get_combined_switchp(map, cop_choose, cop_node->index);

	for (idx = 0; idx < map->num_nodes; ++idx)
	    if (dists[idx] <= future)
	    {
		int final_choose = opt_invert_choose(cop_choose, switchps[idx]);
		int final_dist = get_dist(map, final_choose, idx, my_node->index);

		if ((final_choose == CHOOSE_FOOT && final_dist <= 2)
		    || (final_dist == CHOOSE_CAR && final_dist <= 1))
		{
		    /*
		    if (future == 1)
			dprintf("cop at %s can smell me via %s\n",
				cop_infos[i].node->loc, node_by_index(map, idx)->loc);
		    */
		    ++smell;
		}
	    }

	if (smell)
	    ps->smell_penalty += SMELL_PENALTY / future;
    }

    // cop penalties
    for (i = 0; i < NUM_COPS; ++i)
    {
	ptype_t cop_type = cop_infos[i].type;
	node_line_t *cop_node = cop_infos[i].node;
	int cop_dist;

	cop_dist = get_combined_dist(map, ptype_choose(cop_type), cop_node->index, my_node->index) - future;

	// zero distance is always penalized
	if (is_end_point || cop_dist <= 0)
	    ps->cop_penalty += cop_penalty(cop_dist) / future;
    }

    // bank refuel penalty
    for (bv = m->bank_values; bv != 0; bv = bv->next)
	if (get_bank_refuel(bv->node) > 0)
	    ps->bank_refuel_penalty += BANK_REFUEL_PENALTY / (get_dist(map, CHOOSE_FOOT,
								       bv->node->index, my_node->index) + 1);

    // bank loot bonus
    if (mode == MODE_LOOT)
    {
	for (bv = m->bank_values; bv != 0; bv = bv->next)
	    if (bv->node == my_node && !is_cul_de_sac(map, bv->node) && get_bank_refuel(bv->node) < future)
		ps->bank_loot_bonus += BANK_VALUE_SCALE * bank_sum / 5;
    }
    else			/* bank evade penalty */
    {
	assert(mode == MODE_EVADE);

	for (bv = m->bank_values; bv != 0; bv = bv->next)
	    if (bv->node == my_node)
		ps->bank_evade_penalty += BANK_EVADE_PENALTY;
    }

    if (is_end_point)		/* only for end points */
    {
	// degree penalty
	degree = node_robber_degree(map, my_node);
	assert(degree > 0);
	ps->degree_penalty += DEGREE_PENALTY / (degree * degree);

	// bank distance bonus
	if (mode == MODE_LOOT)
	{
	    for (bv = m->bank_values; bv != 0; bv = bv->next)
	    {
		int dist = get_dist(map, CHOOSE_FOOT, my_node->index, bv->node->index);

		if (dist > 0)
		    ps->bank_distance_bonus += BANK_DIST_VALUE_SCALE * bank_sum / 5 / dist;
	    }
	}
    }

    score = total_pos_score(ps);

    assert(score <= best_position_score);

    return score - best_position_score;
}

/* can return 0!!! */
static node_line_t*
search_best_target (world_skeleton_t *s, world_message_t *m, map_t *map,
		    node_line_t *my_node, cop_info_t *cop_infos, int future,
		    int score_so_far, int best_score_so_far, int max_depth,
		    int ignore_timeout, int *_best_score, position_score_t *pos_scores)
{
    int best_score;
    node_line_t *best_score_node;
    position_score_t ps;

    assert(future >= 0 && future < max_depth);

    /* check for best position */
    best_score = -INF_SCORE;
    best_score_node = 0;
    FOR_ALL_ROBBER_MOVES(my_node, map, node)
    {
	int rec_score;
	node_line_t *rec_node;
	int pos_score = position_score(s, m, map, node, cop_infos, future + 1, future == max_depth - 1, &ps);

	if (!ignore_timeout && timeout)
	    return 0;

	if (score_so_far + pos_score > best_score_so_far)
	{
	    if (future < max_depth - 1)
		rec_node = search_best_target(s, m, map, node, cop_infos, future + 1,
					      score_so_far + pos_score, best_score_so_far,
					      max_depth, ignore_timeout, &rec_score, pos_scores);
	    else
	    {
		rec_score = score_so_far + pos_score;
		rec_node = 0;
	    }

	    if (future == 0)
		dprintf("node %s has score %d\n", node->loc, rec_score);

	    if (rec_score > best_score)
	    {
		best_score = rec_score;
		best_score_node = node;
		pos_scores[future] = ps;
	    }

	    if (best_score > best_score_so_far)
		best_score_so_far = best_score;
	}
    } END_FOR_ALL_ROBBER_MOVES;

    if (!ignore_timeout && timeout)
	return 0;

    if (future == 0)
	assert(best_score_node != 0);

    *_best_score = best_score;

    return best_score_node;
}

static int
generate_cop_moves (map_t *map, int cop, cop_info_t *cop_src, cop_info_t *cop_dst, int (*recurse) (void))
{
    if (cop >= NUM_COPS)
	return recurse();
    else
    {
	int num_moves;
	cop_info_t *valid_moves = get_valid_cop_moves(map, &cop_src[cop], &num_moves);
	int i;

	for (i = 0; i < num_moves; ++i)
	{
	    cop_dst[cop] = valid_moves[i];
	    if (!generate_cop_moves(map, cop + 1, cop_src, cop_dst, recurse))
		return 0;
	}

	return 1;
    }
}

static void
init_cop_infos (world_skeleton_t *s, world_message_t *m, map_t *map, cop_info_t *cop_infos)
{
    int i;

    for (i = 0; i < NUM_COPS; ++i)
	cop_infos[i].node = player_node(m, map, s->cops[i], &cop_infos[i].type);
}

static int
cops_can_smell_me (world_skeleton_t *s, world_message_t *m, map_t *map, node_line_t *my_node)
{
    int i;

    for (i = 0; i < NUM_COPS; ++i)
    {
	ptype_t cop_type;
	node_line_t *cop_node = player_node(m, map, s->cops[i], &cop_type);
	int cop_choose = ptype_choose(cop_type);
	int dist = get_dist(map, cop_choose, cop_node->index, my_node->index);

	if ((cop_type == PTYPE_COP_FOOT && dist <= 2)
	    || (cop_type == PTYPE_COP_CAR && dist <= 1))
	{
	    dprintf("cop %s at %s on %s has distance %d\n", s->cops[i], cop_node->loc,
		    (cop_type == PTYPE_COP_FOOT) ? "foot" : "car", dist);
	    return 1;
	}
    }

    return 0;
}

static void
check_cop_smell (world_skeleton_t *s, world_message_t *m, map_t *map, node_line_t *my_node)
{
    if (cops_can_smell_me(s, m, map, my_node))
    {
	cop_contact_ago = 1;
	dprintf("alert: cops can smell me!\n");
    }
}

static void
alarm_handler (int sig)
{
    static char *message = "timeout alarm!\n";

    timeout = 1;

    write(2, message, strlen(message));
}

static void
set_timeout (void)
{
    timeout = 0;

    signal(SIGALRM, alarm_handler);
    alarm(3);

    dprintf("setting timeout\n");
}

static void
wait_for_input (void)
{
    int c;

    c = fgetc(stdin);
    ungetc(c, stdin);
    /*
    fd_set readfds;
    int result;

    do
    {
	FD_ZERO(&readfds);
	FD_SET(0, &readfds);

	result = select(1, &readfds, 0, 0, 0);
    } while (result != 1);
    */
}

static void
print_pos_scores (position_score_t *ps, int depth)
{
    int i;

    for (i = 0; i < depth; ++i)
    {
	dprintf("node %d                   %s\n"
		"  cul-de-sac penalty      %d\n"
		"  smell penalty           %d\n"
		"  cop penalty             %d\n"
		"  bank refuel penalty     %d\n"
		"  bank loot bonus         %d\n"
		"  bank evade penalty      %d\n"
		"  degree penalty          %d\n"
		"  bank distance bonus     %d\n"
		"  TOTAL                   %d\n\n",
		i, ps[i].node->loc, ps[i].cul_de_sac_penalty,
		ps[i].smell_penalty, ps[i].cop_penalty, ps[i].bank_refuel_penalty,
		ps[i].bank_loot_bonus, ps[i].bank_evade_penalty, ps[i].degree_penalty,
		ps[i].bank_distance_bonus, total_pos_score(&ps[i]));
    }
}

static void
print_next_scores (world_skeleton_t *s, world_message_t *m, map_t *map, cop_info_t *cop_infos, node_line_t *my_node)
{
    int num_moves;
    node_line_t **moves = get_valid_robber_moves(map, my_node, &num_moves);
    int i;

    dprintf("i am at %s (%d)\n", my_node->loc, my_node->index);

    for (i = 0; i < NUM_COPS; ++i)
    {
	int choose = ptype_choose(cop_infos[i].type);

	dprintf("cop at %s (%d) (choose %d, distance %d)\n",
		cop_infos[i].node->loc, cop_infos[i].node->index, choose,
		get_combined_dist(map, choose, cop_infos[i].node->index, my_node->index));
    }

    for (i = 0; i < num_moves; ++i)
    {
	position_score_t ps;

	position_score(s, m, map, moves[i], cop_infos, 1, 0, &ps);

	print_pos_scores(&ps, 1);
    }
}

int
main (void)
{
    world_skeleton_t *s;
    map_t *map;
    announcer_t a;
    int have_timeout = 0;

    /*
    set_timeout();
    have_timeout = 1;
    */

    best_position_score = calc_best_position_score();
    dprintf("best score is %d\n", best_position_score);

    a = init_announcer(stdout);
    announce_reg(a, "bowe", PTYPE_ROBBER);

    s = parse_world_skeleton(stdin);

    //parser_print_world_skeleton(s);

    map = build_map(s);

    /*
    int i;

    for (i = 0; i < map->num_nodes; ++i)
	if (is_cul_de_sac(map, node_by_index(map, i)))
	    dprintf("%s is cul de sac\n", node_by_index(map, i)->loc);
    */

    /*
    printf("dist: %d\n", get_dist(map, CHOOSE_FOOT,
				  node_index(map, "55-and-woodlawn"),
				  node_index(map, "54-and-ridgewood")));
    */
    
    /*
    int dst = loc_index(map, "55-and-s-hyde-park");
    int src = loc_index(map, "55-and-kenwood");

    int* revpath = get_combined_prev(map, CHOOSE_FOOT, src);
    char* changed= get_combined_switchp(map, CHOOSE_FOOT, src);
    
    while(revpath[dst]!=-1)
    {
	    dprintf("come-from %d %s %d\n", dst, index_loc(map, dst), changed[dst]);
	    dst = revpath[dst];
    }
    
	
    exit(0);
    */		    

    for (;;)
    {
	world_message_t *m;
	node_line_t *my_node;
	int best_score;
	node_line_t *best_score_node;
	node_line_t *best_long_score_node;
	cop_info_t cop_infos[NUM_COPS];
	position_score_t pos_scores[ROBBER_LONG_RECURSION];

	wait_for_input();

	/* in the first iteration, we already have a timeout set */
	if (!have_timeout)
	    set_timeout();

	m = parse_world_message(s);

	//parser_print_world_message(m);

	if (!m->game_running)
	    break;

	/* game is still running, compute a move */

	/* determine my location */
	my_node = player_node(m, map, s->name, 0);

	dprintf("we are at %s\n", my_node->loc);

	/* if a cops can smell us, note the contact */
	check_cop_smell(s, m, map, my_node);

	/* set evasion mode if necessary */
	if (cop_contact_ago <= EVASION_TIMEOUT)
	{
	    mode = MODE_EVADE;
	    dprintf("evade mode\n");
	}
	else
	{
	    mode = MODE_LOOT;
	    dprintf("loot mode\n");
	}

	/*
	{
	    int src = node_by_loc(map, "54-and-blackstone")->index;
	    int dst = node_by_loc(map, "54-and-ridgewood")->index;

	    dprintf("****** %d->%d : %d, %d   --   %d, %d\n",
		    src, dst,
		    get_combined_dist(map, CHOOSE_FOOT, src, dst),
		    get_combined_dist(map, CHOOSE_FOOT, dst, src),
		    get_dist(map, CHOOSE_FOOT, src, dst),
		    get_dist(map, CHOOSE_FOOT, dst, src));
	}
	*/

	/* check for best position */
	init_cop_infos(s, m, map, cop_infos);
	best_score_node = search_best_target(s, m, map, my_node, cop_infos, 0, 0, -INF_SCORE,
					     ROBBER_SHORT_RECURSION, 1, &best_score, pos_scores);
	assert(best_score_node != 0);

	best_long_score_node = search_best_target(s, m, map, my_node, cop_infos, 0, 0, -INF_SCORE,
						  ROBBER_LONG_RECURSION, 0, &best_score, pos_scores);

	alarm(0);
	have_timeout = 0;

	if (best_long_score_node != 0)
	    best_score_node = best_long_score_node;
	else
	    dprintf("got stopped by a timeout - using short version\n");

	assert(best_score_node == my_node || is_foot_edge(map, my_node->index, best_score_node->index));

	announce_move(a, s, m, best_score_node->loc, PTYPE_ROBBER);

	dprintf("best score node: %s  score: %d  loot: %d\n",
		best_score_node->loc, best_score, m->loot);

	//print_pos_scores(pos_scores, (best_long_score_node == 0) ? ROBBER_SHORT_RECURSION : ROBBER_LONG_RECURSION);

	//print_next_scores(s, m, map, cop_infos, my_node);

	/* if we step onto a bank, register the refuel */
	if (my_node->tag != NODE_TAG_BANK && best_score_node->tag == NODE_TAG_BANK)
	{
	    register_bank_refuel(best_score_node, 5);
	    cop_contact_ago = 0;
	}

	/* check cop smell again for new node */
	check_cop_smell(s, m, map, best_score_node);

	advance_bank_refuels();

	++cop_contact_ago;

	free_world_message(m);
    }





    //int i,j;


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
