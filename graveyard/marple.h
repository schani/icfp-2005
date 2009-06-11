#ifndef __MARPLE_IS_THE_MRS
#define __MARPLE_IS_THE_MRS

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

/* Enters Mrs. Marple */

#include "parser.h"
#include "parser_print.h"
#include "map.h"
#undef  INFINITY
#include "dijkstra.h"
#include "announcer.h"
#include "params.h"

#define INFTY         (INT_MAX>>2)

#define CHOOSE_FROM_PTYPE(ptype)  ((ptype == PTYPE_COP_FOOT) ? CHOOSE_FOOT : CHOOSE_CAR)
#define LOCAL_BRAINSTATE brain_t* brain = (brain_t*) state

typedef struct brain
{
	map_t* map;
	world_skeleton_t* skel;
	char* name;
	int round;
	world_message_t* old_world_msg[NUM_ROUNDS];
	world_message_t* curr_world_msg;
	ptype_t my_ptype;
	int my_pos;
	int my_prev_pos;
	int rounds_ago;
	int num_last;
	int last_pos[200];
} brain_t;


typedef brain_t* (*init_state_t)(brain_t* state, map_t* map, world_skeleton_t* s);
typedef brain_t* (*update_world_t)(brain_t* state, world_message_t* m);
typedef cop_vote_msg_t* (*create_votes_t)(brain_t* state);
typedef int (*next_index_t)(brain_t* state);
typedef ptype_t (*next_ptype_t)(brain_t* state);

typedef struct bot_calls
{
	init_state_t init_state; 
	update_world_t update_world; 
	create_votes_t create_votes;
	next_index_t get_next_position;
	next_ptype_t get_next_ptype;
} bot_calls_t;



int kill(pid_t pid, int sig);

#define STOP kill(getpid(), SIGSTOP)


#endif
