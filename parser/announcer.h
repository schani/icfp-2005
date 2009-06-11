#ifndef __ANNOUNCER_H__
#define __ANNOUNCER_H__

#include <stdio.h>

#include "token.h"
#include "parser.h"


typedef struct _announcer_t *announcer_t;

announcer_t	init_announcer	(FILE *file);

void		announce_reg 	(announcer_t announcer, char *name, ptype_t type);

void 		announce_move 	(announcer_t announcer, world_skeleton_t *s, world_message_t *m, char *loc, ptype_t type);

void		announce_inform	(announcer_t announcer, cop_inform_msg_t *inform);
void		announce_plan	(announcer_t announcer, cop_plan_msg_t *plan);
void		announce_vote	(announcer_t announcer, cop_vote_msg_t *vote);

#endif
