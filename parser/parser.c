#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "parser.h"
#include "params.h"

static node_line_t*
find_node_with_name (world_skeleton_t *s, char *name)
{
    node_line_t *n;

    for (n = s->nodes; n != 0; n = n->next)
	if (strcmp(name, n->loc) == 0)
	    return n;

    assert(0);

    return 0;
}

static char*
find_bot_name (world_skeleton_t *s, char *name)
{
    int i;

    for (i = 0; i < NUM_ROBBERS; ++i)
	if (strcmp(name, s->robbers[i]) == 0)
	    return s->robbers[i];

    for (i = 0; i < NUM_COPS; ++i)
	if (strcmp(name, s->cops[i]) == 0)
	    return s->cops[i];

    assert(0);

    return 0;
}

static node_line_t*
scan_node (world_skeleton_t *s)
{
    char buf[MAX_TOKEN_SIZE];

    scan_name(s->lexer, buf);
    return find_node_with_name(s, buf);
}

static char*
scan_bot (world_skeleton_t *s)
{
    char buf[MAX_TOKEN_SIZE];

    scan_name(s->lexer, buf);
    return find_bot_name(s, buf);
}

world_skeleton_t*
parse_world_skeleton (FILE *file)
{
    world_skeleton_t *s = MALLOC_TYPE(world_skeleton_t);
    char buf[MAX_TOKEN_SIZE];
    int i;
    node_line_t *n;

    s->lexer = init_lexer(file);

    scan_expect_with_eol(s->lexer, TOKEN_WSK_BLK);

    scan_expect(s->lexer, TOKEN_NAME);
    scan_name(s->lexer, buf);
    s->name = xstrdup(buf);
    scan_expect_eol(s->lexer);

    for (i = 0; i < NUM_ROBBERS; ++i)
    {
	scan_expect(s->lexer, TOKEN_ROBBER);
	scan_name(s->lexer, buf);
	s->robbers[i] = xstrdup(buf);
	scan_expect_eol(s->lexer);
    }

    for (i = 0; i < NUM_COPS; ++i)
    {
	scan_expect(s->lexer, TOKEN_COP);
	scan_name(s->lexer, buf);
	s->cops[i] = xstrdup(buf);
	scan_expect_eol(s->lexer);
    }

    scan_expect_with_eol(s->lexer, TOKEN_NODE_BLK);

    s->nodes = 0;
    for (;;)
    {
	start_token_t token = scan_start(s->lexer);

	if (token == TOKEN_NODE_END)
	{
	    scan_expect_eol(s->lexer);
	    break;
	}

	assert(token == TOKEN_NODE);

	n = MALLOC_TYPE(node_line_t);

	scan_name(s->lexer, buf);
	n->loc = xstrdup(buf);

	n->tag = scan_node_tag(s->lexer);
	n->x = scan_pos_number(s->lexer);
	n->y = scan_pos_number(s->lexer);

	scan_expect_eol(s->lexer);

	n->next = s->nodes;
	s->nodes = n;
    }

    scan_expect_with_eol(s->lexer, TOKEN_EDGE_BLK);

    /* number nodes */
    i = 0;
    for (n = s->nodes; n != 0; n = n->next)
	n->index = i++;

    s->edges = 0;
    for (;;)
    {
	edge_line_t *e;
	start_token_t token = scan_start(s->lexer);

	if (token == TOKEN_EDGE_END)
	{
	    scan_expect_eol(s->lexer);
	    break;
	}

	assert(token == TOKEN_EDGE);

	e = MALLOC_TYPE(edge_line_t);

	e->src_node = scan_node(s);
	e->dst_node = scan_node(s);
	e->type = scan_edge_type(s->lexer);

	scan_expect_eol(s->lexer);

	e->next = s->edges;
	s->edges = e;
    }

    scan_expect_with_eol(s->lexer, TOKEN_WSK_END);

    return s;
}

world_message_t*
parse_world_message (world_skeleton_t *s)
{
    world_message_t *m = MALLOC_TYPE(world_message_t);
    start_token_t token;

    token = scan_start(s->lexer);

    if (token == TOKEN_GAME_OVER)
    {
	m->game_running = 0;
	return m;
    }

    assert(token == TOKEN_WORLD_BLK);
    scan_expect_eol(s->lexer);

    m->game_running = 1;

    scan_expect(s->lexer, TOKEN_WORLD);
    m->world = scan_pos_number(s->lexer);
    scan_expect_eol(s->lexer);

    scan_expect(s->lexer, TOKEN_ROBBED);
    m->loot = scan_pos_number(s->lexer);
    scan_expect_eol(s->lexer);

    scan_expect_with_eol(s->lexer, TOKEN_BANK_BLK);

    m->bank_values = 0;
    for (;;)
    {
	bank_value_line_t *bv;

	token = scan_start(s->lexer);

	if (token == TOKEN_BANK_END)
	{
	    scan_expect_eol(s->lexer);
	    break;
	}

	assert(token == TOKEN_BANK);

	bv = MALLOC_TYPE(bank_value_line_t);

	bv->node = scan_node(s);
	bv->value = scan_pos_number(s->lexer);

	scan_expect_eol(s->lexer);

	bv->next = m->bank_values;
	m->bank_values = bv;
    }

    scan_expect_with_eol(s->lexer, TOKEN_EVIDENCE_BLK);

    m->evidences = 0;
    for (;;)
    {
	evidence_line_t *el;

	token = scan_start(s->lexer);

	if (token == TOKEN_EVIDENCE_END)
	{
	    scan_expect_eol(s->lexer);
	    break;
	}

	assert(token == TOKEN_EVIDENCE);

	el = MALLOC_TYPE(evidence_line_t);

	el->node = scan_node(s);
	el->world = scan_pos_number(s->lexer);

	scan_expect_eol(s->lexer);

	el->next = m->evidences;
	m->evidences = el;
    }

    scan_expect(s->lexer, TOKEN_SMELL);
    m->smell = scan_pos_number(s->lexer);
    scan_expect_eol(s->lexer);

    scan_expect_with_eol(s->lexer, TOKEN_PLAYER_BLK);

    m->players = 0;
    for (;;)
    {
	player_line_t *pl;

	token = scan_start(s->lexer);

	if (token == TOKEN_PLAYER_END)
	{
	    scan_expect_eol(s->lexer);
	    break;
	}

	pl = MALLOC_TYPE(player_line_t);

	pl->bot = scan_bot(s);
	pl->node = scan_node(s);
	pl->type = scan_ptype(s->lexer);

	scan_expect_eol(s->lexer);

	pl->next = m->players;
	m->players = pl;
    }

    scan_expect_with_eol(s->lexer, TOKEN_WORLD_END);

    return m;
}

vote_tally_t*
parse_vote_tally (world_skeleton_t *s)
{
    vote_tally_t *t = MALLOC_TYPE(vote_tally_t);
    start_token_t token = scan_start(s->lexer);

    if (token == TOKEN_NOWINNER)
	t->have_winner = 0;
    else
    {
	assert(token == TOKEN_WINNER);

	t->have_winner = 1;
	t->bot = scan_bot(s);
    }

    scan_expect_eol(s->lexer);

    return t;
}

cop_inform_msg_t*
parse_inform_messages (world_skeleton_t *s)
{
    cop_inform_msg_t *messages = 0;

    scan_expect_with_eol(s->lexer, TOKEN_FROM_BLK);

    for (;;)
    {
	start_token_t token = scan_start(s->lexer);
	char *from_bot;

	if (token == TOKEN_FROM_END)
	{
	    scan_expect_eol(s->lexer);
	    break;
	}

	from_bot = scan_bot(s);

	scan_expect_eol(s->lexer);

	scan_expect_with_eol(s->lexer, TOKEN_INFORM_BLK);

	for (;;)
	{
	    cop_inform_msg_t *i;

	    token = scan_start(s->lexer);

	    if (token == TOKEN_INFORM_END)
	    {
		scan_expect_eol(s->lexer);
		break;
	    }

	    assert(token == TOKEN_INFORM);

	    i = MALLOC_TYPE(cop_inform_msg_t);

	    i->from_bot = from_bot;
	    i->bot = scan_bot(s);
	    i->node = scan_node(s);
	    i->type = scan_ptype(s->lexer);
	    i->world = scan_pos_number(s->lexer);
	    i->certainty = scan_number(s->lexer);

	    scan_expect_eol(s->lexer);

	    i->next = messages;
	    messages = i;
	}
    }

    return messages;
}

cop_plan_msg_t*
parse_plan_messages (world_skeleton_t *s)
{
    cop_plan_msg_t *messages = 0;

    scan_expect_with_eol(s->lexer, TOKEN_FROM_BLK);

    for (;;)
    {
	start_token_t token = scan_start(s->lexer);
	char *from_bot;

	if (token == TOKEN_FROM_END)
	{
	    scan_expect_eol(s->lexer);
	    break;
	}

	from_bot = scan_bot(s);

	scan_expect_eol(s->lexer);

	scan_expect_with_eol(s->lexer, TOKEN_PLAN_BLK);

	for (;;)
	{
	    cop_plan_msg_t *p;

	    token = scan_start(s->lexer);

	    if (token == TOKEN_PLAN_END)
	    {
		scan_expect_eol(s->lexer);
		break;
	    }

	    assert(token == TOKEN_PLAN);

	    p = MALLOC_TYPE(cop_plan_msg_t);

	    p->from_bot = from_bot;
	    p->bot = scan_bot(s);
	    p->node = scan_node(s);
	    p->type = scan_ptype(s->lexer);
	    p->world = scan_pos_number(s->lexer);

	    scan_expect_eol(s->lexer);

	    p->next = messages;
	    messages = p;
	}
    }

    return messages;
}

static void
free_bank_value_lines (bank_value_line_t *l)
{
    while (l != 0)
    {
	bank_value_line_t *next = l->next;

	free(l);
	l = next;
    }
}

static void
free_evidence_lines (evidence_line_t *l)
{
    while (l != 0)
    {
	evidence_line_t *next = l->next;

	free(l);
	l = next;
    }
}

static void
free_player_lines (player_line_t *l)
{
    while (l != 0)
    {
	player_line_t *next = l->next;

	free(l);
	l = next;
    }
}

void
free_world_message (world_message_t *message)
{
    if (message->game_running)
    {
	free_bank_value_lines(message->bank_values);
	free_evidence_lines(message->evidences);
	free_player_lines(message->players);
    }

    free(message);
}

void
free_vote_tally (vote_tally_t *tally)
{
    free(tally);
}

void
free_inform_messages (cop_inform_msg_t *m)
{
    while (m != 0)
    {
	cop_inform_msg_t *next = m->next;

	free(m);
	m = next;
    }
}

void
free_plan_messages (cop_plan_msg_t *m)
{
    while (m != 0)
    {
	cop_plan_msg_t *next = m->next;

	free(m);
	m = next;
    }
}
