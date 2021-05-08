#include "int_element.h"
#include "str_element.h"
#include <stdlib.h>
#include <stdio.h>
#include "refcount.h"
#include "element.h"

/* TODO: Implement all public int_element functions, including element interface functions.

You may add your own private functions here too. */

struct int_element {
    struct int_element_class *class;
    int value;
};

int int_element_get_value(struct int_element *int_elem){
    return int_elem->value;
}


void int_print(struct int_element *int_elem){
    printf("%d\n", int_elem->value);
}

int int_compare(struct element *elem1, struct element *elem2) {
    if (is_int_element(elem1) && !is_int_element(elem2)) {
        return -1;
    }
    if (!is_int_element(elem1) && is_int_element(elem2)) {
        return 1;
    }
    if (is_int_element(elem1)) {
        struct int_element* int_elem1 = (struct int_element*) elem1;
        struct int_element* int_elem2 = (struct int_element*) elem2;
        int diff = int_elem1->value - int_elem2->value;
    if (diff < 0) {
        return -1;
    } else if (diff > 0) {
        return 1;
    } else {
        return 0;
        }
    }
    
} 

struct element_class element_class;

struct int_element_class {
    struct element_class super_class;
    int (*get_value)(struct int_element *);
    int (*is_int_element)(struct element *);
};

struct int_element_class class_obj = {
    int_print,
    int_compare,
    int_element_get_value,
    is_int_element

};

int is_int_element(struct element *elem){
    return (elem->class == &class_obj);
}

void int_elem_finalizer(struct int_element *int_elem) {

}

struct int_element *int_element_new(int value) {
    struct int_element *elem = rc_malloc(sizeof(struct int_element), int_elem_finalizer);
    elem->class = &class_obj;
    elem->value = value;
    return elem;
}
