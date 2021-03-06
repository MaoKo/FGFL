#include <string.h>

#include "vector.h"
#include "utils.h"

static vector_t*
make_len_vector(size_t len) {
    vector_t* vect = NEW(vector_t, 1);
    if (!vect)
        { return (NULL_VECT); }
    memset(vect, 0, sizeof(vector_t));
    vect->body = NEW(void*, len);
    if (!vect->body) {
        FREE(vect);
        return (NULL_VECT);
    }
    vect->alloc = len;
    return (vect);
}

vector_t*
new_vector(void) {
    return (make_len_vector(_VECT_SIZE));
}

vector_t*
dup_vector(vector_t const* vect) {
    if (!vect)
        { return (NULL_VECT); }
    vector_t* new_vect = make_len_vector(vect->alloc);
    if (!new_vect)
        { return (NULL_VECT); }
    memcpy(new_vect->body, vect->body, sizeof(void*) * vect->alloc);
    return (new_vect);
}

void
del_vector(vector_t* vect) {
    if (vect)
        { FREE(vect->body); }
    FREE(vect);
}

void
clear_vector(vector_t* vect) {
    if (!vect)
        { return; }
    memset(vect->body, 0, sizeof(void*) * vect->alloc);
    vect->index = 0;
}

static void
extend_vector(vector_t* vect) {
    size_t new_sz;
    if (vect->index >= vect->alloc) {
        new_sz = round_up(vect->index);
        void** new_body = REALLOC(vect->body, sizeof(void*) * new_sz);
        if (new_body) {
            vect->body = new_body;
            vect->alloc = new_sz;
        }
    }
}

void
insert_vector(vector_t* vect, size_t offset, void* obj) {
    if (!vect)
        { return; }
    extend_vector(vect);
    if (offset >= vect->index)
        { vect->body[vect->index] = obj; }
    else {
        memmove(vect->body + offset + 1, vect->body + offset,
                sizeof(void*) * (vect->index - offset));
        vect->body[offset] = obj;
    }
    ++vect->index;
}

void
erase_vector(vector_t* vect, size_t offset) {
    if (!vect || offset >= vect->index)
        { return; }
    if (offset + 1 != vect->index) {
        memmove(vect->body + offset, vect->body + offset + 1,
                sizeof(void*) * (vect->index - offset - 1));
    }
    --vect->index;
}

void reverse_vector(vector_t* vect) {
    if (!vect)
        { return; }
    size_t middle = vect->index / 2;
    for (size_t i = 0; i < middle; ++i) {
        void* swap_data = vect->body[i];
        vect->body[i] = vect->body[vect->index - i - 1];
        vect->body[vect->index - i - 1] = swap_data;
    }
}

int get_index_vector(vector_t* vect, void const* obj, int (*pred)()) {
    if (!vect)
        { return (-1); }
    for (size_t i = 0; i < SIZE_VECTOR(vect); ++i) {
        if (pred && !(*pred)(AT_VECTOR(vect, i), obj))
            { return (i); }
        if (!pred && (obj == AT_VECTOR(vect, i)))
            { return (i); }
    }
    return (-1);
}

void
foreach_vector(vector_t* vect, void (*foncteur)()) {
    if (!vect)
        { return; }
    for (size_t i = 0; i < SIZE_VECTOR(vect); ++i)
        { (*foncteur)(AT_VECTOR(vect, i)); }
}

void
init_vector(vector_t* vect, size_t size, void* (*constructor)(void)) {
    if (!vect)
        { return; }
    for (size_t i = 0; i < size; ++i)
        { PUSH_BACK_VECTOR(vect, (*constructor)()); }
}

void
append_vector(vector_t* vect, vector_t const* src_vect) {
    if (!vect || !src_vect)
        { return; }
    for (size_t i = 0; i < SIZE_VECTOR(src_vect); ++i)
        { PUSH_BACK_VECTOR(vect, AT_VECTOR(src_vect, i)); }
}

void
move_vector(vector_t* vect, vector_t* src_vect) {
    append_vector(vect, src_vect);
    del_vector(src_vect);
}

