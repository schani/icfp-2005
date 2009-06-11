#ifndef __MAP_H__
#define __MAP_H__

#include <assert.h>

#include "parser.h"

typedef struct
{
    node_line_t *node;
    ptype_t type;
} cop_info_t;

typedef struct
{
    int num_nodes;
    node_line_t **nodes;
    char **foot_adj;   /* foot_adj[from][to] notzero if there's an edge. */
    char **car_adj;    /* same for cars. */

    int *robber_degrees;

    cop_info_t **foot_cop_moves;
    int *num_foot_cop_moves;
    cop_info_t **car_cop_moves;
    int *num_car_cop_moves;

    node_line_t ***robber_moves;
    int *num_robber_moves;

    int *cul_de_sacs;
} map_t;

map_t* build_map (world_skeleton_t *s);

static inline node_line_t*
node_by_index (map_t *map, int index)
{
    assert(index >= 0 && index < map->num_nodes);

    return map->nodes[index];
}

node_line_t* node_by_loc (map_t *map, char *loc);

static inline int
is_foot_edge (map_t *map, int frm_idx, int to_idx)
{
    assert(frm_idx >= 0 && frm_idx < map->num_nodes);
    assert(to_idx >= 0 && to_idx < map->num_nodes);

    return map->foot_adj[frm_idx][to_idx];
}

static inline int
is_car_edge (map_t *map, int frm_idx, int to_idx)
{
    assert(frm_idx >= 0 && frm_idx < map->num_nodes);
    assert(to_idx >= 0 && to_idx < map->num_nodes);

    return map->car_adj[frm_idx][to_idx];
}

static inline int
node_robber_degree (map_t *map, node_line_t *node)
{
    return map->robber_degrees[node->index];
}

int node_robber_degree (map_t *map, node_line_t *node);

cop_info_t* get_valid_cop_moves (map_t *map, cop_info_t *src, int *num_moves);

static inline node_line_t**
get_valid_robber_moves (map_t *map, node_line_t *src, int *num_moves)
{
    *num_moves = map->num_robber_moves[src->index];

    return map->robber_moves[src->index];
}

int is_cul_de_sac (map_t *map, node_line_t *node);

#endif
