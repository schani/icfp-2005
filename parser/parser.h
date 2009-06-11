#ifndef __PARSER_H__
#define __PARSER_H__

#include "params.h"
#include "lexer.h"

/* world skeleton */

typedef struct _node_line_t
{
    char *loc;
    int index;
    node_tag_t tag;
    int x, y;
    struct _node_line_t *next;
} node_line_t;

typedef struct _edge_line_t
{
    node_line_t *src_node;
    node_line_t *dst_node;
    edge_type_t type;
    struct _edge_line_t *next;
} edge_line_t;

typedef struct
{
    lexer_t lexer;
    char *name;
    char *robbers[NUM_ROBBERS];
    char *cops[NUM_COPS];
    node_line_t *nodes;
    edge_line_t *edges;
} world_skeleton_t;

/* world message */

typedef struct _bank_value_line_t
{
    node_line_t *node;
    int value;
    struct _bank_value_line_t *next;
} bank_value_line_t;

typedef struct _evidence_line_t
{
    node_line_t *node;
    int world;
    struct _evidence_line_t *next;
} evidence_line_t;

typedef struct _player_line_t
{
    char *bot;
    node_line_t *node;
    ptype_t type;
    struct _player_line_t *next;
} player_line_t;

typedef struct
{
    int game_running;
    int world;
    int loot;
    bank_value_line_t *bank_values;
    evidence_line_t *evidences;
    int smell;
    player_line_t *players;
} world_message_t;

/* vote tally */

typedef struct
{
    int have_winner;
    char *bot;
} vote_tally_t;


/* vote announce */

typedef struct _cop_vote_msg_t
{
    char *bot;
    struct _cop_vote_msg_t *next;
} cop_vote_msg_t;


/* cop messages */

typedef struct _cop_inform_msg_t
{
    char *from_bot;
    char *bot;
    node_line_t *node;
    ptype_t type;
    int world;
    int certainty;
    struct _cop_inform_msg_t *next;
} cop_inform_msg_t;

typedef struct _cop_plan_msg_t
{
    char *from_bot;
    char *bot;
    node_line_t *node;
    ptype_t type;
    int world;
    struct _cop_plan_msg_t *next;
} cop_plan_msg_t;

world_skeleton_t* parse_world_skeleton (FILE *file);

world_message_t* parse_world_message (world_skeleton_t *skeleton);
vote_tally_t* parse_vote_tally (world_skeleton_t *skeleton);
cop_inform_msg_t* parse_inform_messages (world_skeleton_t *skeleton);
cop_plan_msg_t* parse_plan_messages (world_skeleton_t *skeleton);

void free_world_message (world_message_t *message);
void free_vote_tally (vote_tally_t *tally);
void free_inform_messages (cop_inform_msg_t *inform_msg);
void free_plan_messages (cop_plan_msg_t *plan_msg);

#endif
