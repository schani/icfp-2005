#include <stdio.h>
#include <string.h>

#include "parser_print.h"

const char* txt_ptype_t[] = 
  {"PTYPE_UNKNOWN","PTYPE_COP_FOOT", "PTYPE_COP_CAR", "PTYPE_ROBBER"};
const char* txt_node_tag[] =
  {"NODE_TAG_UNKNOWN", "NODE_TAG_HQ", "NODE_TAG_BANK", "NODE_TAG_ROBBER_START", 
   "NODE_TAG_ORDINARY"};
const char* txt_edge_type[] =
  {"EDGE_TYPE_UNKNOWN", "EDGE_TYPE_FOOT", "EDGE_TYPE_CAR"};

void 
parser_print_world_skeleton (world_skeleton_t* s)
{
  int i;
  if (s) {
    printf ("World_Skeleton:   name %s\n", s->name);
    for (i=0; i<NUM_ROBBERS; i++)
      printf ("\trobber%d: %s\n", i, s->robbers[i]);
    for (i=0; i<NUM_COPS; i++)
      printf ("\tcop%d: %s\n", i, s->cops[i]);
    parser_print_node_line(s->nodes);
    parser_print_edge_line(s->edges);
  }
}

void 
parser_print_world_message (world_message_t* m)
{
  if (m) {
    printf ("World_Message: game_running %d  world %d  loot %d  smell %d\n",
	    m->game_running,
	    m->world,
	    m->loot,
	    m->smell);
    parser_print_bank_value_line(m->bank_values);
    parser_print_evidence_line(m->evidences);
    parser_print_player_line(m->players);
  }
}

void 
parser_print_vote_tally (vote_tally_t* t)
{
  if (t)
    printf ("Vote_Tally: have_winner %d  bot %s\n", t->have_winner, t->bot);
}

void 
parser_print_inform_message (cop_inform_msg_t* m)
{
  if (m) {
    printf ("Inform_Message:\n \
           \t from_bot %s\n    \
           \t bot %s\n	       \
           \t loc %s\n	       \
           \t type %s\n				\
           \t world %d  certainty %d\n",
	    m->from_bot, m->bot, m->node->loc, txt_ptype_t[m->type], m->world, m->certainty);
    if (m->next)
      parser_print_inform_message(m->next);
  }
}

void 
parser_print_plan_message (cop_plan_msg_t* m)
{
  if (m) {
    printf ("Plan_Message:\n \
           \t from_bot %s\n  \
           \t bot %s\n	     \
           \t loc %s\n	     \
           \t type %s\n	     \
           \t world %d\n",
	    m->from_bot, m->bot, m->node->loc, txt_ptype_t[m->type], m->world);
    if (m->next)
      parser_print_plan_message(m->next);
  }
}

void 
parser_print_bank_value_line (bank_value_line_t* v)
{
  if (v) {
    printf ("Bank_Value_Line: loc %s  value %d\n", v->node->loc, v->value);
    if (v->next)
      parser_print_bank_value_line(v->next);
  }
}

void 
parser_print_evidence_line (evidence_line_t* e)
{
  if (e) {
    printf ("Evidence_Line: loc %s  world %d\n", e->node->loc, e->world);
    if (e->next)
      parser_print_evidence_line(e->next); 
  }
}

void 
parser_print_player_line (player_line_t* p)
{
  if (p) {
    printf ("Player_Line:\n \
          \t bot %s\n	    \
          \t loc %s\n	    \
          \t type %s\n",
	    p->bot, p->node->loc, txt_ptype_t[p->type]);
    if (p->next)
      parser_print_player_line(p->next);
  }
}

void 
parser_print_node_line (node_line_t* n)
{
  if (n) {
    printf ("Node_Line:\n loc:%s  tag:%s x:%d y:%d\n",
	    n->loc, txt_node_tag[n->tag], n->x, n->y);
    if (n->next)
      parser_print_node_line(n->next);
  }
}

void 
parser_print_edge_line (edge_line_t* e)
{
  if (e) {
    printf ("Edge_Line:\n src_loc:%s  dst_loc:%s  type:%s\n", 
	    e->src_node->loc, e->dst_node->loc, txt_edge_type[e->type]);
    if (e->next)
      parser_print_edge_line(e->next);
  }
}
