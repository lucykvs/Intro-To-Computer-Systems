#include <stdio.h>
#include <stdlib.h>
#include "str_element.h"
#include "element.h"
#include "refcount.h"
#include <string.h>
/* TODO: Implement all public str_element functions, including element interface functions.

You may add your own private functions here too. */
struct str_element {
    struct str_element_class *class;
    char *value;
};

struct element_class element_class;

struct str_element_class {
    struct element_class super_class;
    char (*str_element_get_value)(struct str_element *);
    int (*is_str_element)(struct element *);
};

void str_print(struct str_element *str_element){
    printf("%s\n", str_element->value);
}

int str_compare(struct element *elem1, struct element *elem2) {
    if (!is_str_element(elem1) && is_str_element(elem2)) {
        return -1;
    }
    if (is_str_element(elem1) && !is_str_element(elem2)) {
        return 1;
    }
    return strcmp(str_element_get_value((struct str_element*)(elem1)), str_element_get_value((struct str_element*)(elem2)));
}

char *str_element_get_value(struct str_element *str_element) {
    return str_element->value;
}

struct str_element_class str_class_obj = {
    str_print,
    str_compare,
    str_element_get_value,
    is_str_element
};

int is_str_element(struct element *element) {
    return (element->class == &str_class_obj);
}


void str_element_finalizer(struct str_element *str_elem) {
    free(str_elem->value);
}
struct str_element *str_element_new(char *value) {
    struct str_element *str_elem = rc_malloc(sizeof(struct str_element), str_element_finalizer);
    str_elem->class = &str_class_obj;
    str_elem->value = strdup(value);
    return str_elem;
}