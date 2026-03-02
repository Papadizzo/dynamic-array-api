#include "vector.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Example: Integer vector */
void destroy_int(void* element) {
    /* Integers don't need special destruction */
    (void)element;
}

int compare_int(const void* a, const void* b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return (ia > ib) - (ia < ib);
}

void print_int(void* element) {
    printf("%d ", *(int*)element);
}

/* Example: String vector */
void destroy_string(void* element) {
    /* element points to the memory where the string pointer is stored */
    char** str_ptr = (char**)element;
    if (str_ptr && *str_ptr) {
        printf("Freeing: %s\n", *str_ptr);  /* Debug output */
        free(*str_ptr);
        *str_ptr = NULL;
    }
}

void* copy_string(const void* element) {
    /* element points to the memory where the source string pointer is stored */
    const char** src_str_ptr = (const char**)element;
    if (!src_str_ptr || !*src_str_ptr) return NULL;
    
    const char* src_str = *src_str_ptr;
    printf("Copying: %s\n", src_str);  /* Debug output */
    
    /* Create a new string */
    char* new_str = malloc(strlen(src_str) + 1);
    if (new_str) {
        strcpy(new_str, src_str);
    }
    
    /* Return the new string pointer - this will be stored in the vector */
    return new_str;
}

int compare_string(const void* a, const void* b) {
    /* a and b are pointers to the memory where string pointers are stored */
    const char** sa = (const char**)a;
    const char** sb = (const char**)b;
    return strcmp(*sa, *sb);
}

void print_string(void* element) {
    /* element points to the memory where the string pointer is stored */
    const char** str_ptr = (const char**)element;
    if (str_ptr && *str_ptr) {
        printf("\"%s\" ", *str_ptr);
    }
}

/* Predicate function for removing strings longer than 5 characters */
bool is_string_longer_than_5(const void* element) {
    /* element points to the memory where the string pointer is stored */
    const char** str_ptr = (const char**)element;
    if (!str_ptr || !*str_ptr) return false;
    return strlen(*str_ptr) > 5;
}

int main() {
    printf("=== Integer Vector Example ===\n");
    
    /* Create integer vector */
    Vector int_vec = vector_create(sizeof(int), destroy_int);
    if (!int_vec) {
        fprintf(stderr, "Failed to create integer vector\n");
        return 1;
    }
    
    /* Push back some values */
    int values[] = {5, 2, 8, 1, 9, 3};
    for (size_t i = 0; i < 6; i++) {
        VectorError err = vector_push_back(int_vec, &values[i]);
        if (err != VECTOR_SUCCESS) {
            fprintf(stderr, "Failed to push back value %d: %s\n", 
                    values[i], vector_error_string(err));
        }
    }
    
    printf("Vector size: %zu, capacity: %zu\n", 
           vector_size(int_vec), vector_capacity(int_vec));
    
    /* Print elements */
    printf("Elements: ");
    vector_for_each(int_vec, print_int);
    printf("\n");
    
    /* Access elements */
    int* front = (int*)vector_front(int_vec);
    int* back = (int*)vector_back(int_vec);
    if (front && back) {
        printf("Front: %d, Back: %d\n", *front, *back);
    }
    
    /* Sort */
    VectorError err = vector_sort(int_vec, compare_int);
    if (err == VECTOR_SUCCESS) {
        printf("Sorted: ");
        vector_for_each(int_vec, print_int);
        printf("\n");
    }
    
    /* Insert at position */
    int new_val = 4;
    err = vector_insert(int_vec, 3, &new_val);
    if (err == VECTOR_SUCCESS) {
        printf("After insert at position 3: ");
        vector_for_each(int_vec, print_int);
        printf("\n");
    }
    
    /* Erase element */
    err = vector_erase(int_vec, 2);
    if (err == VECTOR_SUCCESS) {
        printf("After erase at position 2: ");
        vector_for_each(int_vec, print_int);
        printf("\n");
    }
    
    /* Find element */
    int search_val = 5;
    size_t pos = vector_find(int_vec, &search_val, compare_int);
    if (pos != (size_t)-1) {
        printf("Found %d at position %zu\n", search_val, pos);
    }
    
    /* Reverse */
    err = vector_reverse(int_vec);
    if (err == VECTOR_SUCCESS) {
        printf("Reversed: ");
        vector_for_each(int_vec, print_int);
        printf("\n");
    }
    
    /* Cleanup */
    vector_destroy(int_vec);
    
    printf("\n=== String Vector Example ===\n");
    
    /* Create string vector */
    Vector str_vec = vector_create(sizeof(char*), destroy_string);
    if (!str_vec) {
        fprintf(stderr, "Failed to create string vector\n");
        return 1;
    }
    
    /* Add strings */
    const char* strings[] = {"apple", "banana", "cherry", "date"};
    for (size_t i = 0; i < 4; i++) {
        char* str_copy = malloc(strlen(strings[i]) + 1);
        if (str_copy) {
            strcpy(str_copy, strings[i]);
            printf("Pushing: %s at address %p\n", str_copy, (void*)str_copy);
            
            /* Push the pointer to the string */
            VectorError err = vector_push_back(str_vec, &str_copy);
            if (err != VECTOR_SUCCESS) {
                fprintf(stderr, "Failed to push back string: %s\n", 
                        vector_error_string(err));
                free(str_copy);
            }
        }
    }
    
    printf("\nOriginal strings: ");
    vector_for_each(str_vec, print_string);
    printf("\n");
    
    /* Test direct access */
    char** first = (char**)vector_at(str_vec, 0);
    if (first && *first) {
        printf("First string directly: \"%s\" at address %p\n", *first, (void*)*first);
    }
    
    /* Clone vector */
    printf("\n--- Cloning vector ---\n");
    Vector str_vec2 = vector_clone(str_vec, copy_string);
    if (str_vec2) {
        printf("Cloned strings: ");
        vector_for_each(str_vec2, print_string);
        printf("\n");
        
        /* Sort cloned vector */
        printf("Sorting cloned vector...\n");
        err = vector_sort(str_vec2, compare_string);
        if (err == VECTOR_SUCCESS) {
            printf("Sorted cloned strings: ");
            vector_for_each(str_vec2, print_string);
            printf("\n");
        }
        
        printf("Destroying cloned vector...\n");
        vector_destroy(str_vec2);
    } else {
        printf("Failed to clone vector\n");
    }
    
    /* Remove if (strings longer than 5 characters) */
    printf("\n--- Testing remove_if ---\n");
    printf("Before remove_if: ");
    vector_for_each(str_vec, print_string);
    printf("\n");
    
    err = vector_remove_if(str_vec, is_string_longer_than_5);
    if (err == VECTOR_SUCCESS) {
        printf("After removing strings longer than 5 chars: ");
        vector_for_each(str_vec, print_string);
        printf("\n");
    }
    
    printf("Destroying original vector...\n");
    vector_destroy(str_vec);
    
    return 0;
}