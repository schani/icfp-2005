#ifndef __DIJKSTRA_H
#define __DIJKSTRA_H

#include<limits.h>
#include "map.h"
#include "lexer.h"

#define INFINITY (INT_MAX>>1)

#define CHOOSE_FOOT 0
#define CHOOSE_CAR  1

typedef int cost_t;

// get distance from 'from to 'to
cost_t get_dist(map_t* map, char choose, int from, int to);

// get all distances from 'from
cost_t* get_all_dists(map_t* map, char choose, int to);

// get shortest path to 'to 
// starting at any index i the index of the next node is in rv[i]
int* get_prev(map_t* map, char choose, int to);

// allow switching from 'how to '!how while goiing 
cost_t* get_all_combined_dists(map_t* map, char choose, int to);
// should 1 or not 0 swithch when going to 'to
char* get_combined_switchp(map_t* map, char choose, int to);

// go to 'to either directly or by switching in switchpoint
// deprecated: cannot provide consistent information across switches
//int* get_combined_prev(map_t* map, char choose, int to);

// only get one dist, taking switching into consideration
cost_t get_combined_dist(map_t* map, char choose, int from, int to);

// returns CHOOSE_FOOT for robbers and cops on foot and CHOOSE_CAR for
// cops in car
int ptype_choose (ptype_t type);

#endif
