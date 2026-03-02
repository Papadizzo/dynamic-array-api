#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>  /* Add this for SIZE_MAX */

/* Opaque pointer type for vector */
typedef struct vector* Vector;
typedef const struct vector* ConstVector;

/* Error handling */
typedef enum {
    VECTOR_SUCCESS = 0,
    VECTOR_ERROR_NULL_POINTER,
    VECTOR_ERROR_ALLOCATION_FAILED,
    VECTOR_ERROR_OUT_OF_RANGE,
    VECTOR_ERROR_INVALID_SIZE,
    VECTOR_ERROR_EMPTY
} VectorError;

/* Function pointer types for custom operations */
typedef void (*VectorElementDestroyFunc)(void* element);
typedef void* (*VectorElementCopyFunc)(const void* element);
typedef int (*VectorCompareFunc)(const void* a, const void* b);

/* Creation and destruction */
Vector vector_create(size_t element_size, VectorElementDestroyFunc destroy_func);
Vector vector_create_with_capacity(size_t element_size, size_t capacity, 
                                   VectorElementDestroyFunc destroy_func);
void vector_destroy(Vector vec);

/* Error handling */
VectorError vector_get_last_error(Vector vec);
const char* vector_error_string(VectorError error);

/* Capacity */
size_t vector_size(ConstVector vec);
size_t vector_capacity(ConstVector vec);  /* Fixed: added function name */
bool vector_empty(ConstVector vec);
VectorError vector_reserve(Vector vec, size_t new_capacity);
VectorError vector_shrink_to_fit(Vector vec);
size_t vector_max_size(void);

/* Element access */
void* vector_at(Vector vec, size_t index);
const void* vector_const_at(ConstVector vec, size_t index);
void* vector_front(Vector vec);
const void* vector_const_front(ConstVector vec);
void* vector_back(Vector vec);
const void* vector_const_back(ConstVector vec);
void* vector_data(Vector vec);
const void* vector_const_data(ConstVector vec);

/* Modifiers */
VectorError vector_assign(Vector vec, size_t count, const void* value);
VectorError vector_assign_range(Vector vec, const void* first, const void* last);
VectorError vector_push_back(Vector vec, const void* element);
VectorError vector_pop_back(Vector vec);
VectorError vector_insert(Vector vec, size_t position, const void* element);
VectorError vector_insert_count(Vector vec, size_t position, size_t count, const void* element);
VectorError vector_insert_range(Vector vec, size_t position, const void* first, const void* last);
VectorError vector_erase(Vector vec, size_t position);
VectorError vector_erase_range(Vector vec, size_t first, size_t last);
VectorError vector_clear(Vector vec);
VectorError vector_resize(Vector vec, size_t new_size);
VectorError vector_resize_value(Vector vec, size_t new_size, const void* value);
VectorError vector_swap(Vector vec1, Vector vec2);

/* Iterators (index-based) */
size_t vector_begin(ConstVector vec);
size_t vector_end(ConstVector vec);
size_t vector_next(ConstVector vec, size_t iterator);
size_t vector_prev(ConstVector vec, size_t iterator);

/* Searching and sorting */
size_t vector_find(ConstVector vec, const void* value, VectorCompareFunc compare);
size_t vector_find_if(ConstVector vec, bool (*predicate)(const void*));
VectorError vector_sort(Vector vec, VectorCompareFunc compare);
VectorError vector_reverse(Vector vec);

/* Memory management utilities */
VectorError vector_ensure_capacity(Vector vec, size_t min_capacity);
void* vector_detach_data(Vector vec, size_t* out_size);

/* Advanced operations */
VectorError vector_reserve_additional(Vector vec, size_t additional_elements);
Vector vector_clone(ConstVector vec, VectorElementCopyFunc copy_func);
VectorError vector_append(Vector dest, ConstVector src);

/* Utility functions */
bool vector_contains(ConstVector vec, const void* value, VectorCompareFunc compare);
void vector_for_each(Vector vec, void (*func)(void*));
VectorError vector_remove_if(Vector vec, bool (*predicate)(const void*));

#endif /* VECTOR_H */