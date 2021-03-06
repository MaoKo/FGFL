#include <stdio.h>
#include <string.h>

#include "cfg.h"
#include "cfg_production.h"
#include "cfg_set_op.h"
#include "error.h"
#include "lexer.h"
#include "utils.h"

static size_t special_pos = 0;

cfg_t*
new_cfg(int filde) {
	cfg_t* cfg = NEW(cfg_t, 1);

	if (!cfg)
		{ return (NULL); }
	memset(cfg, 0, sizeof(cfg_t));

	cfg->miss_prod = true;
	cfg->productions = new_vector();
	cfg->non_terminal = new_vector();
	cfg->terminal = new_vector();
	cfg->token_file = new_vector();
	cfg->lex = new_lexer(filde);

	return (cfg);
}

static void
free_symbol(symbol_t* sym) {
	if (sym) {
		FREE(sym->name);
		if (sym->kind == T_NON_TERMINAL) {
			del_bitset(sym->first);
			del_bitset(sym->follow);
			del_bitset(sym->prod_lst);
		}
		else if ((sym->kind == T_TERMINAL) && (sym->prec))
			{ FREE(sym->prec); }
		else if ((sym->kind == T_LITERAL) && (!sym->share_prec))
			{ FREE(sym->prec); }
	}
	FREE(sym);
}

void
del_cfg(cfg_t* cfg) {
	if (cfg) {
		foreach_vector(cfg->non_terminal, &free_symbol);	
		del_vector(cfg->non_terminal);
		
		foreach_vector(cfg->terminal, &free_symbol);
		del_vector(cfg->terminal);
	
		foreach_vector(cfg->productions, &del_production);
		del_vector(cfg->productions);
	
		foreach_vector(cfg->token_file, &free);
		del_vector(cfg->token_file);

		del_lexer(cfg->lex);		
	}
	FREE(cfg);
}

static inline int
cmp_symbol_name(symbol_t const* sym, char const* name) {
	if (!sym->name)
		{ return (1); }
	return (strcmp(sym->name, name));
}

static symbol_t*
add_symbol_cfg(cfg_t* cfg, int kind, char const* crt_lexeme) {
	vector_t* dest = (kind == T_NON_TERMINAL)
						? cfg->non_terminal : cfg->terminal;
	if (crt_lexeme) {
		int index = get_index_vector(dest, crt_lexeme, &cmp_symbol_name);
		if (index != -1)
			{ return ((symbol_t*)AT_VECTOR(dest, index)); }
	}

	symbol_t* symbol = NEW(symbol_t, 1);
	if (!symbol)
		{ return (NULL); }
	memset(symbol, 0, sizeof(*symbol));

	symbol->kind = kind;
	if (crt_lexeme)
		{ symbol->name = strdup(crt_lexeme); }

	int fail = 0;
	if (crt_lexeme && !symbol->name)
		{ fail = 1; }
	
	if (kind == T_LITERAL) 
		{ symbol->terminal_alias = -1; }
	else {
		symbol->is_used = true;
		if (kind == T_TERMINAL)
			{ symbol->is_defined = true; }
		else {
			symbol->prod_lst = new_bitset();
			if (!symbol->prod_lst)
				{ fail = 1; }
		}
	}

	if (fail) {
		free_symbol(symbol);
		return (NULL);
	}

/*
	if (kind == T_NON_TERMINAL)
		{ *strchr(sym->name, '>') = EOS; }

	//vector_t** dst = (kind == T_NON_TERMINAL)
	//? cfg->non_terminal : cfg->terminal;
	//size_t hs = hash_str(sym->name) % HASH_SIZE;
	//else
	//	{ FREE(sym); }
*/

	symbol->index = SIZE_VECTOR(dest);
	PUSH_BACK_VECTOR(dest, symbol);

	return (symbol);
}

static int cfg_section(cfg_t*);

cfg_t*
parse_cfg(int filde) {
	cfg_t* cfg = new_cfg(filde);
	if (cfg_section(cfg) == ERROR) {
		del_cfg(cfg);
		return (NULL);
	}
	return (cfg);
}

void
augment_grammar(cfg_t* cfg) {
	if (!cfg)
		{ return; }

	symbol_t* start = add_symbol_cfg(cfg, T_NON_TERMINAL, "$START");
	start->is_defined = true;

	production_t* prod = new_production(start, cfg);
	ADD_BITSET(start->prod_lst, GET_INDEX(prod));

	add_symbol_rhs(prod, AT_VECTOR(cfg->non_terminal, cfg->goal));
	symbol_t* eof_symbol = add_symbol_cfg(cfg, T_TERMINAL, "EOF");

	eof_symbol->is_eof = true;
	add_symbol_rhs(prod, eof_symbol);

	cfg->goal = start->index;
}

// Different kind of section and their associate list.
static int cfg_path_list(cfg_t*);
static int cfg_alias_list(cfg_t*);
static int cfg_production_list(cfg_t*);
static int cfg_precedence_list(cfg_t*);

static int cfg_production(cfg_t*);
static int cfg_rhs(cfg_t*, symbol_t* lhs);
static int cfg_mimic(cfg_t*, production_t*);
static int cfg_opt_list(cfg_t*, production_t*);
static int cfg_list(cfg_t*, production_t*);
static int cfg_atom(cfg_t*, production_t*);

static int
cfg_entry_section(cfg_t* cfg) {
	int kind = advance_token(cfg->lex);
	int (*section_ptr)(cfg_t*) = NULL;
	switch (kind) {
		case T_EXTERN:		section_ptr = &cfg_path_list;
					break;
		case T_ALIAS:		section_ptr = &cfg_alias_list;
					break;
		case T_PRODUCTION:	section_ptr = &cfg_production_list;
					break;
		case T_PRECEDENCE:	section_ptr = &cfg_precedence_list;
					break; 
	}
	if (!section_ptr) {
		errorf(CURRENT_LINE(cfg->lex), "Bad section directive.");
		return (ERROR);
	}
	char miss_c;
	if ((miss_c = '{', advance_token(cfg->lex) != T_LBRACE)
			|| (miss_c = '\0', (*section_ptr)(cfg) == ERROR)
			|| (miss_c = '}', advance_token(cfg->lex) != T_RBRACE)
			|| (miss_c = ';', advance_token(cfg->lex) != T_SEMI)) {
		if (miss_c) {
			errorf(CURRENT_LINE(cfg->lex),
							"Missing a '%c' after the directive.", miss_c);
		}
		return (ERROR);
	}
	return (DONE);
}

int
cfg_section(cfg_t* cfg) {
	bool empty = true;
	while (peek_token(cfg->lex) != T_EOF) {
		if (cfg_entry_section(cfg) == ERROR)
			{ return (ERROR); }
		empty = false;
	}
	if (empty)
		{ warnf(CURRENT_LINE(cfg->lex), "Empty file."); }
	return (DONE);
}

int
cfg_path_list(cfg_t* cfg) {
	if (peek_token(cfg->lex) == T_RBRACE)
		{ return (DONE); }
	else if (advance_token(cfg->lex) == T_LITERAL) {
		unget_char_back_buffer(LAST_LEXEME(cfg->lex), 1);
		PUSH_BACK_VECTOR(cfg->token_file, strdup(C_LEXEME(cfg->lex) + 1));
		if (peek_token(cfg->lex) == T_COMMA) {
			advance_token(cfg->lex);
			return (cfg_path_list(cfg));
		}
	}
	else {
		errorf(CURRENT_LINE(cfg->lex),
						"Missing a literal in the $EXTERN section.");
		return (ERROR);
	}
	return (DONE);
}

static int
alias_literal_list(cfg_t* cfg, symbol_t* alias_ter) {
	if (peek_token(cfg->lex) != T_LITERAL) {
		errorf(CURRENT_LINE(cfg->lex),
			"Missing a literal after the => in the $ALIAS section.");
		return (ERROR);
	}

	size_t index_alias = alias_ter->index;
	while (peek_token(cfg->lex) == T_LITERAL) {
		advance_token(cfg->lex);
		symbol_t* literal = add_symbol_cfg(cfg, T_LITERAL, C_LEXEME(cfg->lex));

		if (literal->terminal_alias != -1) {
			errorf(CURRENT_LINE(cfg->lex),
						"Redefinning literal %s.", C_LEXEME(cfg->lex));
			return (ERROR);
		}
		else if ((alias_ter->prec && literal->prec)
					&& (alias_ter->prec != literal->prec)) {
			errorf(CURRENT_LINE(cfg->lex),
						"Several precedence for the terminal %s.",
						alias_ter->name);
			return (ERROR);
		}

		literal->is_defined = true;
		literal->terminal_alias = index_alias;
	
		if (literal->prec) {
			literal->share_prec = true;
			alias_ter->prec = literal->prec;
		}

		if (peek_token(cfg->lex) == T_COMMA)
			{ advance_token(cfg->lex); }
	}
	return (DONE);
}

int
cfg_alias_list(cfg_t* cfg) {
	if (peek_token(cfg->lex) == T_RBRACE)
		{ return (DONE); }
	else if (advance_token(cfg->lex) == T_LPAREN) {
		if (advance_token(cfg->lex) != T_TERMINAL) {
			errorf(CURRENT_LINE(cfg->lex),
						"Missing an token identifier in the $ALIAS section.");
			return (ERROR);
		}

		symbol_t* alias_ter = add_symbol_cfg(cfg,
										T_TERMINAL, C_LEXEME(cfg->lex));

		if (advance_token(cfg->lex) != T_BARROW) {
			errorf(CURRENT_LINE(cfg->lex),
						"Missing a => in the $ALIAS section.");
			return (ERROR);
		}
		else if (alias_literal_list(cfg, alias_ter) == ERROR)
			{ return (ERROR); }
		else if (advance_token(cfg->lex) != T_RPAREN) {
			errorf(CURRENT_LINE(cfg->lex),
						"Missing a close paren in the $ALIAS section.");
			return (ERROR);
		}
		else if (peek_token(cfg->lex) == T_COMMA) {
			advance_token(cfg->lex);
			return (cfg_alias_list(cfg));
		}
	}
	else {
		errorf(CURRENT_LINE(cfg->lex),
						"Missing an open paren in the $ALIAS section.");
		return (ERROR);
	}
	return (DONE);
}

static int
cfg_precedence_atom(cfg_t* cfg, size_t kind_prec, size_t prec_depth) {
	size_t allow_kind = peek_token(cfg->lex);

	if ((allow_kind != T_TERMINAL) && (allow_kind != T_LITERAL)) {
		errorf(CURRENT_LINE(cfg->lex),
						"Missing a token identifier or a literal.");
		return (ERROR);
	}

	while ((allow_kind == T_TERMINAL) || (allow_kind == T_LITERAL)) {
		advance_token(cfg->lex);
		symbol_t* prec_ter = add_symbol_cfg(cfg, allow_kind,
													C_LEXEME(cfg->lex));
		if (prec_ter->prec) {
			errorf(CURRENT_LINE(cfg->lex),
							"Precedence already define for the symbol %s.",
							prec_ter->name);
			return (ERROR);
		}

		precedence_t* prec = NEW(precedence_t, 1);
		if (!prec)
			{ return (ERROR); }

		memset(prec, 0, sizeof(*prec));
		prec->precedence = prec_depth;

		switch (kind_prec) {
			case T_LEFT: prec->left = true; break;
			case T_RIGHT: prec->right = true; break;
			case T_NONASSOC:
					prec->right = true;
					prec->left = true;
					break;
		}

		prec_ter->prec = prec;
		if (peek_token(cfg->lex) != T_COMMA
								&& peek_token(cfg->lex) != T_RPAREN) {
			errorf(CURRENT_LINE(cfg->lex),
						"Missing a close paren in the $PRECEDENCE section.");
			return (ERROR);
		}
		else if (peek_token(cfg->lex) == T_COMMA)
			{ advance_token(cfg->lex); }

		allow_kind = peek_token(cfg->lex);
	}

	advance_token(cfg->lex);
	return (DONE);
}

int
cfg_precedence_list(cfg_t* cfg) {
	size_t prec_depth = NOT_PREC + 1;
	while (1) {
		if (peek_token(cfg->lex) == T_RBRACE)
			{ return (DONE); }
		int kind_prec = advance_token(cfg->lex);
		switch (kind_prec) {
			case T_LEFT: case T_RIGHT: case T_NONASSOC: break;
			default: errorf(CURRENT_LINE(cfg->lex),
						"Bad associativity in the $PRECEDENCE section.");
					return (ERROR);
		}
		if (advance_token(cfg->lex) == T_LPAREN) {
			if (cfg_precedence_atom(cfg, kind_prec, prec_depth++) == ERROR)
				{ return (ERROR); }
			if (peek_token(cfg->lex) == T_COMMA)
				{ advance_token(cfg->lex); }
		}
		else {
			errorf(CURRENT_LINE(cfg->lex),
						"Missing an open paren in the $PRECEDENCE section.");
			return (ERROR);
		}
	}
}

int
cfg_production_list(cfg_t* cfg) {
	while (peek_token(cfg->lex) != T_RBRACE) {
		if (cfg_production(cfg) == ERROR)
			{ return (ERROR); }
		special_pos = 0;
	}
	return (DONE);
}

static int
follow_prod(cfg_t* cfg) {
	symbol_t* symbol_lhs = add_symbol_cfg(cfg,
							T_NON_TERMINAL, C_LEXEME(cfg->lex));

	if (peek_token(cfg->lex) == T_ARROW) {
		advance_token(cfg->lex);

		cfg->miss_prod = false;
		symbol_lhs->is_defined = true;

		if (cfg_rhs(cfg, symbol_lhs) == ERROR)
			{ return (ERROR); }
	}
	else if (advance_token(cfg->lex) == T_EQUAL) {
		if (advance_token(cfg->lex) != T_START) {
			errorf(CURRENT_LINE(cfg->lex),
						"Must $START follow '%s = '.", symbol_lhs->name);
			return (ERROR);
		}
		cfg->goal = symbol_lhs->index;
	}
	else {
		errorf(CURRENT_LINE(cfg->lex),
						"Missing an arrow (->) or an equal sign after"
						" the nonterminal %s.", symbol_lhs->name);
		return (ERROR);
	}
	return (DONE);
}

int
cfg_production(cfg_t* cfg) {
	char const* next;
	if ((next = "nonterminal", advance_token(cfg->lex) != T_NON_TERMINAL)
			|| (next = NULL, follow_prod(cfg) == ERROR)
			|| (next = ";", advance_token(cfg->lex) != T_SEMI)) {
		if (next) {
			errorf(CURRENT_LINE(cfg->lex),
							"Missing %s for the production.", next);
		}
		return (ERROR);
	}
	return (DONE);
}

int
cfg_rhs(cfg_t* cfg, symbol_t* lhs) {
	production_t* prod = new_production(lhs, cfg);
	ADD_BITSET(lhs->prod_lst, GET_INDEX(prod));

	if (!prod)
		{ return (ERROR); }
	else if ((cfg_opt_list(cfg, prod) == ERROR)
								|| (cfg_mimic(cfg, prod) == ERROR))
		{ return (ERROR); }
	while (peek_token(cfg->lex) == T_UNION) {
		advance_token(cfg->lex);

		prod = new_production(lhs, cfg);
		ADD_BITSET(lhs->prod_lst, GET_INDEX(prod));

		if (!prod)
			{ return (ERROR); }
		else if (cfg_opt_list(cfg, prod) == ERROR
									|| cfg_mimic(cfg, prod) == ERROR)
			{ return (ERROR); }
	}
	return (DONE);
}

int
cfg_mimic(cfg_t* cfg, production_t* crt_prod) {
	if (peek_token(cfg->lex) != T_LPAREN)
		{ return (DONE); }

	advance_token(cfg->lex);

	if (advance_token(cfg->lex) != T_MIMIC) {
		return (ERROR);
		errorf(CURRENT_LINE(cfg->lex),
					"The directive $MIMIC must follow the open paren.");
	}

	int allow_kind = advance_token(cfg->lex);
	if ((allow_kind != T_TERMINAL) && (allow_kind != T_LITERAL)) {
		errorf(CURRENT_LINE(cfg->lex),
					"The directive $MIMIC must be followed by"
					" a token name or a literal.");
		return (ERROR);
	}

	crt_prod->mimic_sym = add_symbol_cfg(cfg, allow_kind, C_LEXEME(cfg->lex));
	crt_prod->mimic_sym->is_used = true;

	if (advance_token(cfg->lex) != T_RPAREN) {
		errorf(CURRENT_LINE(cfg->lex),
					"A close paren must follow the directive $MIMIC.");
		return (ERROR);
	}

	return (DONE);
}

int
cfg_opt_list(cfg_t* cfg, production_t* prod) {
	if (in_first(cfg->lex, T_LBRACK, T_LBRACE,
								T_NON_TERMINAL, T_TERMINAL, T_LITERAL, -1))
		{ return (cfg_list(cfg, prod)); }
	else if (peek_token(cfg->lex) == T_EMPTY)
		{ advance_token(cfg->lex); }
	else if (!in_first(cfg->lex, T_RBRACK, T_RBRACE, 
									T_LPAREN, T_UNION, T_SEMI, -1)) {
		errorf(CURRENT_LINE(cfg->lex),
					"An open paren, a pipe or a semicolon must follow"
					" the production %zu.", GET_INDEX(prod) + 1);
		return (ERROR);
	}
	return (DONE);
}

int
cfg_list(cfg_t* cfg, production_t* prod) {
	if (cfg_atom(cfg, prod) == ERROR)
		{ return (ERROR); }
	while (in_first(cfg->lex, T_LBRACK, T_LBRACE,
								T_NON_TERMINAL, T_TERMINAL, T_LITERAL, -1)) {
		if (cfg_atom(cfg, prod) == ERROR)
			{ return (ERROR); }
	}
	return (DONE);
}

static int
add_recursive(cfg_t* cfg, symbol_t* opt_nter, size_t base_index) {
	size_t kind_recur = T_LEFT;
	if (peek_token(cfg->lex) == T_BARROW) {
		advance_token(cfg->lex);
		if (advance_token(cfg->lex) != T_LPAREN) {
			errorf(CURRENT_LINE(cfg->lex),
					"Missing an open paren when the kind of"
					" recursivity is choosen.");
			return (ERROR);
		}
		kind_recur = advance_token(cfg->lex);
		switch (kind_recur) {
			case T_LEFT: case T_RIGHT: break;
			default:
				errorf(CURRENT_LINE(cfg->lex), "Missing $RIGHT or $LEFT.");
				return (ERROR);
		}
		if (advance_token(cfg->lex) != T_RPAREN) {
			errorf(CURRENT_LINE(cfg->lex),
					"Missing an close paren when the kind of"
					" recursivity is choosen.");
			return (ERROR);
		}
	}

	for (size_t i = base_index; i < SIZE_VECTOR(cfg->productions); ++i) {
		production_t* prod = (production_t*)AT_VECTOR(cfg->productions, i);
		if (LHS(prod) == opt_nter) {
			if (kind_recur == T_RIGHT)
				{ add_symbol_rhs(prod, opt_nter); }
			else
				{ add_symbol_rhs_front(prod, opt_nter);  }
		}
	}
	return (DONE);
} 

static int
enclosed_element(cfg_t* cfg, production_t* prod) {
	int kind = ((advance_token(cfg->lex) == T_LBRACK) ? OPT : LST);

	int greater_depth = (LHS(prod)->depth > 2);
	int empty_opt = ((peek_token(cfg->lex) == T_RBRACK)
								|| (peek_token(cfg->lex) == T_RBRACE));

	if (greater_depth || empty_opt) {
		int exit_st;
		if (empty_opt) {
			warnf(CURRENT_LINE(cfg->lex), "Empty optional elements '%s'.",
											(kind == OPT) ? "[]" : "{}");
			advance_token(cfg->lex);
			exit_st = DONE;
		}
		if (greater_depth) {
			errorf(CURRENT_LINE(cfg->lex),
								"A nesting depth must be less than 3.");
			exit_st = ERROR;
		}
		return (exit_st);
	}

	symbol_t* opt_nter = add_symbol_cfg(cfg, T_NON_TERMINAL, NULL);

	opt_nter->is_defined = true;
	opt_nter->spec_kind = kind;

	int crt_depth = LHS(prod)->depth;

	if (!crt_depth)
		{ opt_nter->special = LHS(prod); }
	else
		{ opt_nter->special = LHS(prod)->special; }
	opt_nter->depth = crt_depth + 1;	

	opt_nter->spec_nth = ++special_pos;
	add_symbol_rhs(prod, opt_nter);	

	production_t* empty_prod = new_production(opt_nter, cfg);
	ADD_BITSET(opt_nter->prod_lst, GET_INDEX(empty_prod));

	size_t base_index = SIZE_VECTOR(cfg->productions);
	special_pos = 0;
	if (cfg_rhs(cfg, opt_nter) == ERROR)
		{ return (ERROR); }
	else if ((kind == OPT) && (advance_token(cfg->lex) != T_RBRACK)) {
		errorf(CURRENT_LINE(cfg->lex),
				"A close bracket must follow the optional elements.");
		return (ERROR);
	}
	else if ((kind == LST) && (advance_token(cfg->lex) != T_RBRACE)) {
		errorf(CURRENT_LINE(cfg->lex),
				"A close brace must follow the list elements.");
		return (ERROR);
	}

	if (kind == LST && (add_recursive(cfg, opt_nter, base_index) == ERROR))
		{ return (ERROR); }

	special_pos = opt_nter->spec_nth;
	return (DONE);
}

int
cfg_atom(cfg_t* cfg, production_t* prod) {
	if ((peek_token(cfg->lex) == T_LBRACK)
								|| (peek_token(cfg->lex) == T_LBRACE)) {
		if (enclosed_element(cfg, prod) == ERROR)
			{ return (ERROR); }
	}
	else {
		symbol_t* atom_symbol = add_symbol_cfg(cfg,
					advance_token(cfg->lex), C_LEXEME(cfg->lex));

		atom_symbol->is_used = true;
		if ((atom_symbol->kind == T_LITERAL)
									&& (atom_symbol->terminal_alias != -1)) {
			add_symbol_rhs(prod, AT_VECTOR(cfg->terminal,
								atom_symbol->terminal_alias));
		}
		else
			{ add_symbol_rhs(prod, atom_symbol); }
	}
	return (DONE);
}

static int
cfg_unused_symbol(vector_t const* symbol_tab) {
	int unused = DONE;
	for (size_t i = 0; i < SIZE_VECTOR(symbol_tab); ++i) {
		symbol_t* symbol = (symbol_t*)AT_VECTOR(symbol_tab, i);
		if (!symbol->is_defined) {
			errorf(0, "The %s %s is used but not defined.",
								((IS_NON_TERMINAL(symbol))
								? "non terminal" : "literal"), symbol->name);
			unused = ERROR;
		}
		else if (!symbol->is_used) {
			warnf(0, "The symbol %s is defined but not used.",
							symbol->name);
		}
	}
	return (unused);
}

static int
detect_bad_symbol(cfg_t* cfg) {
	if (!cfg)
		{ return (ERROR); }
	int exit_status = (cfg_unused_symbol(cfg->non_terminal) == ERROR)
			|| (cfg_unused_symbol(cfg->terminal) == ERROR );
	return ((exit_status) ? ERROR : DONE);
}

int
cfg_sanity_check(cfg_t* cfg) {
	if (!cfg)
		{ return (ERROR); }
	else if (cfg->miss_prod) {
		errorf(0, "No production have been encounter at all.");
		return (ERROR);
	}
	else if (!SIZE_VECTOR(cfg->token_file)) {
		errorf(0, "At most one location of token must be defined.");
		return (ERROR);
	}
	else if (detect_bad_symbol(cfg) == ERROR)
		{ return (ERROR); }
	else if (preprocess_literal(cfg) == ERROR)
		{ return (ERROR); }

	if (unreachable_production(cfg) == ERROR)
		{ warnf(0, "Some unreachable nonterminal has been remove."); }

	check_mimic_prod(cfg);

	detect_nullable(cfg);
	compute_first(cfg);

	if (cfg_not_realizable(cfg) == ERROR)
		{ return (ERROR); }

	compute_follow(cfg);

	return (DONE);
}

#ifdef PRINT_DEBUG

void
print_terminal(cfg_t const* cfg) {
	puts("=== TERMINAL ===");
	for (size_t i = 0; i < SIZE_VECTOR(cfg->terminal); ++i) {
		printf("%s\n", ((symbol_t*)
			AT_VECTOR(cfg->terminal, i))->name);
	}
}

void
print_non_terminal(cfg_t const* cfg) {
	puts("=== NON_TERMINAL ===");
	for (size_t i = 0; i < SIZE_VECTOR(cfg->non_terminal); ++i) {
		printf("%s\n", ((symbol_t*)
			AT_VECTOR(cfg->non_terminal, i))->name);
	}
}

void
print_production(cfg_t const* cfg) {
	puts("=== PRODUCTION ===");
	for (size_t i = 0; i < SIZE_VECTOR(cfg->productions); ++i) {
		production_t* prod = AT_VECTOR(cfg->productions, i);
		printf("%s -> ", prod->symbol_lhs->name);
		list_rhs_t* list = prod->rhs_element;
		while (list) {
			printf("%s", list->symbol_rhs->name);
			if (list->next)
				{ printf(" "); }
			list = list->next;
		}
		puts("");
	}
}

void
print_nullable(cfg_t const* cfg) {
	puts("=== NULLABLE SYMBOL ===");
	for (size_t i = 0; i < SIZE_VECTOR(cfg->non_terminal); ++i) {
		symbol_t* non_terminal = AT_VECTOR(cfg->non_terminal, i);
		if (non_terminal->nullable)
			{ printf("%s is nullable.\n", non_terminal->name); }
	}
}

void
print_first_set(cfg_t const* cfg) {
	puts("=== FIRST SET ===");
	for (size_t i = 0; i < SIZE_VECTOR(cfg->non_terminal); ++i) {
		symbol_t* non_terminal = AT_VECTOR(cfg->non_terminal, i);
		printf("FIRST(%s) = {", non_terminal->name);
		bool first = true;
		int j;
		while ((j = IT_NEXT(non_terminal->first)) != IT_NULL) {
			if (!first)
				{ printf(", "); }
			symbol_t* terminal = AT_VECTOR(cfg->terminal, j);
			printf("%s", terminal->name);
			first = false;
		}
		puts("}");
	}
}

void
print_follow_set(cfg_t const* cfg) {
	puts("=== FOLLOW SET ===");
	for (size_t i = 0; i < SIZE_VECTOR(cfg->non_terminal); ++i) {
		symbol_t* non_terminal = AT_VECTOR(cfg->non_terminal, i);
		printf("FOLLOW(%s) = {", non_terminal->name);
		bool first = true;
		int j;
		while ((j = IT_NEXT(non_terminal->follow)) != IT_NULL) {
			if (!first)
				{ printf(", "); }
			symbol_t* terminal = AT_VECTOR(cfg->terminal, j);
			printf("%s", terminal->name);
			first = false;
		}
		puts("}");
	}
}

#endif /* PRINT_DEBUG */

