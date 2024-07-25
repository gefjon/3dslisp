#include "print.h"
#include <stdio.h>
#include <stdbool.h>
#include "lisp_object.h"
#include "sys.h"

void print_fixnum(FILE *, fixnum);
void print_boolean(FILE *, bool);
void print_heap_object(FILE *, heap_object *);

void print_object(FILE *out, lisp_object obj) {
  switch (lisp_object_tag(obj)) {
  case OBJECT_FIXNUM:
    print_fixnum(out, lisp_object_as_fixnum(obj));
    break;
  case OBJECT_BOOLEAN:
    print_boolean(out, lisp_object_as_bool(obj));
    break;
  case OBJECT_HEAP_OBJECT:
    print_heap_object(out, lisp_object_as_pointer(obj));
    break;
  default:
    fail("Unexpected lisp_object_tag in print_object");
  }
}

void print_fixnum(FILE *out, fixnum fix) {
  printf("%d", fixnum_to_s32(fix));
}

void print_boolean(FILE *out, bool b) {
  fputs(b ? "true" : "false", out);
}

void print_cons(FILE *, cons *);
void print_array(FILE *, array *);
void print_string(FILE *, string *);
void print_symbol(FILE *, symbol *);

void print_heap_object(FILE *out, heap_object *obj) {
  switch (obj->type) {
  case HEAP_OBJECT_CONS:
    print_cons(out, &obj->as_cons);
    break;
  case HEAP_OBJECT_ARRAY:
    print_array(out, &obj->as_array);
    break;
  case HEAP_OBJECT_STRING:
    print_string(out, &obj->as_string);
    break;
  case HEAP_OBJECT_SYMBOL:
    print_symbol(out, &obj->as_symbol);
    break;
  default:
    fail("Unexpected heap_object_tag in print_heap_object");
  }
}

void print_cons(FILE *out, cons *cell) {
  fputc('(', out);
  print_object(out, cell->head);
  lisp_object next = cell->tail;
  for (;
       lisp_object_cons_p(next);
       next = lisp_object_as_cons(next)->tail) {
    fputc(' ', out);
    print_object(out, lisp_object_as_cons(next)->head);
  }
  if (next != bool_false) {
    fputs(" . ", out);
    print_object(out, next);
  }
  fputc(')', out);
}

void print_array(FILE *out, array *arr) {
  fputs("#(", out);
  bool need_space = false;
  for (u32 idx = 0; idx < arr->length; ++idx) {
    if (need_space) {
      fputc(' ', out);
    } else {
      need_space = true;
    }
    print_object(out, array_get_unchecked(arr, idx));
  }
  fputc(')', out);
}

void write_string(FILE *out, string *str) {
  for (u32 idx = 0; idx < str->length; ++idx) {
    fputc(string_get_unchecked(str, idx), out);
  }
}

void print_string(FILE *out, string *str) {
  fputc('"', out);
  write_string(out, str);
  fputc('"', out);
}

void print_symbol(FILE *out, symbol *sym) {
  write_string(out, sym->name);
}
