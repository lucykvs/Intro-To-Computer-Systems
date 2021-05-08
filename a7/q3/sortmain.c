#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "int_element.h"
#include "str_element.h"
#include "element.h"
#include "refcount.h"

/* If the string is numeric, return an int_element. Otherwise return a str_element. */
struct element *parse_string(char *str) {
  char *endp;
  /* strtol returns a pointer to the first non-numeric character in endp.
     If it gets to the end of the string, that character will be the null terminator. */
  int value = strtol(str, &endp, 10);
  if(str[0] != '\0' && endp[0] == '\0') {
    /* String was non-empty and strtol conversion succeeded - integer */
    return (struct element *)int_element_new(value);
  } else {
    return (struct element *)str_element_new(str);
  }
}

int compare(const void* arg1, const void* arg2) {
    struct element** elem1 = (struct element**) arg1; 
    struct element** elem2 = (struct element**) arg2;

    if (is_int_element(*elem1) && !is_int_element(*elem2)) {
      return -1;
    } 
    if (is_str_element(*elem1) && !is_str_element(*elem2)) {
      return 1;
    }
    if (is_int_element(*elem1)) {
      return (int_element_get_value((struct int_element*)(*elem1)) - int_element_get_value((struct int_element*)(*elem2)));
    }
    if (is_str_element(*elem1)) {
      return strcmp(str_element_get_value((struct str_element*)(*elem1)), str_element_get_value((struct str_element*)(*elem2)));
    }
}

int main(int argc, char **argv) {
  /* TODO: Read elements into a new array using parse_string */
  struct element **array;
  array = malloc(sizeof(struct element *) * argc-1);

  for (int i=0; i<argc-1; i++) {
    struct element *e = parse_string(argv[i+1]);
    array[i] = e;
  }
  /* TODO: Sort elements with qsort */
  qsort((void*)array, argc-1, sizeof(struct element *), compare);
  printf("Sorted: ");
  /* TODO: Print elements, separated by a space */
  for (int i = 0; i < argc - 1; i++) {
    if (is_int_element(array[i])){
      struct int_element* int_element = (struct int_element*) array[i];
      printf("%d ", int_element_get_value(int_element));
    } else {
      struct str_element* str_element = (struct str_element*) array[i];
        printf("%s ", str_element_get_value(str_element));
    }
    
  }
  printf("\n");

  for (int i = 0; i < argc - 1; i++) {
    rc_free_ref(array[i]);
  }
  free(array);
}
