#include "vector.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h> 

#define VECTOR_MIN_CAPACITY 8
#define VECTOR_GROWTH_FACTOR 2

struct vector {
    void* data;
    size_t element_size;
    size_t size;
    size_t capacity;
    VectorElementDestroyFunc destroy_func;
    VectorError last_error;
};

/* Private helper functions */
static VectorError vector_grow(Vector vec, size_t min_capacity) {
    if (!vec) return VECTOR_ERROR_NULL_POINTER;
    
    size_t new_capacity = vec->capacity == 0 ? VECTOR_MIN_CAPACITY : vec->capacity * VECTOR_GROWTH_FACTOR;
    
    /* Ensure we meet minimum capacity */
    while (new_capacity < min_capacity) {
        new_capacity *= VECTOR_GROWTH_FACTOR;
    }
    
    /* Check for overflow */
    if (new_capacity > SIZE_MAX / vec->element_size) {
        vec->last_error = VECTOR_ERROR_ALLOCATION_FAILED;
        return VECTOR_ERROR_ALLOCATION_FAILED;
    }
    
    void* new_data = realloc(vec->data, new_capacity * vec->element_size);
    if (!new_data) {
        vec->last_error = VECTOR_ERROR_ALLOCATION_FAILED;
        return VECTOR_ERROR_ALLOCATION_FAILED;
    }
    
    vec->data = new_data;
    vec->capacity = new_capacity;
    vec->last_error = VECTOR_SUCCESS;
    return VECTOR_SUCCESS;
}

static VectorError vector_check_bounds(ConstVector vec, size_t index) {
    if (!vec) return VECTOR_ERROR_NULL_POINTER;
    if (index >= vec->size) return VECTOR_ERROR_OUT_OF_RANGE;
    return VECTOR_SUCCESS;
}

static void vector_destroy_elements(Vector vec, size_t start, size_t count) {
    if (!vec || !vec->destroy_func) return;
    
    char* elements = (char*)vec->data;
    for (size_t i = 0; i < count; i++) {
        vec->destroy_func(elements + (start + i) * vec->element_size);
    }
}

/* Creation and destruction */
Vector vector_create(size_t element_size, VectorElementDestroyFunc destroy_func) {
    return vector_create_with_capacity(element_size, 0, destroy_func);
}

Vector vector_create_with_capacity(size_t element_size, size_t capacity, 
                                   VectorElementDestroyFunc destroy_func) {
    if (element_size == 0) return NULL;
    
    Vector vec = (Vector)malloc(sizeof(struct vector));
    if (!vec) return NULL;
    
    vec->element_size = element_size;
    vec->size = 0;
    vec->capacity = 0;
    vec->destroy_func = destroy_func;
    vec->last_error = VECTOR_SUCCESS;
    vec->data = NULL;
    
    if (capacity > 0) {
        vec->data = malloc(capacity * element_size);
        if (!vec->data) {
            free(vec);
            return NULL;
        }
        vec->capacity = capacity;
    }
    
    return vec;
}

void vector_destroy(Vector vec) {
    if (!vec) return;
    
    if (vec->destroy_func) {
        vector_destroy_elements(vec, 0, vec->size);
    }
    
    free(vec->data);
    free(vec);
}

/* Error handling */
VectorError vector_get_last_error(Vector vec) {
    return vec ? vec->last_error : VECTOR_ERROR_NULL_POINTER;
}

const char* vector_error_string(VectorError error) {
    switch (error) {
        case VECTOR_SUCCESS: return "Success";
        case VECTOR_ERROR_NULL_POINTER: return "Null pointer provided";
        case VECTOR_ERROR_ALLOCATION_FAILED: return "Memory allocation failed";
        case VECTOR_ERROR_OUT_OF_RANGE: return "Index out of range";
        case VECTOR_ERROR_INVALID_SIZE: return "Invalid size specified";
        case VECTOR_ERROR_EMPTY: return "Vector is empty";
        default: return "Unknown error";
    }
}

/* Capacity */
size_t vector_size(ConstVector vec) {
    return vec ? vec->size : 0;
}

size_t vector_capacity(ConstVector vec) {
    return vec ? vec->capacity : 0;
}

bool vector_empty(ConstVector vec) {
    return vec ? vec->size == 0 : true;
}

VectorError vector_reserve(Vector vec, size_t new_capacity) {
    if (!vec) return VECTOR_ERROR_NULL_POINTER;
    
    if (new_capacity <= vec->capacity) {
        vec->last_error = VECTOR_SUCCESS;
        return VECTOR_SUCCESS;
    }
    
    if (new_capacity > SIZE_MAX / vec->element_size) {
        vec->last_error = VECTOR_ERROR_ALLOCATION_FAILED;
        return VECTOR_ERROR_ALLOCATION_FAILED;
    }
    
    void* new_data = realloc(vec->data, new_capacity * vec->element_size);
    if (!new_data) {
        vec->last_error = VECTOR_ERROR_ALLOCATION_FAILED;
        return VECTOR_ERROR_ALLOCATION_FAILED;
    }
    
    vec->data = new_data;
    vec->capacity = new_capacity;
    vec->last_error = VECTOR_SUCCESS;
    return VECTOR_SUCCESS;
}

VectorError vector_shrink_to_fit(Vector vec) {
    if (!vec) return VECTOR_ERROR_NULL_POINTER;
    
    if (vec->size == vec->capacity) {
        vec->last_error = VECTOR_SUCCESS;
        return VECTOR_SUCCESS;
    }
    
    if (vec->size == 0) {
        free(vec->data);
        vec->data = NULL;
        vec->capacity = 0;
        vec->last_error = VECTOR_SUCCESS;
        return VECTOR_SUCCESS;
    }
    
    void* new_data = realloc(vec->data, vec->size * vec->element_size);
    if (!new_data) {
        /* Not fatal - we can keep the existing allocation */
        vec->last_error = VECTOR_SUCCESS;
        return VECTOR_SUCCESS;
    }
    
    vec->data = new_data;
    vec->capacity = vec->size;
    vec->last_error = VECTOR_SUCCESS;
    return VECTOR_SUCCESS;
}

size_t vector_max_size(void) {
    return SIZE_MAX / sizeof(void*); /* Conservative estimate */
}

/* Element access */
void* vector_at(Vector vec, size_t index) {
    if (!vec) return NULL;
    if (vector_check_bounds(vec, index) != VECTOR_SUCCESS) {
        vec->last_error = VECTOR_ERROR_OUT_OF_RANGE;
        return NULL;
    }
    
    vec->last_error = VECTOR_SUCCESS;
    return (char*)vec->data + index * vec->element_size;
}

const void* vector_const_at(ConstVector vec, size_t index) {
    if (!vec) return NULL;
    if (index >= vec->size) return NULL;
    return (const char*)vec->data + index * vec->element_size;
}

void* vector_front(Vector vec) {
    return vector_at(vec, 0);
}

const void* vector_const_front(ConstVector vec) {
    return vector_const_at(vec, 0);
}

void* vector_back(Vector vec) {
    if (!vec || vec->size == 0) {
        if (vec) vec->last_error = VECTOR_ERROR_EMPTY;
        return NULL;
    }
    return vector_at(vec, vec->size - 1);
}

const void* vector_const_back(ConstVector vec) {
    if (!vec || vec->size == 0) return NULL;
    return vector_const_at(vec, vec->size - 1);
}

void* vector_data(Vector vec) {
    return vec ? vec->data : NULL;
}

const void* vector_const_data(ConstVector vec) {
    return vec ? vec->data : NULL;
}

/* Modifiers */
VectorError vector_assign(Vector vec, size_t count, const void* value) {
    if (!vec || !value) return VECTOR_ERROR_NULL_POINTER;
    
    /* Clear existing elements */
    vector_destroy_elements(vec, 0, vec->size);
    
    VectorError err = vector_resize(vec, count);
    if (err != VECTOR_SUCCESS) return err;
    
    /* Fill with value */
    for (size_t i = 0; i < count; i++) {
        void* dest = (char*)vec->data + i * vec->element_size;
        memcpy(dest, value, vec->element_size);
    }
    
    vec->last_error = VECTOR_SUCCESS;
    return VECTOR_SUCCESS;
}

VectorError vector_assign_range(Vector vec, const void* first, const void* last) {
    if (!vec || !first || !last) return VECTOR_ERROR_NULL_POINTER;
    if (first >= last) return VECTOR_ERROR_INVALID_SIZE;
    
    size_t count = ((const char*)last - (const char*)first) / vec->element_size;
    
    /* Clear existing elements */
    vector_destroy_elements(vec, 0, vec->size);
    
    VectorError err = vector_reserve(vec, count);
    if (err != VECTOR_SUCCESS) return err;
    
    memcpy(vec->data, first, count * vec->element_size);
    vec->size = count;
    
    vec->last_error = VECTOR_SUCCESS;
    return VECTOR_SUCCESS;
}

VectorError vector_push_back(Vector vec, const void* element) {
    if (!vec || !element) return VECTOR_ERROR_NULL_POINTER;
    
    if (vec->size >= vec->capacity) {
        VectorError err = vector_grow(vec, vec->size + 1);
        if (err != VECTOR_SUCCESS) return err;
    }
    
    void* dest = (char*)vec->data + vec->size * vec->element_size;
    memcpy(dest, element, vec->element_size);
    vec->size++;
    
    vec->last_error = VECTOR_SUCCESS;
    return VECTOR_SUCCESS;
}

VectorError vector_pop_back(Vector vec) {
    if (!vec) return VECTOR_ERROR_NULL_POINTER;
    if (vec->size == 0) {
        vec->last_error = VECTOR_ERROR_EMPTY;
        return VECTOR_ERROR_EMPTY;
    }
    
    vec->size--;
    
    if (vec->destroy_func) {
        void* element = (char*)vec->data + vec->size * vec->element_size;
        vec->destroy_func(element);
    }
    
    vec->last_error = VECTOR_SUCCESS;
    return VECTOR_SUCCESS;
}

VectorError vector_insert(Vector vec, size_t position, const void* element) {
    return vector_insert_count(vec, position, 1, element);
}

VectorError vector_insert_count(Vector vec, size_t position, size_t count, const void* element) {
    if (!vec || !element) return VECTOR_ERROR_NULL_POINTER;
    if (position > vec->size) {
        vec->last_error = VECTOR_ERROR_OUT_OF_RANGE;
        return VECTOR_ERROR_OUT_OF_RANGE;
    }
    if (count == 0) {
        vec->last_error = VECTOR_SUCCESS;
        return VECTOR_SUCCESS;
    }
    
    /* Ensure capacity */
    if (vec->size + count > vec->capacity) {
        VectorError err = vector_grow(vec, vec->size + count);
        if (err != VECTOR_SUCCESS) return err;
    }
    
    /* Shift elements after position */
    char* data = (char*)vec->data;
    memmove(data + (position + count) * vec->element_size,
            data + position * vec->element_size,
            (vec->size - position) * vec->element_size);
    
    /* Fill with copies of element */
    for (size_t i = 0; i < count; i++) {
        void* dest = data + (position + i) * vec->element_size;
        memcpy(dest, element, vec->element_size);
    }
    
    vec->size += count;
    vec->last_error = VECTOR_SUCCESS;
    return VECTOR_SUCCESS;
}

VectorError vector_insert_range(Vector vec, size_t position, const void* first, const void* last) {
    if (!vec || !first || !last) return VECTOR_ERROR_NULL_POINTER;
    if (position > vec->size) {
        vec->last_error = VECTOR_ERROR_OUT_OF_RANGE;
        return VECTOR_ERROR_OUT_OF_RANGE;
    }
    if (first >= last) {
        vec->last_error = VECTOR_SUCCESS;
        return VECTOR_SUCCESS;
    }
    
    size_t count = ((const char*)last - (const char*)first) / vec->element_size;
    
    /* Ensure capacity */
    if (vec->size + count > vec->capacity) {
        VectorError err = vector_grow(vec, vec->size + count);
        if (err != VECTOR_SUCCESS) return err;
    }
    
    /* Shift elements after position */
    char* data = (char*)vec->data;
    memmove(data + (position + count) * vec->element_size,
            data + position * vec->element_size,
            (vec->size - position) * vec->element_size);
    
    /* Copy new elements */
    memcpy(data + position * vec->element_size, first, count * vec->element_size);
    
    vec->size += count;
    vec->last_error = VECTOR_SUCCESS;
    return VECTOR_SUCCESS;
}

VectorError vector_erase(Vector vec, size_t position) {
    return vector_erase_range(vec, position, position + 1);
}

VectorError vector_erase_range(Vector vec, size_t first, size_t last) {
    if (!vec) return VECTOR_ERROR_NULL_POINTER;
    if (first > last || last > vec->size) {
        vec->last_error = VECTOR_ERROR_OUT_OF_RANGE;
        return VECTOR_ERROR_OUT_OF_RANGE;
    }
    
    size_t count = last - first;
    if (count == 0) {
        vec->last_error = VECTOR_SUCCESS;
        return VECTOR_SUCCESS;
    }
    
    /* Destroy elements being removed */
    vector_destroy_elements(vec, first, count);
    
    /* Shift elements */
    char* data = (char*)vec->data;
    memmove(data + first * vec->element_size,
            data + last * vec->element_size,
            (vec->size - last) * vec->element_size);
    
    vec->size -= count;
    vec->last_error = VECTOR_SUCCESS;
    return VECTOR_SUCCESS;
}

VectorError vector_clear(Vector vec) {
    if (!vec) return VECTOR_ERROR_NULL_POINTER;
    
    vector_destroy_elements(vec, 0, vec->size);
    vec->size = 0;
    vec->last_error = VECTOR_SUCCESS;
    return VECTOR_SUCCESS;
}

VectorError vector_resize(Vector vec, size_t new_size) {
    return vector_resize_value(vec, new_size, NULL);
}

VectorError vector_resize_value(Vector vec, size_t new_size, const void* value) {
    if (!vec) return VECTOR_ERROR_NULL_POINTER;
    
    if (new_size < vec->size) {
        /* Shrinking */
        vector_destroy_elements(vec, new_size, vec->size - new_size);
        vec->size = new_size;
        vec->last_error = VECTOR_SUCCESS;
        return VECTOR_SUCCESS;
    } else if (new_size > vec->size) {
        /* Growing */
        size_t old_size = vec->size;
        
        /* Ensure capacity */
        if (new_size > vec->capacity) {
            VectorError err = vector_grow(vec, new_size);
            if (err != VECTOR_SUCCESS) return err;
        }
        
        vec->size = new_size;
        
        /* Initialize new elements if value provided */
        if (value) {
            char* data = (char*)vec->data;
            for (size_t i = old_size; i < new_size; i++) {
                memcpy(data + i * vec->element_size, value, vec->element_size);
            }
        }
    }
    
    vec->last_error = VECTOR_SUCCESS;
    return VECTOR_SUCCESS;
}

VectorError vector_swap(Vector vec1, Vector vec2) {
    if (!vec1 || !vec2) return VECTOR_ERROR_NULL_POINTER;
    
    struct vector temp = *vec1;
    *vec1 = *vec2;
    *vec2 = temp;
    
    return VECTOR_SUCCESS;
}

/* Iterators (index-based) */
size_t vector_begin(ConstVector vec) {
    (void)vec; /* Unused */
    return 0;
}

size_t vector_end(ConstVector vec) {
    return vec ? vec->size : 0;
}

size_t vector_next(ConstVector vec, size_t iterator) {
    (void)vec; /* Unused */
    return iterator + 1;
}

size_t vector_prev(ConstVector vec, size_t iterator) {
    (void)vec; /* Unused */
    return iterator > 0 ? iterator - 1 : 0;
}

/* Searching and sorting */
size_t vector_find(ConstVector vec, const void* value, VectorCompareFunc compare) {
    if (!vec || !value || !compare) return (size_t)-1;
    
    const char* data = (const char*)vec->data;
    for (size_t i = 0; i < vec->size; i++) {
        if (compare(data + i * vec->element_size, value) == 0) {
            return i;
        }
    }
    
    return (size_t)-1;
}

size_t vector_find_if(ConstVector vec, bool (*predicate)(const void*)) {
    if (!vec || !predicate) return (size_t)-1;
    
    const char* data = (const char*)vec->data;
    for (size_t i = 0; i < vec->size; i++) {
        if (predicate(data + i * vec->element_size)) {
            return i;
        }
    }
    
    return (size_t)-1;
}

VectorError vector_sort(Vector vec, VectorCompareFunc compare) {
    if (!vec || !compare) return VECTOR_ERROR_NULL_POINTER;
    
    qsort(vec->data, vec->size, vec->element_size, 
          (int (*)(const void*, const void*))compare);
    
    vec->last_error = VECTOR_SUCCESS;
    return VECTOR_SUCCESS;
}

VectorError vector_reverse(Vector vec) {
    if (!vec) return VECTOR_ERROR_NULL_POINTER;
    
    char* data = (char*)vec->data;
    char* temp = (char*)malloc(vec->element_size);
    if (!temp) {
        vec->last_error = VECTOR_ERROR_ALLOCATION_FAILED;
        return VECTOR_ERROR_ALLOCATION_FAILED;
    }
    
    for (size_t i = 0; i < vec->size / 2; i++) {
        size_t j = vec->size - 1 - i;
        memcpy(temp, data + i * vec->element_size, vec->element_size);
        memcpy(data + i * vec->element_size, data + j * vec->element_size, vec->element_size);
        memcpy(data + j * vec->element_size, temp, vec->element_size);
    }
    
    free(temp);
    vec->last_error = VECTOR_SUCCESS;
    return VECTOR_SUCCESS;
}

/* Memory management utilities */
VectorError vector_ensure_capacity(Vector vec, size_t min_capacity) {
    if (!vec) return VECTOR_ERROR_NULL_POINTER;
    
    if (vec->capacity >= min_capacity) {
        vec->last_error = VECTOR_SUCCESS;
        return VECTOR_SUCCESS;
    }
    
    return vector_grow(vec, min_capacity);
}

void* vector_detach_data(Vector vec, size_t* out_size) {
    if (!vec) return NULL;
    
    void* data = vec->data;
    if (out_size) *out_size = vec->size;
    
    /* Reset vector without freeing data */
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
    
    return data;
}

/* Advanced operations */
VectorError vector_reserve_additional(Vector vec, size_t additional_elements) {
    if (!vec) return VECTOR_ERROR_NULL_POINTER;
    
    if (additional_elements > SIZE_MAX - vec->capacity) {
        vec->last_error = VECTOR_ERROR_ALLOCATION_FAILED;
        return VECTOR_ERROR_ALLOCATION_FAILED;
    }
    
    return vector_reserve(vec, vec->capacity + additional_elements);
}

Vector vector_clone(ConstVector vec, VectorElementCopyFunc copy_func) {
    if (!vec) return NULL;
    
    /* Create new vector with same element size and destroy function */
    Vector new_vec = vector_create_with_capacity(vec->element_size, vec->capacity, 
                                                 vec->destroy_func);
    if (!new_vec) return NULL;
    
    if (copy_func) {
        /* Use copy function for each element */
        const char* src = (const char*)vec->data;
        char* dest = (char*)new_vec->data;
        
        for (size_t i = 0; i < vec->size; i++) {
            /* Get the source element */
            const void* src_element = src + i * vec->element_size;
            
            /* Create a copy of the element */
            void* copy = copy_func(src_element);
            if (!copy) {
                /* Clean up already copied elements */
                for (size_t j = 0; j < i; j++) {
                    if (new_vec->destroy_func) {
                        new_vec->destroy_func(dest + j * vec->element_size);
                    }
                }
                vector_destroy(new_vec);
                return NULL;
            }
            
            /* Store the copy in the new vector */
            memcpy(dest + i * vec->element_size, &copy, vec->element_size);
        }
    } else {
        /* Simple bitwise copy */
        memcpy(new_vec->data, vec->data, vec->size * vec->element_size);
    }
    
    new_vec->size = vec->size;
    return new_vec;
}

VectorError vector_append(Vector dest, ConstVector src) {
    if (!dest || !src) return VECTOR_ERROR_NULL_POINTER;
    if (dest->element_size != src->element_size) return VECTOR_ERROR_INVALID_SIZE;
    
    if (src->size == 0) return VECTOR_SUCCESS;
    
    /* Ensure capacity */
    if (dest->size + src->size > dest->capacity) {
        VectorError err = vector_grow(dest, dest->size + src->size);
        if (err != VECTOR_SUCCESS) return err;
    }
    
    /* Copy elements */
    memcpy((char*)dest->data + dest->size * dest->element_size,
           src->data,
           src->size * src->element_size);
    
    dest->size += src->size;
    dest->last_error = VECTOR_SUCCESS;
    return VECTOR_SUCCESS;
}

/* Utility functions */
bool vector_contains(ConstVector vec, const void* value, VectorCompareFunc compare) {
    return vector_find(vec, value, compare) != (size_t)-1;
}

void vector_for_each(Vector vec, void (*func)(void*)) {
    if (!vec || !func) return;
    
    char* data = (char*)vec->data;
    for (size_t i = 0; i < vec->size; i++) {
        func(data + i * vec->element_size);
    }
}

VectorError vector_remove_if(Vector vec, bool (*predicate)(const void*)) {
    if (!vec || !predicate) return VECTOR_ERROR_NULL_POINTER;
    
    size_t write_pos = 0;
    char* data = (char*)vec->data;
    
    for (size_t read_pos = 0; read_pos < vec->size; read_pos++) {
        void* element = data + read_pos * vec->element_size;
        
        if (!predicate(element)) {
            if (write_pos != read_pos) {
                memmove(data + write_pos * vec->element_size,
                        element,
                        vec->element_size);
            }
            write_pos++;
        } else if (vec->destroy_func) {
            vec->destroy_func(element);
        }
    }
    
    vec->size = write_pos;
    vec->last_error = VECTOR_SUCCESS;
    return VECTOR_SUCCESS;
}
