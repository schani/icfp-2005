#ifndef RESI_H
#define RESI_H

typedef struct {
	// what do we today ? 
	// the same thing we do every night.
	// we will conquer the world.
	int rounds_ago;
	ptype_t my_ptype;
	int my_pos;
	int my_prev_pos;
	char* name;
	map_t* map;
	int num_nodes;
	int dynamic_cutoff;
	int* mr_proper;
	world_skeleton_t* skel;
} brain_t;


extern brain_t* 
reserl_create_brain(world_skeleton_t* s, map_t* map);


extern cop_vote_msg_t* 
reserl_make_stupid_votes(world_skeleton_t* s);

void reserl_update_brain(world_message_t* m, brain_t* brain);

int 
reserl_get_move(world_message_t* m, brain_t* brain);
#endif
