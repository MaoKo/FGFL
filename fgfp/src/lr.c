#include <stdio.h>
#include <string.h>

#include "lr.h"
#include "cfg.h"
#include "cfg_production.h"
#include "error.h"
#include "utils.h"
#include "vector.h"

// Hash for speed (TODO implement with hash.c)
static vector_t* record_lr0_item[HASH_LR] = {};
static vector_t* record_kernel[HASH_LR] = {};

// Vector
static vector_t* record_lr1_item = NULL;

static inline int
hash_lr0_item(production_t const* prod, list_rhs_t const* dot_pos) {
	size_t hash = prod->index;
	if (dot_pos)
		{ hash += dot_pos->pos; }
	return (hash % HASH_LR);
}

static size_t
hash_lr1_item(bitset_t* item_set) {
	if (!item_set)
		{ return (0); }

	size_t hash = 0;
	int i;

	while ((i = IT_NEXT(item_set)) != IT_NULL) {
		lr1_item_t* crt_item = (lr1_item_t*)AT_VECTOR(record_lr1_item, i);
		hash += hash_lr0_item(PROD(CORE(crt_item)), DOT(CORE(crt_item)));
	}

	IT_RESET(item_set);
	return (hash % HASH_LR);
}

static lr1_state_t*
new_lr1_state(bitset_t* item) {
	lr1_state_t* state = NEW(lr1_state_t, 1);
	if (!state)
		{ return (NULL); }

	memset(state, 0, sizeof(lr1_state_t));

	state->items = item;
	state->hash_items = hash_lr1_item(item);
	state->first_reach = true;

	return (state);
}

void
del_lr1_state(lr1_state_t* state) {
	if (state) {
		del_bitset(state->items);
		del_trans_list(state->shift_lst);
		del_trans_list(state->goto_lst);
		del_trans_list(state->reduces);
		del_bitset(state->equiv_state);
	}
	FREE(state);
}

static inline int
cmp_lr0_item(lr0_item_t const* it1, lr0_item_t const* it2) {
	return 	(!		((PROD(it1) == PROD(it2))
				&&	(DOT(it1) == DOT(it2))
			));
}

static inline int
cmp_lr1_item(lr1_item_t const* it1, lr1_item_t const* it2) {
	return 	(!  	((CORE(it1) == CORE(it2))
				&&  (eq_bitset(it1->lookahead, it2->lookahead))
			));
}

static int
check_exist_lr0_item(bitset_t* state_item, lr1_item_t* item) {
	int back = IT_BACK(state_item);
	IT_RESET(state_item);

	int find = -1;
	int i;

	while ((i = IT_NEXT(state_item)) != IT_NULL) {
		lr1_item_t* crt_item = (lr1_item_t*)AT_VECTOR(record_lr1_item, i);
		if (!cmp_lr0_item(CORE(crt_item), CORE(item))) {
			find = i;
			break;
		}
	}

	IT_SET(state_item, back);
	return (find);
}

int
cmp_lr0_state(bitset_t* s1, bitset_t* s2) {
	if (count_elt_bitset(s1) != count_elt_bitset(s2))
		{ return (1); }

	int back = IT_BACK(s1);
	IT_RESET(s1);

	int eq = 0;
	int i;

	while ((i = IT_NEXT(s1)) != IT_NULL) {
		lr1_item_t* lr1 = (lr1_item_t*)AT_VECTOR(record_lr1_item, i);
		if (check_exist_lr0_item(s2, lr1) == -1) {
			eq = 1;
			break;
		}
	}

	IT_SET(s1, back);
	return (eq);
}

int
cmp_lr1_state(bitset_t* s1, bitset_t* b2) {
	return (!eq_bitset(s1, b2));
}

static int
cmp_kernel_lr0(kernel_t* kernel, bitset_t* kernel_item) {
	return (cmp_lr0_state(kernel->kernel_item, kernel_item));
}

static int
cmp_kernel_lr1(kernel_t* kernel, bitset_t* kernel_item) {
	return (cmp_lr1_state(kernel->kernel_item, kernel_item));
}

static kernel_t*
new_kernel(bitset_t* kernel_item, symbol_t* symbol, size_t hash_item) {
	if (!kernel_item)
		{ return (NULL); }

	kernel_t* prev = NULL;
	if (!record_kernel[hash_item])
		{ record_kernel[hash_item] = new_vector(); }
	else {
		int index_ker_lr0 = get_index_vector(record_kernel[hash_item],
								kernel_item, &cmp_kernel_lr0);

		if (index_ker_lr0 != -1) {
			prev = (kernel_t*)AT_VECTOR(record_kernel[hash_item],
												index_ker_lr0);

			int index_ker_lr1 = get_index_vector(record_kernel[hash_item],
									kernel_item, &cmp_kernel_lr1);
	
			if (index_ker_lr1 != -1) {
				del_bitset(kernel_item);
				return ((kernel_t*)AT_VECTOR(record_kernel[hash_item],
										index_ker_lr1));
			}
		}
	}

	kernel_t* new_kernel = NEW(kernel_t, 1);

	if (!new_kernel)
		{ return (NULL); }
	memset(new_kernel, 0, sizeof(kernel_t));

	new_kernel->kernel_item = kernel_item;
	new_kernel->rprefix = symbol;
	
	if (prev) {
		new_kernel->non_lr0_kernel_item = prev->non_lr0_kernel_item;
		new_kernel->ref_lr0 = true;
	}

	PUSH_BACK_VECTOR(record_kernel[hash_item], new_kernel);
	return (new_kernel);
}

static void
del_kernel(kernel_t* kernel) {
	if (kernel) {
		del_bitset(kernel->kernel_item);
		if (!kernel->ref_lr0)
			{ del_vector(kernel->non_lr0_kernel_item); }
		del_bitset(kernel->final_state);
	}
	FREE(kernel);
}

static void
del_lr1_item(lr1_item_t* item) {
	if (item) { 
		del_bitset(item->lookahead);
//		del_vector(item->non_kernel_look);
	}
	FREE(item);
}

static void
del_record_kernel(void) {
	for (size_t i = 0; i < HASH_LR; ++i) {
		if (!record_lr0_item[i])
			{ continue; }
		foreach_vector(record_kernel[i], &del_kernel);
		del_vector(record_kernel[i]);

		record_kernel[i] = NULL;
	}
}

static void
del_record_item(void) {
	foreach_vector(record_lr1_item, &del_lr1_item);
	del_vector(record_lr1_item);

	for (size_t i = 0; i < HASH_LR; ++i) {
		if (!record_lr0_item[i])
			{ continue; }
		foreach_vector(record_lr0_item[i], &free);
		del_vector(record_lr0_item[i]);
	
		record_lr0_item[i] = NULL;
	}
}

void del_lalr_record(void) {
	del_record_item();
	del_record_kernel();
}

static lr0_item_t*
new_lr0_item(production_t const* prod, list_rhs_t const* dot_pos) {
	if (!prod)
		{ return (NULL); }

	size_t hash = hash_lr0_item(prod, dot_pos);
	if (!record_lr0_item[hash])
		{ record_lr0_item[hash] = new_vector(); }

	for (size_t i = 0; i < SIZE_VECTOR(record_lr0_item[hash]); ++i) {
		lr0_item_t* target = (lr0_item_t*)AT_VECTOR(record_lr0_item[hash], i);
		if ((target->prod == prod) && (target->dot_pos == dot_pos))
			{ return (target); }
	}

	lr0_item_t* item = NEW(lr0_item_t, 1);
	if (!item)
		{ return (NULL); }

	memset(item, 0, sizeof(lr0_item_t));
	item->prod = prod;
	item->dot_pos = dot_pos;

	if (!DOT(item))
		{ item->is_final = true; }

	PUSH_BACK_VECTOR(record_lr0_item[hash], item);
	return (item);
}

static lr1_item_t*
new_lr1_item_lr0(lr0_item_t* lr0_item, bitset_t const* lookahead) {
	if (!lr0_item)
		{ return (NULL); }
	if (!record_lr1_item)
		{ record_lr1_item = new_vector(); }
	lr1_item_t* item = NEW(lr1_item_t, 1);
	if (!item)
		{ return (NULL); }
	memset(item, 0, sizeof(lr1_item_t));
	item->lookahead = dup_bitset(lookahead);
	item->core = lr0_item;
	return (item);

}

static inline lr1_item_t*
new_lr1_item_ptr(production_t const* prod,
					list_rhs_t const* dot_pos, bitset_t const* lookahead) {
	lr0_item_t* lr0_item = new_lr0_item(prod, dot_pos);
	if (!lr0_item)
		{ return (NULL); }
	return (new_lr1_item_lr0(lr0_item, lookahead));
}

static size_t
new_lr1_item_record(lr1_item_t* item) {
	if (!item)
		{ return (ERROR); }

	int index = get_index_vector(record_lr1_item, item, &cmp_lr1_item);
	if (index != -1) {
		del_lr1_item(item);
		return (index);
	}

	item->index = SIZE_VECTOR(record_lr1_item);
	PUSH_BACK_VECTOR(record_lr1_item, item);

	return (GET_INDEX(item));
}

int
new_lr1_item(production_t const* prod,
					list_rhs_t const* dot_pos, bitset_t const* lookahead) {
	return (new_lr1_item_record(new_lr1_item_ptr(prod, dot_pos, lookahead)));
}

static int
merge_lr1_item(lr1_item_t const* llr, lr1_item_t const* rlr) {
	if (cmp_lr0_item(CORE(llr), CORE(rlr)))
		{ return (ERROR); }

	if (is_subset_bitset(llr->lookahead, rlr->lookahead))
		{ return (GET_INDEX(llr)); }
	else if (is_subset_bitset(rlr->lookahead, llr->lookahead))
		{ return (GET_INDEX(rlr)); }

	bitset_t* new_look = dup_bitset(llr->lookahead);
	UNION_BITSET(new_look, rlr->lookahead);

	size_t index = new_lr1_item(PROD(CORE(llr)), DOT(CORE(llr)), new_look);

	del_bitset(new_look);
	return (index);
}

bool
merge_lr1_state(lr1_state_t* st1, lr1_state_t* st2) {
	if (!st1 || !st2)
		{ return (false); }

	if (st1->hash_items != st2->hash_items
			|| cmp_lr0_state(st1->items, st2->items))
		{ return (false); }

	bitset_t* new_item = new_bitset();
	int i;

	while ((i = IT_NEXT(st1->items)) != IT_NULL) {
		lr1_item_t* st1_item = (lr1_item_t*)AT_VECTOR(record_lr1_item, i);
		int j;
		while ((j = IT_NEXT(st2->items)) != IT_NULL) {
			lr1_item_t* st2_item = (lr1_item_t*)AT_VECTOR(record_lr1_item, j);
			if (!cmp_lr0_item(CORE(st1_item), CORE(st2_item))) {
				int index_new_item = merge_lr1_item(st1_item, st2_item);
				if (index_new_item != ERROR)
					{ ADD_BITSET(new_item, (size_t)index_new_item); }
				break;
			}
		}
		IT_RESET(st2->items);
	}
	IT_RESET(st1->items);

	if (eq_bitset(st1->items, new_item)) {
		del_bitset(new_item);
		return (false);
	}

	del_bitset(st1->items);
	st1->items = new_item;

	return (true);
}

static void
closure_non_kernel_item(cfg_t const* cfg, kernel_t* kernel) {
	if (kernel->non_lr0_kernel_item)
		{ return; }

	bitset_t* prod_lst = new_bitset();
	UNION_BITSET(prod_lst, kernel->rprefix->prod_lst);

	bool add_prod;
	do {
		add_prod = false;
		int i;
		while ((i = IT_NEXT(prod_lst)) != IT_NULL) {
			production_t* prod = (production_t*)AT_VECTOR(cfg->productions, i);
			if (!RHS(prod))
				{ continue; }
			else if (IS_NON_TERMINAL(RHS(prod)->symbol_rhs)) {
				if (is_subset_bitset(prod_lst, RHS(prod)->symbol_rhs->prod_lst))
					{ continue; }
				UNION_BITSET(prod_lst, RHS(prod)->symbol_rhs->prod_lst);
				add_prod = true;
			}
		}
		IT_RESET(prod_lst);
	} while (add_prod);

	kernel->non_lr0_kernel_item = new_vector();
	int i;
	while ((i = IT_NEXT(prod_lst)) != IT_NULL) {
		production_t* prod = (production_t*)AT_VECTOR(cfg->productions, i);
		PUSH_BACK_VECTOR(kernel->non_lr0_kernel_item,
					new_lr0_item(prod, prod->rhs_element));
	}
	del_bitset(prod_lst);
}

static bitset_t*
compute_look(lr0_item_t const* item, bitset_t* before) {
	bitset_t* target_first = first_list_rhs_t(DOT(item)->next);
	if (target_first == NULL_BITSET)
		{ target_first = dup_bitset(before); }
	else if (list_is_nullable(DOT(item)->next))
		{ UNION_BITSET(target_first, before); }
	return (target_first);
}

static bool
propagate_look_lr1_symbol(kernel_t* kernel, vector_t* vect_look,
							symbol_t* symbol, bitset_t* base_look) {
	bool change = false;
	for (size_t i = 0; i < SIZE_VECTOR(kernel->non_lr0_kernel_item); ++i) {
		bitset_t* crt_look = (bitset_t*)AT_VECTOR(vect_look, i);
		lr0_item_t const* crt_item = (lr0_item_t const*)
								AT_VECTOR(kernel->non_lr0_kernel_item, i);

		if (LHS(PROD(crt_item)) == symbol) {
			if (!is_subset_bitset(crt_look, base_look))
				{ change = true; }
			UNION_BITSET(crt_look, base_look);
		}
	}
	return (change);
}

static vector_t*
propagate_look_lr1(kernel_t* kernel) {
	vector_t* vect_look = new_vector();
	init_vector(vect_look, SIZE_VECTOR(kernel->non_lr0_kernel_item),
				(void*(*)(void))&new_bitset);

	bitset_t* kernel_look = new_bitset();

	int i;
	while ((i = IT_NEXT(kernel->kernel_item)) != IT_NULL) {
		lr1_item_t* item = (lr1_item_t*)AT_VECTOR(record_lr1_item, i);
		bitset_t* target_first = compute_look(CORE(item), item->lookahead);
		UNION_BITSET(kernel_look, target_first);
		del_bitset(target_first);
	}
	IT_RESET(kernel->kernel_item);

	propagate_look_lr1_symbol(kernel, vect_look, kernel->rprefix, kernel_look);
	bool repeat;
	do {
		repeat = false;
		for (size_t i = 0; i < SIZE_VECTOR(kernel->non_lr0_kernel_item); ++i) {
			bitset_t* crt_look = (bitset_t*)AT_VECTOR(vect_look, i);
			lr0_item_t const* crt_item = (lr0_item_t const*)
							AT_VECTOR(kernel->non_lr0_kernel_item, i);

			if (crt_item->is_final)
				{ continue; }

			symbol_t* leftmost = DOT(crt_item)->symbol_rhs;
			if (IS_NON_TERMINAL(leftmost)) {
				bitset_t* new_look = compute_look(crt_item, crt_look);
				if (propagate_look_lr1_symbol(kernel, vect_look,
													leftmost, new_look))
					{ repeat = true; }
				del_bitset(new_look);
			}
		}
	} while (repeat);

	del_bitset(kernel_look);
	return (vect_look);
}

static bitset_t*
include_new_item(cfg_t const* cfg, kernel_t* kernel) {
	if (kernel->final_state)
		{ return (kernel->final_state); }

	closure_non_kernel_item(cfg, kernel);
	vector_t* vect_look = propagate_look_lr1(kernel);

	kernel->final_state = new_bitset();
	UNION_BITSET(kernel->final_state, kernel->kernel_item);

	for (size_t i = 0; i < SIZE_VECTOR(kernel->non_lr0_kernel_item); ++i) {
		bitset_t* lr1_look = (bitset_t*)AT_VECTOR(vect_look, i);
		lr0_item_t* lr0_item = (lr0_item_t*)
									AT_VECTOR(kernel->non_lr0_kernel_item, i);
	
		lr1_item_t* add_item = new_lr1_item_lr0(lr0_item, lr1_look);

		if (!add_item)
			{ return (NULL_BITSET); }

		int index = new_lr1_item_record(add_item);
		if (index != ERROR)
			{ ADD_BITSET(kernel->final_state, (size_t)index); }
	}

	foreach_vector(vect_look, &del_bitset);
	del_vector(vect_look);

	return (kernel->final_state);
}

static kernel_t*
split_specific(bitset_t* item_set, symbol_t* symbol) {
	int back = IT_BACK(item_set);
	IT_RESET(item_set);
	
	bitset_t* cprefix = new_bitset();
	int i;

	while ((i = IT_NEXT(item_set)) != IT_NULL) {
		lr1_item_t* crt_item = (lr1_item_t*)AT_VECTOR(record_lr1_item, i);
		if (CORE(crt_item)->is_final)
			{ continue; }
		else if (DOT(CORE(crt_item))->symbol_rhs == symbol)
			{ ADD_BITSET(cprefix, (size_t)i); }
	}

	IT_SET(item_set, back);
	return (new_kernel(cprefix, symbol, hash_lr1_item(cprefix)));
}

static vector_t*
split_same_derivation(bitset_t* item_set) {
	bitset_t* symbol_seen = new_bitset();
	vector_t* kernel_lst = new_vector();

	int i;
	while ((i = IT_NEXT(item_set)) != IT_NULL) {
		lr1_item_t* crt_item = (lr1_item_t*)AT_VECTOR(record_lr1_item, i);
		if (CORE(crt_item)->is_final
				|| !IS_NON_TERMINAL(DOT(CORE(crt_item))->symbol_rhs))
			{ continue; }

		symbol_t* leftmost = DOT(CORE(crt_item))->symbol_rhs;
		if (IS_PRESENT(symbol_seen, leftmost->index))
			{ continue; }

		ADD_BITSET(symbol_seen, leftmost->index);
		kernel_t* crt_kernel = split_specific(item_set, leftmost);

		PUSH_BACK_VECTOR(kernel_lst, crt_kernel);
	}

	IT_RESET(item_set);
	del_bitset(symbol_seen);

	return (kernel_lst);
}

static void
merge_same_lr0_item(bitset_t* item_set) {
	int i;
	while ((i = IT_NEXT(item_set)) != IT_NULL) {
		lr1_item_t* target_item = AT_VECTOR(record_lr1_item, i);
		int j;

		int back = IT_BACK(item_set);
		while ((j = IT_NEXT(item_set)) != IT_NULL) {
			lr1_item_t* old_item = AT_VECTOR(record_lr1_item, j);
			if (!cmp_lr0_item(CORE(target_item), CORE(old_item))) {
				int new_index = merge_lr1_item(target_item, old_item);

				OFF_BITSET(item_set, (size_t)i);
				OFF_BITSET(item_set, (size_t)j);

				if (new_index != -1)
					{ ADD_BITSET(item_set, (size_t)new_index); }
				break;
			}
		}

		IT_SET(item_set, back);
	}
	IT_RESET(item_set);
}

bitset_t*
closure(cfg_t const* cfg, bitset_t* item_set) {
	if (!cfg || !item_set)
		{ return (NULL_BITSET); }

	vector_t* list_kernel = split_same_derivation(item_set);
	bool is_kernel_disjoint = true;

	size_t size_kernel;
	if ((size_kernel = SIZE_VECTOR(list_kernel))) {
		for (size_t i = 0; i < size_kernel - 1; ++i) {
			kernel_t* crt_kernel = (kernel_t*)AT_VECTOR(list_kernel, i);
			kernel_t* next_kernel = (kernel_t*)AT_VECTOR(list_kernel, i + 1);

			if (!is_disjoint_bitset(crt_kernel->rprefix->first,
								next_kernel->rprefix->first)) {
				is_kernel_disjoint = false;
				break;	
			}
		}

		for (size_t i = 0; i < size_kernel; ++i) {
			kernel_t* crt_kernel = (kernel_t*)AT_VECTOR(list_kernel, i);
			UNION_BITSET(item_set, include_new_item(cfg, crt_kernel));
		}
	}
	
	if (!is_kernel_disjoint)
		{ merge_same_lr0_item(item_set);  }

	del_vector(list_kernel);
	return (item_set);
}

bitset_t*
goto_lr(cfg_t const* cfg, bitset_t* item_set, symbol_t const* match_symbol) {
	if (!item_set || !match_symbol)
		{ return (NULL_BITSET); }

	bitset_t* goto_lr_state = new_bitset();

	int i = 0;
	while ((i = IT_NEXT(item_set)) != IT_NULL) {
		lr1_item_t* crt_item = (lr1_item_t*)AT_VECTOR(record_lr1_item, i);
		if (DOT(CORE(crt_item))
					&& (DOT(CORE(crt_item))->symbol_rhs == match_symbol)) {

			int index_item = new_lr1_item(PROD(CORE(crt_item)),
						DOT(CORE(crt_item))->next, crt_item->lookahead);

			if (index_item != ERROR)
				{ ADD_BITSET(goto_lr_state, (size_t)index_item); }
		}
	}

	IT_RESET(item_set);

	if (is_empty_bitset(goto_lr_state)) {
		del_bitset(goto_lr_state);
		return (NULL_BITSET);
	}

	return (closure(cfg, goto_lr_state));
}

static bitset_t*
move_dot_next(cfg_t const* cfg, lr1_state_t* state, int index) {
	lr1_item_t* item = (lr1_item_t*)AT_VECTOR(record_lr1_item, index);

	if (CORE(item)->is_final)
		{ return (NULL_BITSET); }
	else if (IS_TERMINAL(DOT(CORE(item))->symbol_rhs)
				&& IS_EOF(DOT(CORE(item))->symbol_rhs)) {
		state->accept = true;
		return (NULL_BITSET);
	}
	
	unsigned int input;
	symbol_t* target = DOT(CORE(item))->symbol_rhs;

	if (IS_TERMINAL(target)) {
		input = SHIFT(GET_INDEX(target));
		state->last_move = &(state->shift_lst);
	}
	else {
		input = GOTO(GET_INDEX(target));
		state->last_move  = &(state->goto_lst);
	}

	bool find = (cmp_input_trans_list(*(state->last_move), input) != -1);
	if (!find) {
		trans_list_t* action = new_trans_list(input, 0, *(state->last_move));
		*(state->last_move) = action;
	}
	else if (!state->merged)
		{ return (NULL_BITSET); }

	int back = IT_BACK(state->items);
	IT_RESET(state->items);

	bitset_t* lrg = goto_lr(cfg, state->items, DOT(CORE(item))->symbol_rhs);

	IT_SET(state->items, back);
	return (lrg);
}

vector_t*
build_lalr1_states(cfg_t const* cfg) {
	if (!cfg)
		{ return (NULL_VECT); }

	bitset_t* start_bitset = new_bitset();

	production_t* start_prod = BACK_VECTOR(cfg->productions);
	ADD_BITSET(start_bitset, (size_t)new_lr1_item(start_prod,
									start_prod->rhs_element, NULL_BITSET));

	vector_t* lr1_states = new_vector();
	lr1_state_t* start_state = new_lr1_state(closure(cfg, start_bitset));

	PUSH_BACK_VECTOR(lr1_states, start_state);

	vector_t* stack_todo_state = new_vector();
	PUSH_BACK_VECTOR(stack_todo_state, start_state);

	while (!EMPTY_VECTOR(stack_todo_state)) {
//		printf("SIZE VECTOR %zu\n", SIZE_VECTOR(lr1_states));

		lr1_state_t* state = (lr1_state_t*)BACK_VECTOR(stack_todo_state);
		POP_BACK_VECTOR(stack_todo_state);

		bitset_t* target_item = dup_bitset(state->items);
		bool self_merging = false;

		int j;
		while ((j = IT_NEXT(target_item)) != IT_NULL) {
			bitset_t* next = move_dot_next(cfg, state, j);
			if (next == NULL_BITSET)
				{ continue; }

			size_t hash_test = hash_lr1_item(next);
			size_t index;

			lr1_state_t* cmp_item = NULL;
			bool find = false;

			for (index = 0; index < SIZE_VECTOR(lr1_states); ++index) {
				cmp_item = (lr1_state_t*)AT_VECTOR(lr1_states, index);
				if (cmp_item->hash_items == hash_test
							&& !cmp_lr0_state(cmp_item->items, next)) {
					find = true;
					break;
				}
			}

			lr1_state_t* next_state = new_lr1_state(next);
			if (find) {
				if (cmp_lr1_state(cmp_item->items, next_state->items)) {
					if (merge_lr1_state(cmp_item, next_state)) {
						cmp_item->merged = true;
						self_merging = (cmp_item == state);
					}
					PUSH_BACK_VECTOR(stack_todo_state, cmp_item);
				}
				del_lr1_state(next_state);
			}
			else {
				PUSH_BACK_VECTOR(stack_todo_state, next_state);
				PUSH_BACK_VECTOR(lr1_states, next_state);
			}

			if (state->first_reach)
				{ (*(state->last_move))->state = index; }
		}
		del_bitset(target_item);

		state->merged = self_merging;
		state->first_reach = false;
	}

#if 0
	puts("== KERNEL ==");
	for (size_t i = 0; i < SIZE_VECTOR(record_kernel); ++i) {
		kernel_t* kernel = AT_VECTOR(record_kernel, i);
		print_state(cfg, kernel->kernel_item);
		puts("");
	}
#endif

	return (lr1_states);
}

static bool
handle_accept_conflict(cfg_t const* cfg, trans_list_t** reduce_list) {
	bool reach_end = false;
	bool once = false;

	while (!reach_end) {
		trans_list_t* reset_list = (*reduce_list);
		while (reset_list) {
			symbol_t* shift_sym = (symbol_t*)AT_VECTOR(cfg->terminal,
												reset_list->input);
			if (shift_sym->is_eof) {
				del_node_trans_list(reduce_list, reset_list);
				once = true;
				break;
			}

			reset_list = reset_list->next;
		}
		if (!reset_list)
			{ reach_end = true; }
	}
	return (once);
}

bool
check_conflict(cfg_t const* cfg, lr1_state_t* state,
									trans_list_t** reduce_list, int kind) {

	trans_list_t* target_action = (kind == SHIFT_REDUCE)
								? state->shift_lst : state->reduces;
	bool several = false;
	if (state->accept)
		{ several = handle_accept_conflict(cfg, reduce_list); }

	while (target_action) {
		trans_list_t* reset_list = (*reduce_list);
		while (reset_list) {
			if (kind == SHIFT_REDUCE) {
				symbol_t* shift_sym = (symbol_t*)AT_VECTOR(cfg->terminal,
												reset_list->input);

				if ((_SHIFT ^ target_action->input) == reset_list->input) {
					production_t* crt_prod = (production_t*)
												AT_VECTOR(cfg->productions,
												_REDUCE ^ reset_list->state);
					symbol_t* last_sym;
					if (crt_prod->mimic_sym)
						{ last_sym = crt_prod->mimic_sym; }
					else
						{ last_sym = last_symbol_in_prod(crt_prod); }

					if (last_sym && PRECEDENCE(last_sym) != NOT_PREC
							&& PRECEDENCE(shift_sym) != NOT_PREC) {
						if (PRECEDENCE(last_sym) > PRECEDENCE(shift_sym)) {
							del_node_trans_list(&state->shift_lst,
															target_action);
						}
						else if (PRECEDENCE(last_sym) < PRECEDENCE(shift_sym))
							{ del_node_trans_list(reduce_list, reset_list); }
						else {
							if (LEFT(shift_sym)) {
								del_node_trans_list(&state->shift_lst,
															target_action);
							}
							if (RIGHT(shift_sym)) 
							{ del_node_trans_list(reduce_list, reset_list); }
						}
						return (check_conflict(cfg, state, reduce_list, kind));
					}
					else {
						// Favor of shifting
						del_node_trans_list(reduce_list, reset_list);
						several = true; break;
					}
				}
			}
			else if ((kind == REDUCE_REDUCE)
					&& (target_action->input == reset_list->input)) {
				// Favor of reducing by the rule that appear earlier

				trans_list_t const* tnode =
					(target_action->state > reset_list->state) ?
						target_action : reset_list ;

				trans_list_t** list_src =
					(target_action->state > reset_list->state) ?
						&state->reduces : reduce_list ;

				del_node_trans_list(list_src, tnode);
				return (true);
			}
			reset_list = reset_list->next;
		}
		target_action = target_action->next;
	}
	return (several);
}

static void
detect_error(cfg_t const* cfg, lr1_state_t* state,
							trans_list_t* reduce_list, size_t index) {

	if (check_conflict(cfg, state, &reduce_list, SHIFT_REDUCE))
		{ warnf(0, "Shift/Reduce (state %zu).", index); }
	if (check_conflict(cfg, state, &reduce_list, REDUCE_REDUCE))
		{ warnf(0, "Reduce/Reduce (state %zu).", index); }

	if (!state->reduces)
		{ state->reduces = reduce_list; }
	else
		{ append_trans_list(state->reduces, reduce_list); }
}

static void
add_reduce(cfg_t const* cfg, lr1_state_t* state, lr1_item_t* item,
										bitset_t* src_symbol, size_t index) {
	int i;
	trans_list_t* reduce_list = NULL;

	while ((i = IT_NEXT(src_symbol)) != IT_NULL) {
		trans_list_t* next = new_trans_list(i,
								REDUCE(PROD(CORE(item))->index), reduce_list);
//		next->next = reduce_list;
		reduce_list = next;
	}

	IT_RESET(src_symbol);		
	detect_error(cfg, state, reduce_list, index);
}

static inline void
add_reduce_lr0(cfg_t const* cfg, lr1_state_t* state,
								lr1_item_t* item, size_t index) {
	bitset_t* all_terminal = new_bitset();
	for (size_t i = 0; i < SIZE_VECTOR(cfg->terminal); ++i) {
		symbol_t* ter = (symbol_t*)AT_VECTOR(cfg->terminal, i);
		if (IS_LITERAL(ter))
			{ continue; }
		ADD_BITSET(all_terminal, i);
	}
	add_reduce(cfg, state, item, all_terminal, index);
	del_bitset(all_terminal);
}

static inline void
add_reduce_slr(cfg_t const* cfg, lr1_state_t* state,
								lr1_item_t* item, size_t index) {
	symbol_t* symbol_lhs = PROD(CORE(item))->symbol_lhs;
	add_reduce(cfg, state, item, symbol_lhs->follow, index);
}

static inline void
add_reduce_lr1(cfg_t const* cfg, lr1_state_t* state,
								lr1_item_t* item, size_t index) {
	add_reduce(cfg, state, item, item->lookahead, index);
}

void 
compute_reduce_op(cfg_t const* cfg, vector_t* lr1_states) {
	if (!cfg || !lr1_states)
		{ return; }

	for (size_t i = 0; i < SIZE_VECTOR(lr1_states); ++i) {
		lr1_state_t* state = (lr1_state_t*)AT_VECTOR(lr1_states, i);
		int j;
		while ((j = IT_NEXT(state->items)) != IT_NULL) {
			lr1_item_t* item = (lr1_item_t*)AT_VECTOR(record_lr1_item, j);
			if (CORE(item)->is_final)
				{ add_reduce_lr1(cfg, state, item, i); }
		}
		IT_RESET(state->items);
	}
}

#ifdef PRINT_DEBUG

void
print_lr0_item(lr0_item_t* lr0_item) {
	printf("%s -> ", PROD(lr0_item)->symbol_lhs->name);
	list_rhs_t* list = PROD(lr0_item)->rhs_element;
	while (list) {
		if (list == DOT(lr0_item))
			{ printf(". "); }
		printf("%s", list->symbol_rhs->name);
		if (list->next)
			{ printf(" "); }
		list = list->next;
	}
	if (lr0_item->is_final) {
		if (PROD(lr0_item)->rhs_element)
			{ printf(" "); }
		printf(".");
	}
}

void
print_lr1_item(cfg_t const* cfg, lr1_item_t* lr1_item) {
	print_lr0_item(CORE(lr1_item));
	printf(", ");
	if (is_empty_bitset(lr1_item->lookahead))
		{ printf("???"); }
	else {
		printf("[");
		int j;
		bool first = true;
		while ((j = IT_NEXT(lr1_item->lookahead)) != IT_NULL) {
			symbol_t* ter = (symbol_t*)AT_VECTOR(cfg->terminal, j);
			if (!first)
				{ printf(", "); }
			printf("%s", ter->name);
			first = false;
		}
		IT_RESET(lr1_item->lookahead);
		printf("]");
	}	
}

void
print_state(cfg_t const* cfg, bitset_t* item_set) {
	int back = IT_BACK(item_set);
	IT_RESET(item_set);

	int i;
	while ((i = IT_NEXT(item_set)) != IT_NULL) {
		lr1_item_t* crt_item = (lr1_item_t*)AT_VECTOR(record_lr1_item, i);
		print_lr1_item(cfg, crt_item);
		puts("");
	}
	IT_SET(item_set, back);
}

static void
print_move(cfg_t const* cfg, trans_list_t* list, int kind) {
	char const* pattern = (kind == _SHIFT) ? "SHIFT" : "GOTO";
	vector_t const* src_vect = (kind == _SHIFT)
							? cfg->terminal : cfg->non_terminal;
	while (list) {
		if (kind & list->input) {
			printf("%s(%s)", pattern, ((symbol_t*)
				AT_VECTOR(src_vect, list->input ^ kind))->name);
		}
		printf(" into state %d.\n", list->state);
		list = list->next;
	}
}

void
print_report(cfg_t const* cfg, vector_t const* lr1_states) {
	for (size_t i = 0; i < SIZE_VECTOR(lr1_states); ++i) {
		printf("===== State %zu =====\n", i);
		lr1_state_t* state = AT_VECTOR(lr1_states, i);
		if (state->accept)
			{ printf("(Accept): "); }
		print_state(cfg, state->items);

		print_move(cfg, state->shift_lst, _SHIFT);
		print_move(cfg, state->goto_lst, _GOTO);

		trans_list_t* list = NULL;
		list = state->reduces;
		while (list) {
			printf("TOKEN(%s) => REDUCE(%d).\n",
				((symbol_t*)AT_VECTOR(cfg->terminal, list->input))->name,
				(_REDUCE ^ list->state) + 1);
			list = list->next;
		}
	}
}

#endif /* PRINT_DEBUG */

