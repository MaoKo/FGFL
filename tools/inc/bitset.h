#ifndef BITSET_H
#define BITSET_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint64_t                _SETTYPE;

#define _BITS_IN_WORD           (sizeof(_SETTYPE) << 3)
#define _BYTE_SETTYPE(x)        ((x) * sizeof(_SETTYPE))
#define _DIV_WSIZE(x)           ((x) >> 6)
#define _MOD_WSIZE(x)           ((x) & 0x3F)
#define _DEFWORDS               (8)
#define _DEFBITS                (_DEFWORDS * _BITS_IN_WORD)
#define _ROUND(bit)             ((_DIV_WSIZE(bit) >> 3) << 4)

//Iterator Bitset
#define IT_INVERT(bs)           ((!((bs)->invert) ? (bs)->invert = true\
                                    : ((bs)->invert = false)), IT_RESET(bs))

#define IT_NEXT(bs)             (_next_bitset(bs))
#define IT_BACK(bs)             ((bs)->siter)
#define IT_SET(bs, iter)        ((bs)->siter = iter)

#define IT_RESET(bs)            ((!bs) ? 0 : (!bs->invert) ? ((bs)->siter = 0)\
                                        : ((bs)->siter = (bs)->nbits - 1))
#define IT_NULL                 (-1)

#define OP_BITSET(bs, x, op)    ((bs)->map[_DIV_WSIZE(x)] op\
                                                (1UL<<_MOD_WSIZE(x)))

#define IS_PRESENT(bs, x)       ((!bs || x >= (bs)->nbits) ? false\
                                        : (OP_BITSET(bs, x, &)))

#define ADD_BITSET(bs, x)       ((x >= (bs)->nbits)  ? _add_bitset(bs, x)\
                                        : (OP_BITSET(bs, x, |=)))

#define OFF_BITSET(bs, x)       ((x >= (bs)->nbits) ? 0 :\
                                        (OP_BITSET(bs, x, &= ~)))

#define CLEAR_BITSET(bs)        (memset((bs)->map, 0,\
                                        _BYTE_SETTYPE((bs)->nwords)))

//Common operation available with set

enum {
    _UNION,
    _INTERSECT,
    _DIFF,
    _COMPL,
};

#define UNION_BITSET(x, y)      (_op_bitset(_UNION, x, y))
#define INTERSECT_BITSET(x, y)  (_op_bitset(_INTERSECT, x, y))
#define DIFF_BITSET(x, y)       (_op_bitset(_DIFF, x, y))
#define COMPL_BITSET(x)         (_op_bitset(_COMPL, x, NULL))

#define NULL_BITSET             (NULL)

typedef struct {
    unsigned short nwords;
    unsigned int nbits;
    _SETTYPE* map;
    _SETTYPE defmap[_DEFWORDS];
    int siter;
    bool invert;
} bitset_t;

bitset_t* new_bitset(void);
void del_bitset(bitset_t*);
bitset_t* dup_bitset(bitset_t const*);
_SETTYPE _add_bitset(bitset_t*, size_t);
void add_range_bitset(bitset_t*, size_t, size_t);
bool is_empty_bitset(bitset_t const*);
bool eq_bitset(bitset_t const*, bitset_t const*);
bitset_t* _op_bitset(int, bitset_t*, bitset_t*);
void truncate_bitset(bitset_t*);
int _next_bitset(bitset_t*);
_SETTYPE hash_bitset(bitset_t const*);
bool is_subset_bitset(bitset_t const*, bitset_t const*);
bool is_disjoint_bitset(bitset_t const*, bitset_t const*);
size_t count_elt_bitset(bitset_t const*);
#ifdef PRINT_DEBUG
void print_bitset(bitset_t*);
#endif /* PRINT_DEBUG */

#endif /* BITSET_H */
