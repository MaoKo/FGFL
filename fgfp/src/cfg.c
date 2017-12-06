#include <stdio.h>
#include <string.h>

#include "cfg.h"
#include "cfg_production.h"
#include "lexer.h"
#include "utils.h"

#define ONLY_TOKEN
#include "fgfx.lex.h"
#undef ONLY_TOKEN

static lexer_t* lex = NULL;

cfg_t*
new_cfg(void) {
	cfg_t* cfg = NEW(cfg_t, 1);
	if (!cfg)
		{ return (NULL); }
	memset(cfg, 0, sizeof(cfg_t));
	cfg->productions = new_vector();
	cfg->non_terminal = new_vector();
	cfg->terminal = new_vector();
	return (cfg);
}

static void
free_symbol(symbol_t* sym) {
	if (sym)
		{ FREE(sym->name); }
	FREE(sym);
}

void
del_cfg(cfg_t* cfg) {
	if (cfg) {
		//for (size_t i = 0; i < HASH_SIZE; ++i) {
		foreach_vector(cfg->non_terminal, &free_symbol);	
		del_vector(cfg->non_terminal);
		
		foreach_vector(cfg->terminal, &free_symbol);
		del_vector(cfg->terminal);
		//}
		foreach_vector(cfg->productions, &del_production);
		del_vector(cfg->productions);
	}
	FREE(cfg);
}


static int
cmp_symbol_name(symbol_t const* sym, char const* name) {
	return (strcmp(sym->name, name));
}

static symbol_t*
add_symbol_cfg(cfg_t* cfg, int kind) {
	char* crt_lexeme = BODY_BUFFER(LAST_LEXEME(lex));
	vector_t* dest = (kind == TNON_TER) ? cfg->non_terminal : cfg->terminal;
	int index = get_index_vector(dest, crt_lexeme, &cmp_symbol_name);
	if (index != -1)
		{ return (AT_VECTOR(dest, index)); }
	symbol_t* symbol = NEW(symbol_t, 1);
	if (!symbol)
		{ return (NULL); }
	symbol->kind = kind;	
/*
	size_t offset = 0;
	if (kind == NON_TERMINAL)
		{ offset = 1; }
*/
	symbol->name = strdup(crt_lexeme /* + offset */);
	if (!symbol->name) {
		free_symbol(symbol);
		return (NULL);
	}
/*
	if (kind == NON_TERMINAL)
		{ *strchr(sym->name, '>') = EOS; }
*/
	//vector_t** dst = (kind == NON_TERMINAL)
	//? cfg->non_terminal : cfg->terminal;
	//size_t hs = hash_str(sym->name) % HASH_SIZE;
	//else
	//	{ FREE(sym); }
	PUSH_BACK_VECTOR(dest, symbol);
	return (symbol);
}

static int cfg_syntax(cfg_t*);

cfg_t*
parse_cfg(int filde) {
	if (lex) {
		del_lexer(lex);
		lex = NULL;
	}
	if (!(lex = new_lexer(filde)))
		{ return (NULL); }
	cfg_t* cfg = new_cfg();
	if (!cfg)
		{ return (NULL); }
	if (cfg_syntax(cfg) == ERROR) {
		del_cfg(cfg);
		/* ERROR */
		return (NULL);
	}
	return (cfg);
}

static int cfg_prod(cfg_t*);
static int cfg_rhs(cfg_t*, symbol_t* lhs);
static int cfg_opt_list(cfg_t*, production_t*);
static int cfg_list(cfg_t*, production_t*);
static int cfg_atom(cfg_t*, production_t*);

int
cfg_syntax(cfg_t* cfg) {
	if (cfg_prod(cfg) == ERROR)
		{ return (ERROR); }
	while (peek_token(lex) != TEOF) {
		if (cfg_prod(cfg) == ERROR)
			{ return (ERROR); }
	}
	return (DONE);
}

int cfg_prod(cfg_t* cfg) {
	if (advance_token(lex) != TNON_TER) {
		/* ERROR */
		return (ERROR);
	}
	symbol_t* symbol_lhs = add_symbol_cfg(cfg, TNON_TER);
	symbol_lhs->is_defined = true;
	if ((advance_token(lex) != TARROW) || (cfg_rhs(cfg, symbol_lhs) == ERROR)
			|| (advance_token(lex) != TSEMI)) {
		free_symbol(symbol_lhs);
		/* ERROR */
		return (ERROR);
	}
	return (DONE);
}

int
cfg_rhs(cfg_t* cfg, symbol_t* lhs) {
	production_t* prod = new_production(lhs);
	if (!prod)
		{ return (ERROR); }
	if (cfg_opt_list(cfg, prod) == ERROR) {
		del_production(prod);
		return (ERROR);
	}
	PUSH_BACK_VECTOR(cfg->productions, prod);
	while (peek_token(lex) == TUNION) {
		advance_token(lex);
		prod = new_production(lhs);
		if (!prod)
			{ return (ERROR); }
		if (cfg_opt_list(cfg, prod) == ERROR) {
			del_production(prod);
			return (ERROR);
		}
		PUSH_BACK_VECTOR(cfg->productions, prod);
	}
	return (DONE);
}

int
cfg_opt_list(cfg_t* cfg, production_t* prod) {
	if (in_first(lex, TNON_TER, TTOKEN, -1))
		{ return (cfg_list(cfg, prod)); }
	else if (!in_first(lex, TUNION, TSEMI, -1)) {
		/* ERROR */
		return (ERROR);
	}
	return (DONE);
}

int
cfg_list(cfg_t* cfg, production_t* prod) {
	if (in_first(lex, TNON_TER, TTOKEN, -1)) {
		if (cfg_atom(cfg, prod) == ERROR)
			{ return (ERROR); }
	}
	while (in_first(lex, TNON_TER, TTOKEN, -1)) {
		if (cfg_atom(cfg, prod) == ERROR)
			{ return (ERROR); }
	}
	return (DONE);
}

int
cfg_atom(cfg_t* cfg, production_t* prod) {
	if (in_first(lex, TNON_TER, TTOKEN, -1)) {
		add_symbol_rhs(prod,
			add_symbol_cfg(cfg, advance_token(lex))); 
	}
	else {
		/* ERROR */
		return (ERROR);
	}
	return (DONE);
}

int
detect_bad_symbol(cfg_t* cfg) {
	if (!cfg)
		{ return (ERROR); }
	for (size_t i = 0; i < SIZE_VECTOR(cfg->non_terminal); ++i) {
		symbol_t* symbol = (symbol_t*)AT_VECTOR(cfg->non_terminal, i);
		if (!symbol->is_defined) {
			fprintf(stderr, "Non_Terminal %s used but no defined.\n",
				symbol->name);
		}
	}
	return (DONE);
}

void
detect_nullable(cfg_t* cfg) {
	if (!cfg)
		{ return; }
	for (size_t i = 0; i < SIZE_VECTOR(cfg->productions); ++i) {
		production_t* prod = (production_t*)AT_VECTOR(cfg->productions, i);
		if (!prod->rhs_element)
			{ prod->symbol_lhs->nullable = true; }
	}
}
