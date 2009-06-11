
#include <stdio.h>

#include "params.h"
#include "parser.h"
#include "map.h"

#define SECRET_CODE 74

typedef struct _cop_knowledge {
	int turn; 
	int prpsize;
	int *prp[MAX_MOVES]; /* Possible robber positions, for each turn. 
				This is a bitfield. */

	int prp_cnt[MAX_MOVES];  /* Number of prps. Caches #of bits set 
                                    in prp. */

	world_skeleton_t *world;
	map_t *map;
} cop_knowledge_t;


  /* Parses world and inits possible positions. */
cop_knowledge_t *cl_init (FILE *file);
void cl_free (cop_knowledge_t *c);

  /* Nonzero when robber may be at position index */
int isset_prp (cop_knowledge_t *c, int index);

  /* Computes all possible robber moves. This advances the internal
     move counter, so call this exactly once by turn. 
     Returns # of possible positions. */ 
int cl_compute_robber_moves (cop_knowledge_t *c);

  /* Call these when robber is smelled, evidence found or robber
     robbed a bank. Updates possible robber position. 

     Call smell_robber even if you do not smell the robber -- it 
     might still restrict prp's. 

     found_evidence may alter history, which also may affect current 
     position in some cases.

     All return number of possible positions. 
   */

int cl_smell_robber (cop_knowledge_t *c, int mapindex, int dist, ptype_t coptype);
int cl_found_evidence (cop_knowledge_t *c, int ago, int index, world_message_t *wm);

int cl_robber_robbed_bank (cop_knowledge_t *c, int mapindex);
int cl_robber_didnt_rob_bank (cop_knowledge_t *c);

/* Iterators for current prp. Return -1 when there are no more prp's. */
int cl_first_prp (cop_knowledge_t *c);
int cl_next_prp_after (cop_knowledge_t *c, int index);

/* Dumps prp to FILE. */
int cl_dump_prp (FILE *file, cop_knowledge_t *c);
int cl_dump_prp_history (FILE *file, cop_knowledge_t *c, int turn);

int cl_draw_knowledge_map (FILE *file, cop_knowledge_t *c, world_message_t *wm); 

/* printf's the possible locations for use by others. Expects 
   opening tags already be sent. */
int cl_inform_others (cop_knowledge_t *c);

/* Takes info from other cops. Currently only trusts info from
   other karin cops. */
int cl_evaluate_information (cop_knowledge_t *c, cop_inform_msg_t *ci);

/* range functions. */
int *alloc_init_range (cop_knowledge_t *c);
void set (int *r, int index);
void unset (int *r, int index);
int isset (int *r, int index);



