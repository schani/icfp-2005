
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "utils.h"
#include "lexer.h"
#include "token.h"


#define	MAXLINE		1024


struct _lexer_t {
	FILE *input;
	char buffer[MAXLINE];
	int index;
	int length;	
};

typedef struct _token *token_array_t;



#define	ARRAY_SIZE(a)	(a ## _size)

// #include "token_arrays.c"


/*
**	read one line of input (including newline)
*/
static inline void read_line (lexer_t lexer)
{
	char *ret = fgets(lexer->buffer, MAXLINE, lexer->input);
	
	assert(ret);
	lexer->length = strlen(lexer->buffer);
	lexer->index = 0;
}


/*
**	skip over whitespace
**	currently ' ' and '\t'
*/
static inline void skip_space (lexer_t lexer)
{
	char *pos = &lexer->buffer[lexer->index];
	int left = lexer->length - lexer->index;

	if (left && ((*pos == ' ') || (*pos == '\t')))
		lexer->index++;
}


/*
**	ensure that the lexer has valid input
**	(i.e. at least one char in buffer)
**	skip over space
*/
static inline void advance (lexer_t lexer)
{
	if (lexer->index == lexer->length)
		read_line (lexer);
	else
		skip_space (lexer);
		
	assert (lexer->index < lexer->length);
}



/*
**	consume a given token
**	do not skip over whitespace 
*/
static void consume_token_noskip (lexer_t lexer, token_t *token)
{
	char *pos = &lexer->buffer[lexer->index];
	int left = lexer->length - lexer->index;
	int len = token->len;

	assert(left >= len);
	assert(strncmp(pos, token->string, len) == 0);
	
	lexer->index += len;
}

/*
**	consume a given token
**	optional skip over whitespace 
*/
static void consume_token (lexer_t lexer, token_t *token, int skip)
{
	if (skip)
		skip_space(lexer);
	consume_token_noskip(lexer, token);
}


/*
**	check for token from token array
*/
static token_t *find_token (lexer_t lexer, token_array_t array, int size)
{
	int i, left;
	char *pos;
	
	advance(lexer);
	pos = &lexer->buffer[lexer->index];
	left = lexer->length - lexer->index;
	
	for (i=0; i<size; i++) {
		token_t *token = &array[i];
		int len = token->len;
		
		if (!len)
			continue;

		if (strncmp(token->string, pos, min(left, len)) == 0)
			return token;
	}
	return NULL;
}


/*
**	check for token from token array
**	optional skip over whitespace and
**	consume token
*/
static inline int find_and_consume(lexer_t lexer, token_array_t array, int size, int skip)
{
	token_t *elem = find_token(lexer, array, size);

	assert(elem);
	consume_token(lexer, elem, skip);
	return elem->num;
}






lexer_t	init_lexer (FILE *file)
{
	lexer_t new = MALLOC_TYPE(struct _lexer_t);
	
	assert(new);
	
	new->input = file;
	new->index = 0;
	new->length = 0;
	return new;
}






void	scan_expect (lexer_t lexer, start_token_t token)
{
	advance(lexer);
	consume_token(lexer, &start_token_s[token], 1);
}

void	scan_expect_eol	(lexer_t lexer)
{
	static token_t crnl = 	{ .string = "\r\n", .len = 2 };
	static token_t nl = 	{ .string = "\n",   .len = 1 };
	
	char *pos;
	
	advance(lexer);
	pos = &lexer->buffer[lexer->index];

	/* this is based on the assumption that there
	   might be a whitespace after the last token
	   right before the newline (unverified) */
	skip_space (lexer);

	assert(lexer->index < lexer->length);
	if (*pos == '\r')
		consume_token_noskip (lexer, &crnl);
	else
		consume_token_noskip (lexer, &nl);
	
	assert(lexer->index == lexer->length);
}

void	scan_expect_with_eol (lexer_t lexer, start_token_t token)
{
	scan_expect (lexer, token);
	scan_expect_eol	(lexer);
}


void	scan_name (lexer_t lexer, char *dest)
{
	char *pos;
	
	skip_space(lexer);
	pos = &lexer->buffer[lexer->index];
	
	assert(lexer->index < lexer->length);

	while (lexer->index < lexer->length) {
		switch (*pos) {
		case '-':
		case 'a' ... 'z':
		case 'A' ... 'Z':
		case '0' ... '9':
		case '_':
		case '#':
		case '(':
		case ')':
			*dest++ = *pos++;
			lexer->index++;
			break;

		default:
			goto exit;
		}
	}
exit:
	*dest = '\0';
	return;
}


int	scan_number_noskip (lexer_t lexer)
{
	char *pos = &lexer->buffer[lexer->index];
	int number = 0;
	
	assert(lexer->index < lexer->length);

	while (lexer->index < lexer->length) {
		switch (*pos) {
		case '0' ... '9':
			number = (number * 10) + (*pos++ - '0');
			lexer->index++;
			break;

		default:
			goto exit;
		}
	}
exit:
	return number;		
}


int	scan_pos_number (lexer_t lexer)
{
	skip_space(lexer);
	return scan_number_noskip(lexer);		
}

int	scan_number (lexer_t lexer)
{
	static token_t neg = 	{ .string = "-", .len = 1 };
	int number = 0, sign = 1;
	char *pos;

	skip_space(lexer);
	pos = &lexer->buffer[lexer->index];
	
	assert(lexer->index < lexer->length);
	
	if (*pos == '-') {
		consume_token_noskip (lexer, &neg);
		sign = -1;
	}
	number = scan_number_noskip (lexer);
	return number * sign;
}



start_token_t scan_start (lexer_t lexer)
{
	return (start_token_t)find_and_consume(lexer, 
		start_token_s, ARRAY_SIZE(start_token_s), 0);
}


ptype_t	scan_ptype (lexer_t lexer)
{
	return (ptype_t)find_and_consume(lexer, 
		ptype_s, ARRAY_SIZE(ptype_s), 1);
}	


edge_type_t scan_edge_type (lexer_t lexer)
{
	return (edge_type_t)find_and_consume(lexer, 
		edge_type_s, ARRAY_SIZE(edge_type_s), 1);
}	


node_tag_t scan_node_tag (lexer_t lexer)
{
	return (node_tag_t)find_and_consume(lexer, 
		node_tag_s, ARRAY_SIZE(node_tag_s), 1);
}	
	



#ifdef	LEXER_MAIN

int	main(int argc, char *argv[])
{

	lexer_t lexer;
	int token, number;
	char buffer[256];
	
	lexer = init_lexer(stdin);
	
	while (1) {
		token = scan_start(lexer);
		printf("scan_start: %d [%s]\n", token, start_token_s[token].string);
		number = scan_pos_number(lexer);
		printf("scan_pos_number: %d\n", number);
		number = scan_number(lexer);
		printf("scan_number: %d\n", number);
		scan_name(lexer, buffer);
		printf("scan_name: »%s«\n", buffer);
		token = scan_ptype(lexer);
		printf("scan_ptype: %d [%s]\n", token, ptype_s[token].string);
		token = scan_edge_type(lexer);
		printf("scan_edge_type: %d [%s]\n", token, edge_type_s[token].string);
		token = scan_node_tag(lexer);
		printf("scan_node_tag: %d [%s]\n", token, node_tag_s[token].string);
		scan_expect_eol(lexer);
	}


}

#endif
