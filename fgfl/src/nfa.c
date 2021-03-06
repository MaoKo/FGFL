#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "lexical_spec.h"
#include "regex.h"
#include "regex_node.h"
#include "nfa.h"
#include "utils.h"
#include "bitset.h"
#include "error.h"

vector_t* record_nfa_state = NULL;
vector_t* record_nfa_automata = NULL;

static nfa_automaton_t* dfs_regex_node(regex_node_t*);

static void
del_nfa_state(nfa_state_t* state) {
    if (state) {
        if (state->symbol_edge == EDGE_CLASS)
            { del_bitset(state->class); }
        del_bitset(state->eclos);
    }
    FREE(state);
}

static void
del_nfa_automaton(nfa_automaton_t* nfa_m) {
    if ((nfa_m) && (nfa_m->kind_nfa == NFA_MASTER))
        { del_bitset(nfa_m->out_automata); }
    FREE(nfa_m); 
}

void
del_nfa_record(void) {
    foreach_vector(record_nfa_state, &del_nfa_state);
    del_vector(record_nfa_state);
    foreach_vector(record_nfa_automata, &del_nfa_automaton);
    del_vector(record_nfa_automata);
}

static nfa_automaton_t*
new_nfa_automaton(int kind, ...) {
    if (!record_nfa_automata)
        { record_nfa_automata = new_vector(); }

    nfa_automaton_t* nfa_m = NEW(nfa_automaton_t, 1);
    if (!nfa_m)
        { return (NULL_AUTOMATON); }
    memset(nfa_m, 0, sizeof(*nfa_m)); 

    va_list args;
    va_start(args, kind);

    switch (kind) {
        case NFA_CLASS:
                nfa_m->tail_state = va_arg(args, nfa_state_t*);
                break;

        case NFA_SYMBOL:
                nfa_m->out_symbol = va_arg(args, int);
                break;

        case NFA_EPSILON:
                break;

        case NFA_UNION:
        case NFA_CONCAT:
                nfa_m->left = va_arg(args, nfa_automaton_t*);
                nfa_m->right = va_arg(args, nfa_automaton_t*);
                if (kind == NFA_UNION)
                    { nfa_m->tail_state = va_arg(args, nfa_state_t*); }
                break;

        case NFA_CLOSURE:
                nfa_m->middle = va_arg(args, nfa_automaton_t*);
                break;

        case NFA_MASTER:
                nfa_m->out_automata = va_arg(args, bitset_t*);
                break;
    }

    if (kind != NFA_MASTER)
        { nfa_m->head_state = va_arg(args, nfa_state_t*); }

    nfa_m->kind_nfa = kind;

    nfa_m->index = SIZE_VECTOR(record_nfa_automata);
    PUSH_BACK_VECTOR(record_nfa_automata, nfa_m);

    va_end(args);
    return (nfa_m);
}

nfa_state_t*
new_nfa_state(int symbol_edge, ...) {
    nfa_state_t* state = NEW(nfa_state_t, 1);

    if (!state)
        { return (NULL_NFA_STATE); }
    memset(state, 0, sizeof(*state));

    if (!record_nfa_state)
        { record_nfa_state = new_vector(); }

    va_list args;
    va_start(args, symbol_edge);

    state->symbol_edge = symbol_edge;
    switch (symbol_edge) {
        case EDGE_AUTOMATA:
            state->edge = va_arg(args, nfa_automaton_t*);
            state->edge2 = va_arg(args, nfa_automaton_t*);
            break;

        case EDGE_CLASS:
            state->class = va_arg(args, bitset_t*);            
            state->out_state = va_arg(args, nfa_state_t*);
            break;

        case NO_EDGE:
            break;

        default:
            state->out_state = va_arg(args, nfa_state_t*);
    }

    state->final_type = NO_FINAL;

    state->index = SIZE_VECTOR(record_nfa_state);
    PUSH_BACK_VECTOR(record_nfa_state, state);

    va_end(args);
    return (state);
}

static nfa_automaton_t*
regex_node_class(bitset_t* root_class) {
    nfa_state_t* new_head_state = new_nfa_state(NO_EDGE);
    if (!new_head_state)
        { return (NULL_AUTOMATON); }

    nfa_state_t* new_tail_state = new_nfa_state(EDGE_CLASS,
                                    dup_bitset(root_class), new_head_state);
    if (!new_tail_state) {
        del_nfa_state(new_head_state);
        return (NULL_AUTOMATON);
    }
    
    return (new_nfa_automaton(NFA_CLASS, new_tail_state, new_head_state));
}

static inline nfa_automaton_t*
regex_node_symbol(int symbol) {
    nfa_state_t* new_head_state = new_nfa_state(NO_EDGE);
    if (!new_head_state)
        { return (NULL_AUTOMATON); }
    return (new_nfa_automaton(NFA_SYMBOL, symbol, new_head_state));
}

static inline nfa_automaton_t*
regex_node_epsilon(void) {
    nfa_state_t* new_head_state = new_nfa_state(NO_EDGE);
    if (!new_head_state)
        { return (NULL_AUTOMATON); }
    return (new_nfa_automaton(NFA_EPSILON, new_head_state));
}

static nfa_automaton_t*
regex_node_tilde(regex_node_t* root) {
    size_t count_tilde = 0;
    while (root->kind_ast == AST_TILDE) {
        root = root->left;
        ++count_tilde;
    }
    if ((root->kind_ast != AST_SYMBOL) && (root->kind_ast != AST_CLASS)) {
        errorf(0, "The tilde '~' operator can only apply to"
                    " symbol or character class.");
        return (NULL_AUTOMATON);
    }
    else if (count_tilde % 2)
        { return (dfs_regex_node(root)); }

    bitset_t* negate_lang = NULL_BITSET;
    if (root->kind_ast == AST_SYMBOL) {
        negate_lang = new_bitset();
        ADD_BITSET(negate_lang, (size_t)root->symbol);
    }
    else
        { negate_lang = dup_bitset(root->class); }
    
    nfa_automaton_t* new_m = regex_node_class(COMPL_BITSET(negate_lang));
    del_bitset(negate_lang);

    return (new_m);
}

static nfa_automaton_t*
regex_node_option(regex_node_t* root) {
    regex_node_t* child = root->left;
    if (root->igcase)
        { set_option_ast(child, &set_igcase); }
    if (root->dotall)
        { set_option_ast(child, &set_dotall); }
    if (root->skipws)
        { set_option_ast(child, &set_skipws); }
    if (root->reverse)
        { reverse_regex_concat(child); }

    nfa_automaton_t* left = dfs_regex_node(child);
    return (left);
}

static inline nfa_automaton_t*
regex_node_dot(bool is_dotall) {
    bitset_t* dot_range = new_bitset();

    ADD_BITSET(dot_range, '\n');
    COMPL_BITSET(dot_range);

    if (is_dotall)
        { ADD_BITSET(dot_range, '\n'); }
    nfa_automaton_t* nfa_m = regex_node_class(dot_range);

    del_bitset(dot_range);
    return (nfa_m);
}

static nfa_automaton_t*
regex_node_union(regex_node_t* root) {
    nfa_automaton_t* left = dfs_regex_node(root->left);
    if (!left)
        { return (NULL_AUTOMATON); }

    nfa_automaton_t* right = dfs_regex_node(root->right);
    if (!right) {
        del_nfa_automaton(left);
        return (NULL_AUTOMATON);
    }

    nfa_state_t* new_tail_state = new_nfa_state(EDGE_AUTOMATA, left, right);
    nfa_state_t* new_head_state = new_nfa_state(NO_EDGE);

    if ((!new_tail_state) || (!new_head_state)) {
        del_nfa_automaton(left);
        del_nfa_automaton(right);
        return (NULL_AUTOMATON);
    }

    if (left->head_state->symbol_edge == NO_EDGE)
        { left->head_state->symbol_edge = EPSILON; }
    else
        { left->head_state->symbol_edge = EDGE_BOTH; }
    left->head_state->out_state = new_head_state; 

    if (right->head_state->symbol_edge == NO_EDGE)
        { right->head_state->symbol_edge = EPSILON; } 
    else
        { right->head_state->symbol_edge = EDGE_BOTH; }
    right->head_state->out_state = new_head_state;   

    return (new_nfa_automaton(NFA_UNION,
                                left, right, new_tail_state, new_head_state));
}

static bool
set_states_lookahead(nfa_automaton_t* right) {
    switch (right->kind_nfa) {
        case NFA_UNION: ;
            bool once_false = true;
            once_false = set_states_lookahead(right->right);
            if (!set_states_lookahead(right->left))
                { once_false = false; }
            return (once_false);

        case NFA_CONCAT:
            if (!set_states_lookahead(right->left)
                                && !set_states_lookahead(right->right))
                { return (false); }
            break;

        case NFA_CLOSURE:
            set_states_lookahead(right->middle);
            return (false);
 
        case NFA_CLASS:
        case NFA_SYMBOL:
            right->head_state->beg_lookahead = true;
            break;

        case NFA_EPSILON:
            return (false);

        default:
            break;
    }
    return (true);
}

static nfa_automaton_t*
regex_node_concat(regex_node_t* root) {
    nfa_automaton_t* left = dfs_regex_node(root->left);
    if (!left)
        { return (NULL_AUTOMATON); }

    size_t start_state = SIZE_VECTOR(record_nfa_state);

    nfa_automaton_t* right = dfs_regex_node(root->right);
    if (!right) {
        del_nfa_automaton(left);
        return (NULL_AUTOMATON);
    }
    else if (IS_LOOK(root)) {
        for (size_t i = start_state; i < SIZE_VECTOR(record_nfa_state); ++i) {
            nfa_state_t* crt_state = (nfa_state_t*)
                                        AT_VECTOR(record_nfa_state, i);
            crt_state->in_look_machine = true;
        }
    }

    if (left->head_state->symbol_edge != EDGE_AUTOMATA) {
        left->head_state->symbol_edge = EDGE_AUTOMATA;
        left->head_state->edge = right;
    }
    else
        { left->head_state->edge2 = right; }

    if (root->kind_ast == AST_LOOK)
        { set_states_lookahead(right); }

    return (new_nfa_automaton(NFA_CONCAT, left, right, right->head_state));
}

static nfa_automaton_t*
regex_node_closure(regex_node_t* root) {
    if (root->kind_ast == AST_QUES) {
        root->right = new_regex_node(AST_EPSILON);
        root->kind_ast = AST_UNION;

        return (regex_node_union(root));
    }
    else if (root->kind_ast == AST_PLUS) {
        root->right = new_regex_node(AST_STAR, cpy_regex_node(root->left));
        root->kind_ast = AST_CONCAT;

        return (regex_node_concat(root));
    }

    nfa_automaton_t* middle = dfs_regex_node(root->left);
    if (!middle)
        { return (NULL_AUTOMATON); }

    nfa_state_t* new_head_state = new_nfa_state(EDGE_AUTOMATA,
                                            middle, NULL_AUTOMATON);

    if (!new_head_state) {
        del_nfa_automaton(middle);
        return (NULL_AUTOMATON);
    }

    if (middle->head_state->symbol_edge == NO_EDGE) 
        { middle->head_state->symbol_edge = EPSILON; }
    else
        { middle->head_state->symbol_edge = EDGE_BOTH; }
    middle->head_state->out_state = new_head_state;

    return (new_nfa_automaton(NFA_CLOSURE, middle, new_head_state));
}

/* Depth First Search over the ast for constructing a sub-NFA */
static nfa_automaton_t* 
dfs_regex_node(regex_node_t* root) {
    if (root) {
        switch (root->kind_ast) {
            case AST_UNION:
                return (regex_node_union(root));

            case AST_LOOK:
            case AST_CONCAT:
                return (regex_node_concat(root));

            case AST_QUES:
            case AST_PLUS:
            case AST_STAR:
                return (regex_node_closure(root));

            case AST_TILDE:
                return (regex_node_tilde(root->left));

            case AST_OPTION:
                return (regex_node_option(root));

            case AST_SYMBOL:
                return (regex_node_symbol(root->symbol));

            case AST_EPSILON:
                return (regex_node_epsilon());

            case AST_DOT:
                return (regex_node_dot(root->is_dotall));

            case AST_CLASS:
                return (regex_node_class(root->class));

            default:
                break;
        }   
    }
    return (NULL_AUTOMATON);
}

static int
transform_regex_nfa(spec_entry_t* crt_entry) {
    regex_node_t* root = crt_entry->regex_ast;
    int exit_st = DONE;

    if ((crt_entry->nfa_m = dfs_regex_node(root)) == NULL_AUTOMATON)
        { exit_st = ERROR; }
    else
        { STATE_FINAL(crt_entry->nfa_m->head_state, GET_INDEX(crt_entry)); }

    if (crt_entry->beg_line)
        { STATE_ANCHOR(crt_entry->nfa_m->head_state); }

    del_regex_node(root);
    crt_entry->regex_ast = NULL_NODE;

    return (exit_st);
}

static int
build_nfa_from_state(lexical_spec_t* spec) {
    for (size_t i = 0; i < SIZE_VECTOR(spec->state_vect); ++i) {
        spec_entry_t* crt_state = (spec_entry_t*)AT_VECTOR(spec->state_vect, i);
        bitset_t* set_frag = new_bitset();

        for (size_t i = 0; i < SIZE_VECTOR(spec->entry_vect); ++i) {
            spec_entry_t* entry = (spec_entry_t*)AT_VECTOR(spec->entry_vect, i);
            if (entry->kind != T_KEYWORD && entry->nfa_m) {

                if ((cmp_input_trans_list(entry->state_lst,
                                                GET_INDEX(crt_state)) != -1)
                        || (!entry->state_lst && ((GET_INDEX(crt_state)
                            == (size_t)spec->start_state) || entry->all_state)))

                    { ADD_BITSET(set_frag, GET_INDEX(entry->nfa_m)); }
            }
        }

        nfa_automaton_t* nfa_m = new_nfa_automaton(NFA_MASTER, set_frag);
        crt_state->state_master = new_nfa_state(EDGE_AUTOMATA,
                                                    nfa_m, NULL_NFA_STATE);
    }
    return (DONE);
}

int
build_nfa(lexical_spec_t* spec) {
    if (!spec)
        { return (ERROR); }

    // Transform all regex to nfa
    for (size_t i = 0; i < SIZE_VECTOR(spec->entry_vect); ++i) {
        spec_entry_t* entry = (spec_entry_t*)AT_VECTOR(spec->entry_vect, i);
        if (entry->kind == T_KEYWORD || entry->fragment)
            { continue; }
        else if (transform_regex_nfa(entry) == ERROR)
            { return (ERROR); }
    }

    bool active_state = (spec->start_state != -1);
    if (active_state)
        { return (build_nfa_from_state(spec)); }
    else {
        bitset_t* set_frag = new_bitset();
        for (size_t i = 0; i < SIZE_VECTOR(spec->entry_vect); ++i) {
            spec_entry_t* entry = (spec_entry_t*)AT_VECTOR(spec->entry_vect, i);
            if (entry->kind != T_KEYWORD && entry->nfa_m)
                { ADD_BITSET(set_frag, GET_INDEX(entry->nfa_m)); }
        }
        nfa_automaton_t* nfa_m = new_nfa_automaton(NFA_MASTER, set_frag);
        spec->master = new_nfa_state(EDGE_AUTOMATA, nfa_m, NULL_NFA_STATE);
    }

    return (DONE);
}
