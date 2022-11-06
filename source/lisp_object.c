#include <3ds.h>
#include "lisp_object.h"

object_tag lisp_object_tag(lisp_object obj) {
  u32 tagged_addr = (u32) obj;
  if (!(tagged_addr & 1)) {
    return OBJECT_FIXNUM;
  } else {
    return tagged_addr & TAG_BITS;
  }
}

bool lisp_object_pointer_p(lisp_object obj) {
  return lisp_object_tag(obj) == OBJECT_HEAP_OBJECT;
}

heap_object *lisp_object_as_pointer(lisp_object obj) {
  u32 tagged_addr = (u32) obj;
  u32 addr = tagged_addr & (~TAG_BITS);
  return (void *) addr;
}

lisp_object tag_heap_object(heap_object *obj) {
  u32 addr = (u32) obj;
  u32 tagged_addr = addr | OBJECT_HEAP_OBJECT;
  return (lisp_object) tagged_addr;
}

heap_object_tag get_heap_object_tag(heap_object *obj) {
  return obj->type;
}

heap_object *inner_pointer_as_heap_object(void *inner_addr) {
  void *outer_addr = inner_addr - sizeof(object_tag);
  return outer_addr;
}

#define x_each_heap_object          \
  X(array, HEAP_OBJECT_ARRAY);      \
  X(cons, HEAP_OBJECT_CONS);        \
  X(string, HEAP_OBJECT_STRING);    \
  X(symbol, HEAP_OBJECT_SYMBOL);

#define X(TYPE_NAME, WANTED_TAG)                              \
  bool lisp_object_ ## TYPE_NAME ## _p (lisp_object obj) {    \
    if (lisp_object_pointer_p(obj)) {                         \
      return lisp_object_as_pointer(obj)->type == WANTED_TAG; \
    } else {                                                  \
      return false;                                           \
    }                                                         \
  }
x_each_heap_object
#undef X

#define X(TYPE_NAME, WANTED_TAG)                              \
  TYPE_NAME *lisp_object_as_ ## TYPE_NAME (lisp_object obj) { \
    return &lisp_object_as_pointer(obj)->as_ ## TYPE_NAME ;   \
  }
x_each_heap_object
#undef X

#define X(TYPE_NAME, WANTED_TAG)                                \
  lisp_object tag_ ## TYPE_NAME (TYPE_NAME *obj) {              \
    heap_object *base_addr = inner_pointer_as_heap_object(obj); \
    return tag_heap_object(base_addr);                          \
  }
x_each_heap_object
#undef X
  
lisp_object array_get_unchecked(array *arr, u32 index) {
  return arr->contents[index];
}

void array_set_unchecked(array *arr, u32 index, lisp_object new_value) {
  arr->contents[index] = new_value;
}

char string_get_unchecked(string *str, u32 index) {
  return str->contents[index];
}

void string_set_unchecked(string *str, u32 index, char new_value) {
  str->contents[index] = new_value;
}

bool lisp_object_fixnum_p(lisp_object fix) {
  return lisp_object_tag(fix) == OBJECT_FIXNUM;
}

fixnum lisp_object_as_fixnum(lisp_object fix) {
  return (fixnum) fix;
}

lisp_object tag_fixnum(fixnum fix) {
  return (lisp_object) fix;
}

s32 fixnum_to_s32(fixnum fix) {
  return fix >> 1;
}

fixnum s32_to_fixnum(s32 num) {
  return num << 1;
}

bool lisp_object_bool_p(lisp_object b) {
  return lisp_object_tag(b) == OBJECT_BOOLEAN;
}


bool lisp_object_as_bool(lisp_object b) {
  return b != bool_false;
}
