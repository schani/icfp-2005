
#include <assert.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>

#include "dprintf.h"
#include "coplib.h"

static int cl_draw_knowledge_map_range (FILE *file, cop_knowledge_t *c, world_message_t *wm, int *r);

void dpr (cop_knowledge_t *c, int turn) 
{
	int i;
	dprintf ("size: %d\n", c->prpsize);
	dprintf ("turn: %d\n", turn);
	for  (i=0; i<c->prpsize; i++) {
		dprintf ("%x ", c->prp[turn][i]);
	}
}


int *alloc_init_range (cop_knowledge_t *c)
{
	int *range; 

        range = malloc (c->prpsize * sizeof (int));
        if (!range) {
                exit (1);
        }
        bzero (range, c->prpsize * sizeof (int)); 
	return range; 
}

static void clear_prp_all (cop_knowledge_t *c)
{
	int i;

        for (i=0; i<c->prpsize; i++) {
                c->prp[c->turn-1][i] = 0;
        }
	c->prp_cnt[c->turn-1] = 0;
}

void unset (int *r, int index)
{
	r[index / (sizeof (int)*8)] &= ~(1 << index % (sizeof (int) * 8));
}

void set (int *r, int index)
{
	r[index / (sizeof (int)*8)] |= (1 << index % (sizeof (int) * 8));
}

int isset (int *r, int index)
{
	return ((r[index / (sizeof (int)*8)] & (1 << index % (sizeof (int) * 8))) != 0);
}

static int isset_prp_history (cop_knowledge_t *c, int index, int turn)
{
	return ((c->prp[turn][index / (sizeof (int)*8)] & (1 << index % (sizeof (int) * 8))) != 0);
}

int isset_prp (cop_knowledge_t *c, int index)
{
	return isset_prp_history (c, index, c->turn-1);
}

static int clearset_prp_history (cop_knowledge_t *c, int index, int turn, int do_set)
{
	int old;

	old = isset_prp_history (c, index, turn);

	if (do_set) {
		c->prp[turn][index / (sizeof (int)*8)] |= (1 << index % (sizeof (int) * 8));
	} else {
		c->prp[turn][index / (sizeof (int)*8)] &= ~(1 << index % (sizeof (int) * 8));
	}
	return old;
}

static int set_prp (cop_knowledge_t *c, int index)
{
	return clearset_prp_history (c, index, c->turn-1, 1);
}
	
static int clear_prp (cop_knowledge_t *c, int index)
{
	return clearset_prp_history (c, index, c->turn-1, 0);
}
	
	

  /* Allocates new prp and updates turn. Returns prp. */
static int *new_turn (cop_knowledge_t *c)
{
	c->prp[c->turn] = malloc (c->prpsize*sizeof (int));
	if (!c->prp[c->turn]) {
		return NULL;
	}
	bzero (c->prp[c->turn], c->prpsize*sizeof (int));

	c->turn++;
	return c->prp[c->turn-1];
}

  /* Parses world and inits possible positions. */
cop_knowledge_t *cl_init (FILE *file)
{
	cop_knowledge_t *c;
	node_line_t *startnode;

	c = malloc (sizeof (*c));
	if (!c) return NULL;

	bzero (c, sizeof (*c));

	c->world = parse_world_skeleton (file);
	
	c->map = build_map (c->world);
	if (!c->map) { 
		cl_free (c);
		return NULL;
	}

	c->prpsize = (c->map->num_nodes - 1) / (sizeof (int)*8) + 1;
	if (!new_turn (c)) {
		cl_free (c);
		return NULL;
	}

	startnode = node_by_loc (c->map, ROBBER_START_POS);
	if (!startnode) {
		dprintf ("ALERT: Start pos is not on map!!\n");
		exit (1);
	} 
	set_prp (c, startnode->index);
	c->prp_cnt[c->turn-1] = 1;
	
	return c; 
}


void cl_free (cop_knowledge_t *c)
{	
	int i;
	if (!c) return;	

	for (i=0;i<c->turn;i++) {
		if (c->prp[i]) free (c->prp[i]);
	}
	if (c->world) free (c->world);
	if (c->map) free (c->map);
}

	
/* Iterators for current prp. Return -1 when there are no more prp's. */
int cl_next_prp_after_history (cop_knowledge_t *c, int index, int turn)
{
	for (index++;(index<c->map->num_nodes) && !isset_prp_history (c, index, turn);index++);
	if (index >= c->map->num_nodes) return -1;
	return index;
}

	
int cl_first_prp_history (cop_knowledge_t *c, int turn)
{
	return cl_next_prp_after_history (c, -1, turn);
}

int cl_next_prp_after (cop_knowledge_t *c, int index)
{
	return cl_next_prp_after_history (c, index, c->turn-1);
}

int cl_first_prp (cop_knowledge_t *c)
{
        return cl_next_prp_after_history (c, -1, c->turn-1);
}


  /* Computes all possible robber moves. This advances the internal
     move counter, so call this exactly once by turn.
     Returns # of possible positions. */
int cl_compute_robber_moves (cop_knowledge_t *c) 
{
	int index, other;
	int i;

	assert (c);
        if (!new_turn (c)) {
		assert (0);
        }
	for (i=0; i<c->prpsize; i++) {
		c->prp[c->turn-1][i] = c->prp[c->turn-2][i];
	}
	c->prp_cnt[c->turn-1] = c->prp_cnt[c->turn-2];
	
	for (index = cl_first_prp_history (c, c->turn-2); 
	     index!=-1; 
	     index = cl_next_prp_after_history (c, index, c->turn-2)) {
		for (other = 0; other < c->map->num_nodes; other++) {
			if (c->map->foot_adj[index][other] ||
                            c->map->foot_adj[other][index]) {
				if (!set_prp (c, other)) {
					c->prp_cnt[c->turn-1]++;
				}
			}
		} 
	}

	return c->prp_cnt[c->turn-1];
}
	

int *compute_range (cop_knowledge_t *c, int mapindex, int dist, ptype_t coptype)
{
	int *range;
	int d1, d2;

	range = alloc_init_range (c);

	if (coptype == PTYPE_COP_CAR) {
                switch (dist) {
                case 1:
                        for (d1 = 0; d1 < c->map->num_nodes; d1++) {
                                if (c->map->foot_adj[mapindex][d1] || 
                                    c->map->car_adj[mapindex][d1]) {
					set (range, d1);
                                }
                        }
                        break;

                default:
                        dprintf ("Illegal smell distance for car cop: %d\n", dist);
                        exit (1);
		}
	} else {
                switch (dist) {
                case 1:
                        for (d1 = 0; d1 < c->map->num_nodes; d1++) {
                                if (c->map->foot_adj[mapindex][d1] ||
                                    c->map->foot_adj[d1][mapindex]) {
					set (range, d1);
                                }
                        }
                        break;

                case 2:
                        for (d1 = 0; d1 < c->map->num_nodes; d1++) {
                                for (d2 = 0; d2 < c->map->num_nodes; d2++) {
                                        if (d2 != mapindex && (
                                        (c->map->foot_adj[mapindex][d1] && c->map->foot_adj[d1][d2]) ||
                                        (c->map->foot_adj[mapindex][d1] && c->map->foot_adj[d2][d1]) ||
                                        (c->map->foot_adj[d1][mapindex] && c->map->foot_adj[d1][d2]) ||
                                        (c->map->foot_adj[d1][mapindex] && c->map->foot_adj[d2][d1]))) {
						set (range, d2);
                                        }
                                }
                        }
                        break;

                default:
                        dprintf ("Illegal smell distance for foot cop: %d\n", dist);
                        exit (1);
		}
	}

	return range; 
}

int prp_and (cop_knowledge_t *c, int *r)
{
	int i; 

	for (i=0; i<c->map->num_nodes; i++) {
		if (isset_prp (c, i) && !isset (r, i)) {
			clear_prp (c, i);
			c->prp_cnt[c->turn-1]--;
		}
	}

	return c->prp_cnt[c->turn-1];
}

int prp_without (cop_knowledge_t *c, int *r)
{
	int i; 

	for (i=0; i<c->map->num_nodes; i++) {
		if (isset_prp (c, i) && isset (r, i)) {
			clear_prp (c, i);
			c->prp_cnt[c->turn-1]--;
		}
	}

	return c->prp_cnt[c->turn-1];
}

  /* Call this when robber is smelled, evidence found or robber
     robbed a bank. Updates possible robber position. */
int cl_smell_robber (cop_knowledge_t *c, int mapindex, int dist, ptype_t coptype)
{
	int *smell_range;

//	dprintf ("cl_smell_robber at %d dist %d\n", mapindex, dist);

/* robber cannot occupy cop's pos .. */
	smell_range = alloc_init_range (c);
	set (smell_range, mapindex);
	prp_without (c, smell_range);

	if (coptype == PTYPE_COP_CAR) {
		switch (dist) {
		case 0:
			smell_range = compute_range (c, mapindex, 1, coptype);
			prp_without (c, smell_range);
			free (smell_range);
			break;
		
		case 1: 
			smell_range = compute_range (c, mapindex, dist, coptype);
			prp_and (c, smell_range);
			free (smell_range);
			break;
	
		default: 
			dprintf ("Illegal smell distance for car cop: %d\n", dist);
			exit (1);
		}
	} else {
                switch (dist) {
                case 0:
			smell_range = compute_range (c, mapindex, 1, coptype);
			prp_without (c, smell_range);
			free (smell_range);

			smell_range = compute_range (c, mapindex, 2, coptype);
			prp_without (c, smell_range);
			free (smell_range);

			break;

		case 1:
		case 2:
			smell_range = compute_range (c, mapindex, dist, coptype);
			prp_and (c, smell_range);
			free (smell_range);
			break;

                default:
                        dprintf ("Illegal smell distance for foot cop: %d\n", dist);
                        exit (1);
		}
	}

	return c->prp_cnt[c->turn-1];
}



int cl_found_evidence (cop_knowledge_t *c, int ago, int where, world_message_t *wm)
{
	int *range, *range_new;
	int i, j, index, other;
	int cnt;

	range = alloc_init_range (c);
	range_new = alloc_init_range (c);
	set (range, where);

	if (ago < 0 || ago > 200) {
		dprintf ("******* ALERT: ago is %d\n", ago);
	}
	for (i = 0; i<ago; i++) {
	        for (index=0; index<c->prpsize; index++) {
			range_new[index] = 0;
        	}
	        for (index = 0; index < c->map->num_nodes; index++) {
			if (isset (range, index)) {
				set (range_new, index);
                		for (other = 0; other < c->map->num_nodes; other++) {
		                        if (c->map->foot_adj[index][other] ||
               		                    c->map->foot_adj[other][index]) {
						set (range_new, other);
					}
                                }
                        }
                }
	        for (j=0; j<c->prpsize; j++) {
			range[j] = range_new[j];
        	}
        }

/* dprintf ("************* range map *************\n");
cl_draw_knowledge_map_range (stderr, c, wm, range); */

	cnt = prp_and (c, range);
	free (range);
	free (range_new);

	return cnt;
}

int cl_robber_robbed_bank (cop_knowledge_t *c, int mapindex)
{
	int i; 
	
	for (i=0; i<c->map->num_nodes; i++) {
		if (i!=mapindex) {
			clear_prp (c, i);
		} else {
			set_prp (c, i);
		}
	}
	c->prp_cnt[c->turn-1] = 1;

	return 1;
}

int cl_robber_didnt_rob_bank (cop_knowledge_t *c)
{
	int i;
	node_line_t *node; 

        for (i=0; i<c->map->num_nodes; i++) {
		node = node_by_index (c->map, i);
		if (node->tag == NODE_TAG_BANK) {
			if (clear_prp (c, i)) {
// dprintf ("robber can't be at bank at %s\n", node->loc);
				c->prp_cnt[c->turn-1]--; 
			}
		}
	}

	return c->prp_cnt[c->turn-1];
}

static int cl_draw_knowledge_map_range (FILE *file, cop_knowledge_t *c, world_message_t *wm, int *r) 
{
#define TERM_X 80
#define TERM_Y 25

	static int maxx = -1, maxy; 
	static int scalex, scaley;
	static char map[TERM_X][TERM_Y];
	node_line_t *n;
	player_line_t *p;
	int players;

	int i, j;

	if (maxx==-1) {
		for (i=0; i<c->map->num_nodes; i++) {
			n=node_by_index (c->map, i);
			if (n->x > maxx) maxx = n->x;
			if (n->y > maxy) maxy = n->y;
		}
		scalex = maxx / TERM_X + 1;
		scaley = maxy / TERM_Y + 1;

dprintf ("maxx: %d maxy: %d scalex: %d scaley: %d\n", 
	maxx, maxy, scalex, scaley);
	}

	for (i=0; i<TERM_X; i++) {
		for (j=0; j<TERM_Y; j++) {
			map[i][j] = ' ';
		}
	}

	for (i=0; i<c->map->num_nodes; i++) {
		n=node_by_index (c->map, i);

		if (isset (r, i)) {
			map[n->x / scalex][n->y / scaley] = '?';
		} else {
			players=0;
			for (p=wm->players; p; p=p->next) {
				if (p->node->index == i) {
					players++;
				}
			}
			map[n->x / scalex][n->y / scaley] = players ? players+'0' : '.';
		}
	}

	for (j=0; j<TERM_Y; j++) {
		for (i=0; i<TERM_X; i++) {
			fputc (map[i][j], file);
		}
		fputc ('\n', file);
	}

	return 0;
}

int cl_draw_knowledge_map (FILE *file, cop_knowledge_t *c, world_message_t *wm) 
{
	return cl_draw_knowledge_map_range (file, c, wm, c->prp[c->turn-1]);
}

int cl_dump_prp_history (FILE *file, cop_knowledge_t *c, int turn)
{
	int index; 
	int cnt; 
	node_line_t *node; 

	cnt = 0;
	for (index = 0; index < c->map->num_nodes; index++) {
		if (isset_prp_history (c, index, turn)) {
			if (c->prp_cnt[turn] < 20) {
				node = node_by_index (c->map, index);
				if (c->prp_cnt[turn] > 1) {
					fprintf (file, "%s: Turn %d: Robber possibly at %s(%d).\n", c->world->name, turn, node->loc, index);
				} else {
					fprintf (file, "%s: Turn %d: Robber at %s(%d)!!!\n", c->world->name, turn, node->loc, index);
				}
			}
			cnt++;
		}
	}
	dprintf ("%s: Turn %d %d possible robber positions\n", c->world->name, turn, c->prp_cnt[turn]);
	assert (cnt == c->prp_cnt[turn]); 
	assert (cnt);
	
	return cnt; 
}
	
int cl_dump_prp (FILE *file, cop_knowledge_t *c)
{
	return cl_dump_prp_history (file, c, c->turn-1);
}
	


int cl_inform_others (cop_knowledge_t *c)
{
	int i;
	node_line_t *node;

	for (i=0; i<c->map->num_nodes; i++) {
		if (isset_prp (c, i)) {
			node = node_by_index (c->map, i);
			printf ("inf: %s %s robber %d %d\n", c->world->robbers[0], node->loc, c->turn, SECRET_CODE);
		}
	}

	return 0;
}

int find_cop_by_name (cop_knowledge_t *c, char *s)
{
	int cop;

        for (cop=0; cop<NUM_COPS; cop++) {
		if (strcmp (c->world->cops[cop], s) == 0) {
			return cop;
		}
	}
	dprintf ("*** ALERT unknown cop.\n");
	exit (1);
}

/* !! currectly assumes all karin cops. */
int cl_evaluate_information (cop_knowledge_t *c, cop_inform_msg_t *ci)
{
	int *pos_map[NUM_COPS];
	int is_karin_cop[NUM_COPS];
	int cop;
	int this_cop; 
	
	for (cop=0; cop<NUM_COPS; cop++) {
		is_karin_cop[cop] = 0;
		pos_map[cop] = alloc_init_range (c);
	}
	this_cop = find_cop_by_name (c, c->world->name);

	for (; ci; ci=ci->next) {
		if (strcmp (ci->bot, c->world->robbers[0]) == 0) {
			if (ci->certainty == SECRET_CODE) {
/* It is *most likely* a karin cop's info. */
				cop = find_cop_by_name (c, ci->from_bot);
				set (pos_map[cop], ci->node->index);	
				is_karin_cop[cop] = 1;
			}
		}
	}

        for (cop=0; cop<NUM_COPS; cop++) {
		if (is_karin_cop[cop]) {
			prp_and (c, pos_map[cop]);
		}
		free (pos_map[cop]);
	}

	return c->prp_cnt[c->turn-1];
}
	



