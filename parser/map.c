#include <stdlib.h>

#include "map.h"
#include "utils.h"
#include "dijkstra.h"

static int
is_valid_cop_move (map_t *map, node_line_t *src, ptype_t src_type, node_line_t *dst, ptype_t dst_type)
{
    if (src_type == dst_type || src->tag == NODE_TAG_HQ)
    {
	if (src == dst)
	    return 1;

	if (dst_type == PTYPE_COP_FOOT)
	    return is_foot_edge(map, src->index, dst->index);
	else
	{
	    assert(dst_type == PTYPE_COP_CAR);
	    return is_car_edge(map, src->index, dst->index);
	}
    }
    else
	return 0;
}

static int
is_valid_robber_move (map_t *map, node_line_t *src, node_line_t *dst)
{
    return src == dst || is_foot_edge(map, src->index, dst->index);
}

static void
init_moves (map_t *map, int i, cop_info_t **cop_moves, int *num_cop_moves, ptype_t src_type)
{
    int num_moves = 0;
    int j, k;

    for (j = 0; j < map->num_nodes; ++j)
    {
	if (is_valid_cop_move(map, map->nodes[i], src_type, map->nodes[j], PTYPE_COP_FOOT))
	    ++num_moves;
	if (is_valid_cop_move(map, map->nodes[i], src_type, map->nodes[j], PTYPE_COP_CAR))
	    ++num_moves;
    }

    num_cop_moves[i] = num_moves;

    cop_moves[i] = (cop_info_t*)malloc(sizeof(cop_info_t) * num_moves);
    assert(cop_moves[i] != 0);

    k = 0;
    for (j = 0; j < map->num_nodes; ++j)
    {
	if (is_valid_cop_move(map, map->nodes[i], src_type, map->nodes[j], PTYPE_COP_FOOT))
	{
	    cop_moves[i][k].node = map->nodes[j];
	    cop_moves[i][k].type = PTYPE_COP_FOOT;
	    ++k;
	}

	if (is_valid_cop_move(map, map->nodes[i], src_type, map->nodes[j], PTYPE_COP_CAR))
	{
	    cop_moves[i][k].node = map->nodes[j];
	    cop_moves[i][k].type = PTYPE_COP_CAR;
	    ++k;
	}
    }

    assert(k == num_moves);
}

static void
init_cop_moves (map_t *map)
{
    int i;

    map->foot_cop_moves = (cop_info_t**)malloc(sizeof(cop_info_t*) * map->num_nodes);
    assert(map->foot_cop_moves != 0);

    map->car_cop_moves = (cop_info_t**)malloc(sizeof(cop_info_t*) * map->num_nodes);
    assert(map->car_cop_moves != 0);

    map->num_foot_cop_moves = (int*)malloc(sizeof(int) * map->num_nodes);
    assert(map->num_foot_cop_moves != 0);

    map->num_car_cop_moves = (int*)malloc(sizeof(int) * map->num_nodes);
    assert(map->num_car_cop_moves != 0);

    for (i = 0; i < map->num_nodes; ++i)
    {
	init_moves(map, i, map->foot_cop_moves, map->num_foot_cop_moves, PTYPE_COP_FOOT);
	init_moves(map, i, map->car_cop_moves, map->num_car_cop_moves, PTYPE_COP_CAR);
    }
}

static void
init_robber_moves (map_t *map)
{
    int i, j;

    map->robber_moves = (node_line_t***)malloc(sizeof(node_line_t**) * map->num_nodes);
    assert(map->robber_moves != 0);

    map->num_robber_moves = (int*)malloc(sizeof(int) * map->num_nodes);
    assert(map->num_robber_moves != 0);

    for (i = 0; i < map->num_nodes; ++i)
    {
	int num_moves = 0;
	int k;

	for (j = 0; j < map->num_nodes; ++j)
	    if (is_valid_robber_move(map, map->nodes[i], map->nodes[j]))
		++num_moves;

	map->num_robber_moves[i] = num_moves;

	map->robber_moves[i] = (node_line_t**)malloc(sizeof(node_line_t*) * num_moves);
	assert(map->robber_moves[i] != 0);

	k = 0;
	for (j = 0; j < map->num_nodes; ++j)
	    if (is_valid_robber_move(map, map->nodes[i], map->nodes[j]))
		map->robber_moves[i][k++] = map->nodes[j];

	assert(k == num_moves);
    }
}

node_line_t*
node_by_loc (map_t *map, char *loc)
{
    int i;

    for (i = 0; i < map->num_nodes; ++i)
	if (strcmp(loc, map->nodes[i]->loc) == 0)
	    return map->nodes[i];

    assert(0);
    return 0;
}

map_t*
build_map (world_skeleton_t *s)
{
    map_t *map = MALLOC_TYPE(map_t);
    int i;
    node_line_t *n;
    edge_line_t *e;

    i = 0;
    for (n = s->nodes; n != 0; n = n->next)
	++i;

    map->num_nodes = i;

    i = 0;
    for (n = s->nodes; n != 0; n = n->next)
	assert(n->index >= 0 && n->index < map->num_nodes);

    map->nodes = (node_line_t**)malloc(sizeof(node_line_t*) * map->num_nodes);
    assert(map->nodes != 0);

    map->foot_adj = (char**)malloc(sizeof(char*) * map->num_nodes);
    assert(map->foot_adj != 0);

    map->car_adj = (char**)malloc(sizeof(char*) * map->num_nodes);
    assert(map->car_adj != 0);

    for (i = 0; i < map->num_nodes; ++i)
    {
	map->foot_adj[i] = (char*)malloc(map->num_nodes);
	assert(map->foot_adj[i] != 0);
	memset(map->foot_adj[i], 0, map->num_nodes);

	map->car_adj[i] = (char*)malloc(map->num_nodes);
	assert(map->car_adj[i] != 0);
	memset(map->car_adj[i], 0, map->num_nodes);
    }

    for (n = s->nodes; n != 0; n = n->next)
	map->nodes[n->index] = n;

    for (e = s->edges; e != 0; e = e->next)
    {
	int src_idx = e->src_node->index;
	int dst_idx = e->dst_node->index;

	if (e->type == EDGE_TYPE_FOOT)
	{
	    map->foot_adj[src_idx][dst_idx] = map->foot_adj[dst_idx][src_idx] = 1;
	    map->car_adj[src_idx][dst_idx] = 1;
	}
	else
	{
	    assert(e->type == EDGE_TYPE_CAR);

	    map->car_adj[src_idx][dst_idx] = 1;
	}
    }

    map->robber_degrees = (int*)malloc(sizeof(int) * map->num_nodes);
    for (i = 0; i < map->num_nodes; ++i)
    {
	int j;
	int degree = 0;

	for (j = 0; j < map->num_nodes; ++j)
	    if (is_foot_edge(map, i, j))
		++degree;

	map->robber_degrees[i] = degree;
    }

    map->cul_de_sacs = (int*)malloc(sizeof(int) * map->num_nodes);
    for (i = 0; i < map->num_nodes; ++i)
	map->cul_de_sacs[i] = -1;

    init_cop_moves(map);
    init_robber_moves(map);

    return map;
}

cop_info_t*
get_valid_cop_moves (map_t *map, cop_info_t *src, int *num_moves)
{
    if (src->type == PTYPE_COP_FOOT)
    {
	*num_moves = map->num_foot_cop_moves[src->node->index];
	return map->foot_cop_moves[src->node->index];
    }
    else
    {
	assert(src->type == PTYPE_COP_CAR);
	*num_moves = map->num_car_cop_moves[src->node->index];
	return map->car_cop_moves[src->node->index];
    }
}

int
is_cul_de_sac (map_t *map, node_line_t *node)
{
    if (map->cul_de_sacs[node->index] < 0)
    {
	int num_occs[map->num_nodes];
	int i;
	int is_sac;

	for (i = 0; i < map->num_nodes; ++i)
	    num_occs[i] = 0;

	for (i = 0; i < map->num_nodes; ++i)
	    if (i != node->index)
	    {
		int idx = i;

		while (idx >= 0 && idx != node->index)
		{
		    idx = get_prev(map, CHOOSE_FOOT, node->index)[idx];
		    assert(idx >= 0);
		    if (idx != node->index)
			++num_occs[idx];
		}
	    }

	is_sac = 0;
	for (i = 0; i < map->num_nodes; ++i)
	    if (num_occs[i] >= map->num_nodes - 10)
	    {
		is_sac = 1;
		break;
	    }

	map->cul_de_sacs[node->index] = is_sac;
    }

    assert(map->cul_de_sacs[node->index] >= 0);

    return map->cul_de_sacs[node->index];
}
