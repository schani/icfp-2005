/* 
 * reinkommt Resi Berghammer, die oide vom Bullen 
 * enters Resi Mountainhammer, the oldie of the Boole
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "dprintf.h"
#include "parser.h"
#include "parser_print.h"
#include "map.h"
#undef  INFINITY
#include "dijkstra.h"
#include "announcer.h"

#include "resi.h"

extern int dump_scores;



int main (int argc, char** argv)
{
    world_skeleton_t *s;
    map_t *map;
    announcer_t a;
    int i;
    char* name = "resi";


    int move[NUM_COPS];
    brain_t* mcduff[NUM_COPS];
    char* mc_duff_names[NUM_COPS];

    if(argc>1)
    {
	    srand(argv[1][0]);
	    if(argv[1][0]=='1')
	    {
		    dump_scores = 1;
	    }
    }

    a = init_announcer(stdout);
    
    ptype_t t = PTYPE_COP_FOOT;
/*     if(rand()%9>6) */
/*     { */
/* 	    t = PTYPE_COP_CAR; */
/*     } */
    
    announce_reg(a, name, t);


    
    dprintf("GETTING SKEL\n");
    s = parse_world_skeleton(stdin);
    dprintf("GOT SKEL\n");




    //parser_print_world_skeleton(s);

    map = build_map(s);

    dprintf("NAME: %s\n", s->name);


    //////////////////////////////// <RESI>
    for(i=0; i<NUM_COPS; i++ )
    {
	    mcduff[i] = reserl_create_brain(s, map);
	    dprintf("mcduff[%d] = %p\n", i, mcduff[i]);
    }
    //////////////////////////////// </RESI>    

    for(i=0; i<NUM_COPS; i++ )
    {
	    mc_duff_names[i] = s->cops[i];
	    mcduff[i]->name = s->cops[i];
    }
    


    /*
    printf("dist: %d\n", get_dist(map, CHOOSE_FOOT,
				  node_index(map, "55-and-woodlawn"),
				  node_index(map, "54-and-ridgewood")));
    */

/*     dprintf("---- %d  %d\n", */
/* 	    node_by_loc(map, "54-and-ridgewood")->index,  */
/* 	    node_by_loc(map, "54-and-blackstone")->index); */

/*     dprintf("*********** %d\n",  */
/* 	    get_combined_dist(map, CHOOSE_FOOT,  */
/* 			      node_by_loc(map, "54-and-ridgewood")->index,  */
/* 			      node_by_loc(map, "54-and-blackstone")->index)); */
/*     dprintf("*********** %d\n",  */
/* 	    get_combined_dist(map, CHOOSE_FOOT,  */
/* 			      node_by_loc(map, "54-and-blackstone")->index, */
/* 			      node_by_loc(map, "54-and-ridgewood")->index)); */
	    




    for(;;)
    {
	dprintf("entering loop \n");
    
	world_message_t *m;
	cop_inform_msg_t* inform;
	cop_plan_msg_t* other_plans;
	cop_vote_msg_t* votes;
	vote_tally_t* tally;
	cop_plan_msg_t plans[NUM_COPS];

	dprintf("GETTING WORLD\n");
	m = parse_world_message(s);
	dprintf("GOT WORLD\n");
	
	//////////////////////////////// <RESI>
	for(i=0; i<NUM_COPS; i++)
	{
		reserl_update_brain(m, mcduff[i]);
	}
	//////////////////////////////// </RESI>


	announce_inform(a, NULL);

	if(!m->game_running)
		break; 


	dprintf("GETTING inform\n");
	inform = parse_inform_messages(s);
	free_inform_messages(inform);
	dprintf("GOT inform\n");

	

	//////////////////////////////// <RESI>
	for(i=0; i<NUM_COPS; i++)
	{
		move[i] = reserl_get_move(m, mcduff[i]);
	}

	//////////////////////////////// </RESI>
	
	for (i = 0; i<NUM_COPS; i++) {
		dprintf("mcduff name = %s\n", mcduff[i]->name);
	    plans[i].bot = mcduff[i]->name;
	    plans[i].node = node_by_index(map, move[i]);
	    plans[i].type = mcduff[i]->my_ptype;
	    plans[i].world = m->world + 1;
	    plans[i].next = &plans[i+1];
	}
	plans[NUM_COPS-1].next = NULL;


//	dprintf("announcing plan\n");
	announce_plan(a, plans);
	dprintf("GETTING PLAN\n");
	other_plans = parse_plan_messages(s);
	dprintf("GOT PLAN\n");

	free_plan_messages(other_plans);
	



	//////////////////////////////// <RESI>
	votes = reserl_make_stupid_votes(s);
	//////////////////////////////// </RESI>

//	dprintf("announcing vote\n");
	announce_vote(a, votes);
//	dprintf("announced vote\n");
	free(votes);

	dprintf("GETTING vote\n");
	tally = parse_vote_tally(s);
	free_vote_tally(tally);
	dprintf("GOT vote\n");
	
	int k=0;
	int myindex = 0;
	for(k=0; k<NUM_COPS; k++)
	{
		if(0==strcmp(s->name, mcduff[k]->name))
		{
			myindex = k;
			break;
		}
	}

	dprintf("--------------- %d\n", myindex);
	//dprintf("announcing move to %s \n", node_by_index(map, bestdst)->loc);
	announce_move(a, s, m, node_by_index(map, move[myindex])->loc, mcduff[myindex]->my_ptype);
    }
	

    return 0;
}

