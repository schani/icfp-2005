#ifndef __PARSER_PRINT_H__
#define __PARSER_PRINT_H__

#include "parser.h"

void parser_print_world_skeleton (world_skeleton_t* s);
void parser_print_world_message (world_message_t* m);
void parser_print_vote_tally (vote_tally_t* t);
void parser_print_inform_message (cop_inform_msg_t* m);
void parser_print_plan_message (cop_plan_msg_t* m);
void parser_print_bank_value_line (bank_value_line_t* v);
void parser_print_evidence_line (evidence_line_t* e);
void parser_print_player_line (player_line_t* p);
void parser_print_node_line (node_line_t* n);
void parser_print_edge_line (edge_line_t* e);

#endif
