#ifndef _FGFX_LEX_H_
#define _FGFX_LEX_H_

#include <stdint.h>
#include <stdbool.h>

enum {
	DUMMY_STATE,
	S_GLOBAL,
	S_SEEN_REGEX,
};

#define TOTAL_STATE	3
#define INIT_STATE	S_GLOBAL

enum {
	T_EQUAL,
	T_STAR,
	T_REGEX,
	T_NON_TERMINAL,
	T_LITERAL,
	T_UNION,
	T_LBRACK,
	T_RBRACK,
	T_DIRECTIVE,
	T_TERMINAL,
	T_ARROW,
	T_BARROW,
	T_SEMI,
	T_COMMA,
	T_LBRACE,
	T_RBRACE,
	T_LPAREN,
	T_RPAREN,
	T_BEG_REGEX,
	T_END_REGEX,
	T_SPACE,
	T_COMMENT,
	T_SKIP,
	T_TOKEN,
	T_KEYWORD,
	T_IGCASE,
	T_STATE,
	T_BEGIN,
	T_FRAG,
	T_INITIAL,
	T_ALL,
	T_NONE,
	T_REJECT,
	T_EXTERN,
	T_PRODUCTION,
	T_ALIAS,
	T_PRECEDENCE,
	T_MIMIC,
	T_EMPTY,
	T_START,
	T_LEFT,
	T_RIGHT,
	T_NONASSOC,
	T_ERROR,
	T_EOF,
};

#define TOTAL_TOKEN	45

#ifndef _ONLY_STATE_TOKEN_

static int8_t
fgfx_begin_table[TOTAL_TOKEN][TOTAL_STATE] = {
	[T_BEG_REGEX][S_GLOBAL] = S_SEEN_REGEX, 
	[T_END_REGEX][S_SEEN_REGEX] = S_GLOBAL, 
};

#define START_STATE	1
#define DEAD_STATE	0

static uint8_t
fgfx_GLOBAL_state_table[36][256] = {
/*   0 */	{},
/*   1 */	{[125]=19, [124]=18, [123]=17, [97 ... 122]=14, [95]=14, [93]=16, [91]=15, [65 ... 90]=14, [61]=13, [60]=12, [59]=11, [47]=10, [45]=9, [44]=8, [42]=7, [41]=6, [40]=5, [39]=4, [36]=3, [32]=2, [9 ... 10]=2},
/*   2 */	{[32]=2, [9 ... 10]=2},
/*   3 */	{[97 ... 122]=20, [95]=20, [65 ... 90]=20},
/*   4 */	{[93 ... 255]=21, [92]=22, [40 ... 91]=21, [35 ... 38]=21, [11 ... 33]=21, [0 ... 9]=21},
/*   5 */	{},
/*   6 */	{},
/*   7 */	{},
/*   8 */	{},
/*   9 */	{[62]=23},
/*  10 */	{[47]=26, [42]=25, [32]=24, [9]=24},
/*  11 */	{},
/*  12 */	{[97 ... 122]=27, [95]=27, [65 ... 90]=27},
/*  13 */	{[62]=28},
/*  14 */	{[97 ... 122]=14, [95]=14, [65 ... 90]=14, [48 ... 57]=14},
/*  15 */	{},
/*  16 */	{},
/*  17 */	{},
/*  18 */	{},
/*  19 */	{},
/*  20 */	{[97 ... 122]=20, [95]=20, [65 ... 90]=20, [48 ... 57]=20},
/*  21 */	{[93 ... 255]=21, [92]=22, [40 ... 91]=21, [39]=29, [35 ... 38]=21, [11 ... 33]=21, [0 ... 9]=21},
/*  22 */	{[92]=21, [39]=21, [34]=21},
/*  23 */	{},
/*  24 */	{[32]=24, [9]=24},
/*  25 */	{[43 ... 255]=25, [42]=30, [0 ... 41]=25},
/*  26 */	{[11 ... 255]=26, [0 ... 9]=26},
/*  27 */	{[97 ... 122]=27, [95]=27, [65 ... 90]=27, [62]=32, [48 ... 57]=27, [39]=31},
/*  28 */	{},
/*  29 */	{},
/*  30 */	{[48 ... 255]=25, [47]=33, [43 ... 46]=25, [42]=30, [0 ... 41]=25},
/*  31 */	{[62]=32, [39]=34},
/*  32 */	{},
/*  33 */	{},
/*  34 */	{[62]=32, [39]=35},
/*  35 */	{[62]=32},
};

static uint8_t
fgfx_GLOBAL_final_table[23][2] = {
	{ 2, 	T_SPACE },
	{ 5, 	T_LPAREN },
	{ 6, 	T_RPAREN },
	{ 7, 	T_STAR },
	{ 8, 	T_COMMA },
	{ 10, 	T_BEG_REGEX },
	{ 11, 	T_SEMI },
	{ 13, 	T_EQUAL },
	{ 14, 	T_TERMINAL },
	{ 15, 	T_LBRACK },
	{ 16, 	T_RBRACK },
	{ 17, 	T_LBRACE },
	{ 18, 	T_UNION },
	{ 19, 	T_RBRACE },
	{ 20, 	T_DIRECTIVE },
	{ 23, 	T_ARROW },
	{ 24, 	T_BEG_REGEX },
	{ 26, 	T_COMMENT },
	{ 28, 	T_BARROW },
	{ 29, 	T_LITERAL },
	{ 32, 	T_NON_TERMINAL },
	{ 33, 	T_COMMENT },
	{ 0 },
};

static uint8_t
fgfx_SEEN_REGEX_state_table[5][256] = {
/*   0 */	{},
/*   1 */	{[93 ... 255]=2, [92]=4, [48 ... 91]=2, [47]=3, [33 ... 46]=2, [32]=1, [11 ... 31]=2, [9]=1, [0 ... 8]=2},
/*   2 */	{[93 ... 255]=2, [92]=4, [48 ... 91]=2, [11 ... 46]=2, [0 ... 9]=2},
/*   3 */	{},
/*   4 */	{[0 ... 255]=2},
};

static uint8_t
fgfx_SEEN_REGEX_final_table[4][2] = {
	{ 1, 	T_REGEX },
	{ 2, 	T_REGEX },
	{ 3, 	T_END_REGEX },
	{ 0 },
};

#define LOOK_TABLE_NOT_DEFINE	

static int8_t
fgfx_skip_table[5] = {
	T_BEG_REGEX,
	T_END_REGEX,
	T_SPACE,
	T_COMMENT,
	-1,
};

#endif /* _ONLY_STATE_TOKEN_ */
#endif /* _FGFX_LEX_H_ */
