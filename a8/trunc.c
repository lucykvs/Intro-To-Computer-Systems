#include <stdlib.h>
#include <stdio.h>
#include "list.h"
#include <string.h>


void print_int (element_t ev) {
  int* e = (int*) ev;
  printf("%d\n", e);
}

void print_str (element_t ev) {
  char* e = (char*) ev;
  printf("%s\n", e);
}

void print_with_spaces (element_t ev) {
  char* e = (char*) ev;
  printf("%s", e);
  printf(" ");
}

void parse_int(element_t *out, element_t in) {
  char *endp;
  char *str = (char*) in;
  int* i = (int*) out;
  /* strtol returns a pointer to the first non-numeric character in endp.
     If it gets to the end of the string, that character will be the null terminator. */
  *i = strtol(str, &endp, 10);
  if(str == endp) {
    /* String was non-empty and not an integer */
    *i = -1;
  }
}

void to_null(element_t* out, element_t str_in, element_t int_in){
    char** strout = (char**) out;
    char* strin = (char*) str_in;
    int intin = (int) int_in;
    
    if(intin >= 0) *strout = NULL;
    else *strout = strin;
}

int positive(element_t int_in){
    int intin = (int) int_in;
    return intin >= 0;
}

int is_null(element_t str_in){
    char* strin = (char*) str_in;
    return strin != NULL;
}

void truncate(element_t* out, element_t str_in, element_t int_in){
    char** strout = (char**) out;
    char* strin = (char*) str_in;
    int intin = (int) int_in;

    if (strlen(strin) <= intin) *strout = strin;
    else {
        strin[intin] = '\0';
        *strout = strin;
    }
}

// void concatenate(element_t* str_out_ptr, element_t str_out, element_t str_in){
//     char** stroutptr = (char**) str_out_ptr; 
//     char* strout = (char*) str_out;
//     char* strin = (char*) str_in;
//     int new_len = strlen(strin) + strlen(strout) + 20;
//     strout = (char*) realloc(strout, new_len);
//     if (strlen(strin) != 0) {
//         strcat(strout, strin);
//     }
// }

void max(element_t* int_out_ptr, element_t int_out, element_t int_in){
    int* intoutptr = (int*) int_out_ptr;
    int intout = (int) int_out;
    int intin = (int) int_in;
    *intoutptr = intout > intin ? intout : intin; 
}

int main(int argc, char **argv) {
  struct list* arg_list = list_create();

  /* Parse input arguments */
  for (int i=0; i<argc-1; i++) {
    element_t e = (element_t) argv[i+1];
    list_append(arg_list, e);
  }

//   // testing argument list
//   list_foreach(print_str, arg_list);

  struct list* l0 = list_create();
  list_map1(parse_int, l0, arg_list);

  // testing int list
//   list_foreach(print_int, l0);

//   // map integers in arg_list to null
  struct list* l1 = list_create();
  list_map2(to_null, l1, arg_list, l0);

  struct list* l2 = list_create();
  list_filter(positive, l2, l0);

  // testing positive int list
//   list_foreach(print_int, l2);

  struct list* l3 = list_create();
  list_filter(is_null, l3, l1);

  // testing filtered string list
//   list_foreach(print_str, l3);

  // create and print list of truncated strings
  struct list* l4 = list_create();
  list_map2(truncate, l4, l3, l2);
  list_foreach(print_str, l4);

  list_foreach(print_with_spaces, l4);
  printf("\n");
//   char* s = " ", *sp = &s;
//   list_foldl(concatenate, (element_t*) &sp, l4);
//   printf("%s\n", s); 

  element_t i = 0;
  list_foldl(max, &i, l2);
  printf("%d\n", i);

  list_destroy (arg_list);
  list_destroy (l0);
  list_destroy (l1);
  list_destroy (l2);
  list_destroy (l3);
  list_destroy (l4);
}

