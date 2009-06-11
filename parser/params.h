#ifndef __PARAMS_H__
#define __PARAMS_H__

#define NUM_ROBBERS      1
#define NUM_COPS         5

#define MAX_TOKEN_SIZE     101

#define NUM_BANKS          6
#define MAX_BANK_REFUELS   5
#define TOTAL_BANK_MONEY   6000

#define INF_SCORE          99999999

// robber position score parameters
#define BANK_VALUE_SCALE        4
#define BANK_DIST_VALUE_SCALE   3
#define SMELL_PENALTY           1000
#define DEGREE_PENALTY          5000
#define CUL_DE_SAC_PENALTY      15000
#define BANK_REFUEL_PENALTY     5000
#define BANK_EVADE_PENALTY      8000

#define COP_MINUS_2_PENALTY   20000
#define COP_MINUS_1_PENALTY   15000
#define COP_0_PENALTY         10000
#define COP_1_PENALTY         3000
#define COP_2_PENALTY         1000
#define COP_3_PENALTY         300
#define COP_4_PENALTY         100

#define EVASION_TIMEOUT     8

#define MAX_MOVES          200
#define ROBBER_START_POS   "54-and-ridgewood"

// robber recursion control
#define ROBBER_SHORT_RECURSION   5
#define ROBBER_LONG_RECURSION    7
#define ROBBER_MINIMAX_DEPTH     1

#endif
