
#include "parser.h"

typedef void *columbo_t;

columbo_t *columbo_init (world_skelleton_t *w);
columbo_t *columbo_new_world (columbo_t *c, world_message_t *wm);
char *columbo_move (columbo_t *c, world_skelleton_t *w);

