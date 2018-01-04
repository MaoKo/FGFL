#ifndef PREPROCESS_REGEX_H
#define PREPROCESS_REGEX_H

#include "fgfl.h"
#include "vector.h"
#include "bitset.h"

size_t size_ident(char const*);
vector_t* topological_sort(token_spec_t*);
int compute_regex(token_spec_t*);

#endif /* PREPROCESS_REGEX_H */
