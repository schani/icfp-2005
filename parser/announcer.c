
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "announcer.h"
#include "token.h"


struct _announcer_t {
	FILE *output;
};


#define	ARRAY_SIZE(a)	(sizeof(a)/sizeof(token_t))

// #include "token_arrays.c"


static void output_token(announcer_t announcer, token_t *token, int space)
{
	if (space)
		fputc(' ', announcer->output);
	fwrite(token->string, sizeof(char), token->len, announcer->output);
}

static void output_string(announcer_t announcer, char *string, int space)
{
	if (space)
		fputc(' ', announcer->output);
	fputs(string, announcer->output);
}

static void output_number(announcer_t announcer, int number, int space)
{
	if (space)
		fputc(' ', announcer->output);
	fprintf(announcer->output, "%d", number);
}


static void output_eol(announcer_t announcer)
{
	fputc('\n', announcer->output);
    	fflush(announcer->output);
}


static void output_start_token(announcer_t announcer, start_token_t type)
{
	output_token(announcer, &start_token_s[type], 0);
}


static void output_ptype(announcer_t announcer, ptype_t type)
{
	output_token(announcer, &ptype_s[type], 1);
}


announcer_t init_announcer (FILE *file)
{
    	announcer_t new = MALLOC_TYPE(struct _announcer_t);

    	assert(new);
 
    	new->output = file;
    	return new;
}

void	announce_reg (announcer_t announcer, char *name, ptype_t type)
{
	output_start_token(announcer, TOKEN_REGISTER);
	output_string(announcer, name, 1);
	output_ptype(announcer, type);
	output_eol(announcer);
}

void	announce_move (announcer_t announcer, world_skeleton_t *s, world_message_t *m, char *loc, ptype_t type)
{
 	/* FIXME: make sure loc does exist
 	   make sure we have a correct ptype
 	   make sure we actually can travel to loc */

	output_start_token(announcer, TOKEN_MOVE);
	output_string(announcer, loc, 1);
	output_ptype(announcer, type);
	output_eol(announcer);
}


void	announce_inform	(announcer_t announcer, cop_inform_msg_t *inform)
{
	output_start_token(announcer, TOKEN_INFORM_BLK);
	output_eol(announcer);
	
	while (inform) {
		output_start_token(announcer, TOKEN_INFORM);
		output_string(announcer, inform->bot, 1);
		output_string(announcer, inform->node->loc, 1);
		output_ptype(announcer, inform->type);
		output_number(announcer, inform->world, 1);
		output_number(announcer, inform->certainty, 1);
		output_eol(announcer);
		inform = inform->next;
	}
	
	output_start_token(announcer, TOKEN_INFORM_END);
	output_eol(announcer);
}

void	announce_plan	(announcer_t announcer, cop_plan_msg_t *plan)
{
	output_start_token(announcer, TOKEN_PLAN_BLK);
	output_eol(announcer);

	while (plan) {
		output_start_token(announcer, TOKEN_PLAN);
		output_string(announcer, plan->bot, 1);
		output_string(announcer, plan->node->loc, 1);
		output_ptype(announcer, plan->type);
		output_number(announcer, plan->world, 1);
		output_eol(announcer);
		plan = plan->next;	
	}
	
	output_start_token(announcer, TOKEN_PLAN_END);
	output_eol(announcer);
}

void	announce_vote	(announcer_t announcer, cop_vote_msg_t *vote)
{
	output_start_token(announcer, TOKEN_VOTE_BLK);
	output_eol(announcer);

	while (vote) {
		output_start_token(announcer, TOKEN_VOTE);
		output_string(announcer, vote->bot, 1);
		output_eol(announcer);
		vote = vote->next;	
	}
	
	output_start_token(announcer, TOKEN_VOTE_END);
	output_eol(announcer);
}


#ifdef	ANNOUNCER_MAIN

int	main(int argc, char *argv[])
{

	announcer_t announcer;
	
	announcer = init_announcer(stdout);
	
	announce_reg (announcer, "TestName", PTYPE_ROBBER);
	announce_move (announcer, NULL, NULL, "SomeLoc", PTYPE_ROBBER);

	exit(0);
}

#endif
