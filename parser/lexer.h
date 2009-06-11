#ifndef __LEXER_H__
#define __LEXER_H__

#include <stdio.h>

#include "token.h"


typedef struct _lexer_t *lexer_t;

lexer_t		init_lexer	(FILE *);

start_token_t	scan_start	(lexer_t);
void		scan_expect	(lexer_t, start_token_t);
void		scan_expect_eol	(lexer_t);
void		scan_expect_with_eol (lexer_t, start_token_t);

void		scan_name 	(lexer_t, char *);
int		scan_pos_number (lexer_t);
int		scan_number 	(lexer_t);

ptype_t		scan_ptype	(lexer_t);
edge_type_t	scan_edge_type	(lexer_t);
node_tag_t	scan_node_tag	(lexer_t);

#endif	/* __LEXER_H__ */
