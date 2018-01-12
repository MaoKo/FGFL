#ifndef _FGFX_LEX_H_
#define _FGFX_LEX_H_

#include <stdint.h>

enum {
	T_SPACE,
	T_COMMENT,
	T_EQUAL,
	T_REGEX,
	T_STAR,
	T_NON_TERMINAL,
	T_UNION,
	T_LBRACK,
	T_RBRACK,
	T_LITERAL,
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

#define TOTAL_TOKEN	42

#ifndef _ONLY_STATE_TOKEN_

#define START_STATE	1
#define DEAD_STATE	0

static uint8_t
fgfx_state_table[42][256] = {
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
/*  10 */	{[93 ... 255]=24, [92]=27, [48 ... 91]=24, [47]=26, [43 ... 46]=24, [42]=25, [11 ... 41]=24, [0 ... 9]=24},
/*  11 */	{},
/*  12 */	{[97 ... 122]=28, [95]=28, [65 ... 90]=28},
/*  13 */	{[62]=29},
/*  14 */	{[97 ... 122]=14, [95]=14, [65 ... 90]=14, [48 ... 57]=14},
/*  15 */	{},
/*  16 */	{},
/*  17 */	{},
/*  18 */	{},
/*  19 */	{},
/*  20 */	{[97 ... 122]=20, [95]=20, [65 ... 90]=20, [48 ... 57]=20},
/*  21 */	{[93 ... 255]=21, [92]=22, [40 ... 91]=21, [39]=30, [35 ... 38]=21, [11 ... 33]=21, [0 ... 9]=21},
/*  22 */	{[92]=21, [39]=21, [34]=21},
/*  23 */	{},
/*  24 */	{[93 ... 255]=24, [92]=27, [48 ... 91]=24, [47]=31, [11 ... 46]=24, [0 ... 9]=24},
/*  25 */	{[93 ... 255]=25, [92]=35, [48 ... 91]=25, [47]=34, [43 ... 46]=25, [42]=33, [11 ... 41]=25, [10]=32, [0 ... 9]=25},
/*  26 */	{[11 ... 255]=26, [0 ... 9]=26},
/*  27 */	{[0 ... 255]=24},
/*  28 */	{[97 ... 122]=28, [95]=28, [65 ... 90]=28, [62]=37, [48 ... 57]=28, [39]=36},
/*  29 */	{},
/*  30 */	{},
/*  31 */	{},
/*  32 */	{[43 ... 255]=32, [42]=38, [0 ... 41]=32},
/*  33 */	{[93 ... 255]=25, [92]=35, [48 ... 91]=25, [47]=39, [43 ... 46]=25, [42]=33, [11 ... 41]=25, [10]=32, [0 ... 9]=25},
/*  34 */	{[43 ... 255]=32, [42]=38, [0 ... 41]=32},
/*  35 */	{[43 ... 255]=25, [42]=33, [0 ... 41]=25},
/*  36 */	{[62]=37, [39]=40},
/*  37 */	{},
/*  38 */	{[48 ... 255]=32, [47]=39, [43 ... 46]=32, [42]=38, [0 ... 41]=32},
/*  39 */	{},
/*  40 */	{[62]=37, [39]=41},
/*  41 */	{[62]=37},
};

static uint8_t
fgfx_final_table[23][2] = {
	{ 2, 	T_SPACE },
	{ 5, 	T_LPAREN },
	{ 6, 	T_RPAREN },
	{ 7, 	T_STAR },
	{ 8, 	T_COMMA },
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
	{ 26, 	T_COMMENT },
	{ 29, 	T_BARROW },
	{ 30, 	T_LITERAL },
	{ 31, 	T_REGEX },
	{ 34, 	T_REGEX },
	{ 37, 	T_NON_TERMINAL },
	{ 39, 	T_COMMENT },
	{ 0 },
};

static int8_t
fgfx_skip_table[3] = {
	T_SPACE,
	T_COMMENT,
	-1,
};

#endif /* _ONLY_STATE_TOKEN_ */
#endif /* _FGFX_LEX_H_ */
