#include<limits.h>
#include<assert.h>
#include<stdlib.h>
#include<string.h>

#include"map.h"
#include"dijkstra.h"

#define NO_LINK 0
#define NO_NODE -1
#define YES 1
#define NO  0
#define UNKNOWN -1

struct state 
{
	int size;
	int source;
	cost_t* dist;
	char* done;
	int* prev;
	char** adj;
};

struct comb_state
{
	int source;
	cost_t* dist;
	int* prev;
	char* change;
};

static map_t* map = 0;
struct cache
{
	struct state** states;
	struct comb_state** comb_states;
};

static struct cache cache[2] = {{0,0},{0,0}};


struct state* create_state(int source, char choose)
{
	int i;
	int size = map->num_nodes;
	struct state* s = calloc(sizeof(struct state), 1);

	assert(source>-1);
	assert(source<map->num_nodes);

	s->size = size;
	s->source = source;
	s->dist = calloc(sizeof(int), size);
	s->done = calloc(sizeof(char), size);
	s->prev = calloc(sizeof(int), size);

	if(choose == CHOOSE_FOOT)
		s->adj = map->foot_adj;
	else
		s->adj = map->car_adj;

	for(i=0;i<size;i++)
	{
		s->dist[i] = INFINITY;
		s->prev[i] = NO_NODE;
	}

	s->dist[source] = 0;


	return s;
}

// RELAX(u,v,w) == relax(state, u, v)
void relax(struct state* state, int from, int to)
{
	assert(state->adj[to][from]>0);

	cost_t new_cost = state->dist[from] + (cost_t) state->adj[to][from];
	if(state->dist[to] > new_cost)
	{
		state->dist[to] = new_cost;
		state->prev[to] = from;
	}
}

int get_min(struct state* state)
{
	int i;
	cost_t min = INFINITY;
	int best = -1;
	
	for(i=0; i<state->size; i++)
	{
		if(!state->done[i])
		{
			if(state->dist[i] < min)
			{
				best = i;
				min = state->dist[i];
			}
		}
	}
	return best;
}


void add_done(struct state* state, int node)
{
	state->done[node] = 1;
}


/** currently ~N^2 **/
struct state* calc_dijkstra(char choose, int source)
{
	struct state* state = create_state(source, choose);
	int neigh;
	int next_node;
	
	while((next_node = get_min(state))!=NO_NODE)
	{
		add_done(state, next_node);
		
		// foreach neighbour of next_node
		for(neigh=0; neigh < state->size; neigh++)
		{
			if(state->adj[neigh][next_node]!=NO_LINK)
			{ 			      // (if neigh is neighbour)
				relax(state, next_node, neigh);
			}
		}
	}

	// secure against the vicious -1 
	state->prev[source] = source;

	return state;
}


void init_cache(map_t* the_map)
{
	if(map == 0)
	{
		map = the_map;
		cache[0].states = calloc(sizeof(struct states*),map->num_nodes);
		cache[0].comb_states = calloc(sizeof(struct comb_states*),map->num_nodes);
		cache[1].states = calloc(sizeof(struct states*),map->num_nodes);
		cache[1].comb_states = calloc(sizeof(struct comb_states*),map->num_nodes);
	}
}


struct state* find_state(char choose, int source)
{
	if(cache[(int)choose].states == 0)
		return 0;
	else
		return cache[(int)choose].states[source];
}

void add_state(char choose, struct state* state)
{
	cache[(int)choose].states[state->source] = state;
}


struct state* get_state(map_t* graph, char choose, int source)
{
	init_cache(graph);
	struct state* state = find_state(choose, source);
	
	if(state==0)
	{
		state = calc_dijkstra(choose, source);
		add_state(choose, state);
	}
	
	return state;
}


cost_t* get_all_dists(map_t* graph, char choose, int to)
{
	struct state* state = get_state(graph, choose, to);
	return state->dist;
}

cost_t get_dist(map_t* graph, char choose, int from, int to)
{
	struct state* state = get_state(graph, choose, to);
	return state->dist[from];
}

int* get_prev(map_t* graph, char choose, int to)
{
	struct state* state = get_state(graph, choose, to);
	return state->prev;
}


struct comb_state* init_combined_state(map_t* map, int source)
{
	int sz = map->num_nodes;
	int i;
	struct comb_state* s = calloc(sizeof(struct comb_state), 1);

	s->source = source;
	s->dist = calloc(sizeof(cost_t), sz);
	s->prev = calloc(sizeof(int), sz);
	s->change = calloc(sizeof(char), sz);

	for(i=0; i<sz; i++)
	{
		s->dist[i] = INFINITY;
		s->prev[i] = NO_NODE;
		s->change[i] = UNKNOWN;
	}
	
	return s;
}


int get_switching_point()
{
	return node_by_loc(map, "55-and-woodlawn")->index;
}

struct comb_state* get_cached_combined_state(map_t* map, int source, char choose)
{
	init_cache(map);
	return cache[(int)choose].comb_states[source];
}

void save_combined_state_in_cache(struct comb_state* cs, int choose)
{
	cache[choose].comb_states[cs->source] = cs;
}

struct comb_state* calc_combined_state(map_t* map, char choose, int from);

struct comb_state* get_combined_state(map_t* map, char choose, int from)
{
	struct comb_state* comb_state = get_cached_combined_state(map, from, choose);
	
	if(comb_state != 0)
	{
		return comb_state;
	}
	else
	{
		return calc_combined_state(map, choose, from);
	}
}


struct comb_state* calc_combined_state(map_t* map, char choose, int to)
{
	int switch_point = get_switching_point(map);
	struct state* choose_state = get_state(map, choose, to);
	struct state* switched_state = get_state(map, !choose, to);
	struct state* to_switch_point_state = get_state(map, choose, switch_point);
	
	struct comb_state* state = init_combined_state(map, to);
	cost_t cost_from_switchpoint = get_all_dists(map, !choose, to)[switch_point]; 
	int i;
	int sz = map->num_nodes;
	
	
	if(choose_state->dist[switch_point] <= switched_state->dist[switch_point])
	{
		for(i=0; i<sz; i++)
		{
			state->dist[i] = choose_state->dist[i];
			state->prev[i] = choose_state->prev[i];
			state->change[i] = NO;
		}
	}
	else
	{
		for(i=0; i<sz; i++)
		{
			cost_t new_dist = to_switch_point_state->dist[i] + cost_from_switchpoint;
			
			if(new_dist > choose_state->dist[i])
			{
				state->dist[i] = choose_state->dist[i];
				state->prev[i] = choose_state->prev[i];
				state->change[i] = NO;
			}
			else
			{
				state->dist[i] = new_dist;
				state->prev[i] = to_switch_point_state->prev[i]; 
				state->change[i] = YES;
			}
		}
	}
	save_combined_state_in_cache(state, choose);

	return state;
}


char* get_combined_switchp(map_t* map, char choose, int to)
{
	return get_combined_state(map, choose, to)->change;
}

int* get_combined_prev(map_t* map, char choose, int to)
{
	return get_combined_state(map, choose, to)->prev;
}

cost_t* get_all_combined_dists(map_t* map, char choose, int to)
{
	return get_combined_state(map, choose, to)->dist;
}

cost_t get_combined_dist(map_t* map, char choose, int from, int to)
{
	return get_combined_state(map, choose, to)->dist[from];
}

int
ptype_choose (ptype_t type)
{
    if (type == PTYPE_COP_CAR)
	return CHOOSE_CAR;
    else
	return CHOOSE_FOOT;
}
