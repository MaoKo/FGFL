#ifndef _FGFX_LEX_H_
#define _FGFX_LEX_H_

#include <stdint.h>

typedef	uint8_t	dfa_state_t;
typedef	uint8_t	final_state_t;

enum {
	T_SPACE,
	T_COMMENT,
	T_LOCAL_TOK,
	T_EQUAL,
	T_REGEX,
	T_NTER,
	T_ARROW,
	T_UNION,
	T_LITERAL,
	T_DIRECTIVE,
	T_GLOBAL_TOK,
	T_BARROW,
	T_SEMI,
	T_COMMA,
	T_LBRACE,
	T_RBRACE,
	T_LBRACK,
	T_RBRACK,
	T_LPAREN,
	T_RPAREN,
	T_SKIP,
	T_TOKEN,
	T_KEYWORD,
	T_IGCASE,
	T_EXTERN,
	T_PRODUCTION,
	T_ALIAS,
	T_START,
	T_PRECEDENCE,
	T_EMPTY,
	T_LEFT,
	T_RIGHT,
	T_NONASSOC,
	T_MIMIC,
	T_ERROR,
	T_EOF,
};

#define TOTAL_TOKEN	36

#ifndef _ONLY_TOKEN_

#define START_STATE	1
#define DEAD_STATE	0

static dfa_state_t
fgfx_state_table[43][256] = {
/*   0 */	{},
/*   1 */	{[125]=19, [124]=18, [123]=17, [97 ... 122]=14, [95]=14, [93]=16, [91]=15, [65 ... 90]=14, [64]=13, [61]=12, [60]=11, [59]=10, [47]=9, [45]=8, [44]=7, [41]=6, [40]=5, [39]=4, [36]=3, [32]=2, [9 ... 10]=2},
/*   2 */	{[32]=2, [9 ... 10]=2},
/*   3 */	{[97 ... 122]=20, [95]=20, [65 ... 90]=20},
/*   4 */	{[93 ... 255]=21, [92]=22, [40 ... 91]=21, [35 ... 38]=21, [11 ... 33]=21, [0 ... 9]=21},
/*   5 */	{},
/*   6 */	{},
/*   7 */	{},
/*   8 */	{[62]=23},
/*   9 */	{[93 ... 255]=24, [92]=27, [48 ... 91]=24, [47]=26, [43 ... 46]=24, [42]=25, [11 ... 41]=24, [0 ... 9]=24},
/*  10 */	{},
/*  11 */	{[97 ... 122]=28, [95]=28, [65 ... 90]=28},
/*  12 */	{[62]=29},
/*  13 */	{[97 ... 122]=30, [95]=30, [65 ... 90]=30},
/*  14 */	{[97 ... 122]=14, [95]=14, [65 ... 90]=14, [48 ... 57]=14},
/*  15 */	{},
/*  16 */	{},
/*  17 */	{},
/*  18 */	{},
/*  19 */	{},
/*  20 */	{[97 ... 122]=20, [95]=20, [65 ... 90]=20, [48 ... 57]=20},
/*  21 */	{[93 ... 255]=21, [92]=22, [40 ... 91]=21, [39]=31, [35 ... 38]=21, [11 ... 33]=21, [0 ... 9]=21},
/*  22 */	{[92]=21, [39]=21, [34]=21},
/*  23 */	{},
/*  24 */	{[93 ... 255]=24, [92]=27, [48 ... 91]=24, [47]=32, [11 ... 46]=24, [0 ... 9]=24},
/*  25 */	{[93 ... 255]=25, [92]=36, [48 ... 91]=25, [47]=35, [43 ... 46]=25, [42]=34, [11 ... 41]=25, [10]=33, [0 ... 9]=25},
/*  26 */	{[11 ... 255]=26, [0 ... 9]=26},
/*  27 */	{[0 ... 255]=24},
/*  28 */	{[97 ... 122]=28, [95]=28, [65 ... 90]=28, [62]=38, [48 ... 57]=28, [39]=37},
/*  29 */	{},
/*  30 */	{[97 ... 122]=30, [95]=30, [65 ... 90]=30, [48 ... 57]=30},
/*  31 */	{},
/*  32 */	{},
/*  33 */	{[43 ... 255]=33, [42]=39, [0 ... 41]=33},
/*  34 */	{[93 ... 255]=25, [92]=36, [48 ... 91]=25, [47]=40, [43 ... 46]=25, [42]=34, [11 ... 41]=25, [10]=33, [0 ... 9]=25},
/*  35 */	{[43 ... 255]=33, [42]=39, [0 ... 41]=33},
/*  36 */	{[43 ... 255]=25, [42]=34, [0 ... 41]=25},
/*  37 */	{[62]=38, [39]=41},
/*  38 */	{},
/*  39 */	{[48 ... 255]=33, [47]=40, [43 ... 46]=33, [42]=39, [0 ... 41]=33},
/*  40 */	{},
/*  41 */	{[62]=38, [39]=42},
/*  42 */	{[62]=38},
};

#define SIZE_FINAL_TAB	22

static final_state_t
fgfx_final_table[SIZE_FINAL_TAB][2] = {
	{ 2, 	T_SPACE },
	{ 5, 	T_LPAREN },
	{ 6, 	T_RPAREN },
	{ 7, 	T_COMMA },
	{ 10, 	T_SEMI },
	{ 12, 	T_EQUAL },
	{ 14, 	T_GLOBAL_TOK },
	{ 15, 	T_LBRACK },
	{ 16, 	T_RBRACK },
	{ 17, 	T_LBRACE },
	{ 18, 	T_UNION },
	{ 19, 	T_RBRACE },
	{ 20, 	T_DIRECTIVE },
	{ 23, 	T_ARROW },
	{ 26, 	T_COMMENT },
	{ 29, 	T_BARROW },
	{ 30, 	T_LOCAL_TOK },
	{ 31, 	T_LITERAL },
	{ 32, 	T_REGEX },
	{ 35, 	T_REGEX },
	{ 38, 	T_NTER },
	{ 40, 	T_COMMENT },
};

static int
fgfx_skip_table[3] = {
	T_SPACE,
	T_COMMENT,
	-1,
};

#endif /* _ONLY_TOKEN_ */
#endif /* _FGFX_LEX_H_ */
