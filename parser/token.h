#ifndef __TOKEN_H__
#define __TOKEN_H__


struct _token {
	int num;
	int len;
	char *string;
};

typedef struct _token token_t;

typedef enum {
	TOKEN_NAME,		/* name: 	*/
	TOKEN_ROBBER,		/* robber: 	*/
	TOKEN_COP,		/* cop: 	*/
	TOKEN_NODE,		/* nod: 	*/
	TOKEN_EDGE,		/* edg: 	*/
	TOKEN_WORLD,		/* wor: 	*/
	TOKEN_ROBBED,		/* rbd: 	*/
	TOKEN_BANK,		/* bv: 		*/
	TOKEN_EVIDENCE,		/* ev: 		*/
	TOKEN_PLAYER,		/* pl: 		*/
	TOKEN_SMELL,		/* smell: 	*/
	TOKEN_INFORM,		/* inf: 	*/
	TOKEN_PLAN,		/* plan:	*/
	TOKEN_VOTE,		/* vote:	*/
	TOKEN_WINNER,		/* winner:	*/
	TOKEN_NOWINNER,		/* nowinner:	*/
	TOKEN_FROM,		/* from:	*/
	TOKEN_MOVE,		/* mov: 	*/
	TOKEN_REGISTER,		/* reg: 	*/

	TOKEN_GAME_OVER,	/* game-over 	*/

/*	TOKEN_UNKNOWN_BLK = 256,		*/

	TOKEN_NODE_BLK,		/* nod\ 	*/
	TOKEN_NODE_END,		/* nod/ 	*/
	
	TOKEN_EDGE_BLK,		/* edg\ 	*/
	TOKEN_EDGE_END,		/* edg/ 	*/

	TOKEN_WSK_BLK,		/* wsk\ 	*/
	TOKEN_WSK_END,		/* wsk/ 	*/

	TOKEN_WORLD_BLK,	/* wor\ 	*/
	TOKEN_WORLD_END,	/* wor/ 	*/

	TOKEN_BANK_BLK,		/* bv\ 		*/
	TOKEN_BANK_END,		/* bv/ 		*/

	TOKEN_EVIDENCE_BLK,	/* ev\ 		*/
	TOKEN_EVIDENCE_END,	/* ev/ 		*/

	TOKEN_PLAYER_BLK,	/* pl\ 		*/
	TOKEN_PLAYER_END,	/* pl/ 		*/
	
	TOKEN_INFORM_BLK,	/* inf\ 	*/
	TOKEN_INFORM_END,	/* inf/ 	*/

	TOKEN_PLAN_BLK,		/* plan\ 	*/
	TOKEN_PLAN_END,		/* plan/ 	*/

	TOKEN_VOTE_BLK,		/* vote\ 	*/
	TOKEN_VOTE_END,		/* vote/ 	*/

	TOKEN_FROM_BLK,		/* from\ 	*/
	TOKEN_FROM_END,		/* from/ 	*/
	
	start_token_s_size
} start_token_t;

typedef enum {
	PTYPE_COP_FOOT,		/* cop-foot	*/
	PTYPE_COP_CAR,		/* cop-car	*/
	PTYPE_ROBBER,		/* robber	*/

	ptype_s_size
} ptype_t;

typedef enum {
	EDGE_TYPE_FOOT,		/* foot		*/
	EDGE_TYPE_CAR,		/* car		*/

	edge_type_s_size
} edge_type_t;

typedef enum {
	NODE_TAG_HQ,		/* hq		*/
	NODE_TAG_BANK,		/* bank		*/
	NODE_TAG_ROBBER_START,	/* robber-start	*/
	NODE_TAG_ORDINARY,	/* ordinary	*/

	node_tag_s_size
} node_tag_t;


extern 	struct _token start_token_s[start_token_s_size];
extern	struct _token ptype_s[ptype_s_size];
extern	struct _token edge_type_s[edge_type_s_size];
extern	struct _token node_tag_s[node_tag_s_size];


#endif	/* __TOKEN_H__ */
