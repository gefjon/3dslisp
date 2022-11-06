#pragma once

#include <3ds.h>

typedef void *lisp_object;

static const u32 TAG_BITS = 0b11;

typedef enum {
  OBJECT_FIXNUM = 0
  , OBJECT_HEAP_OBJECT = 0b01
  , OBJECT_BOOLEAN = 0b11
  // other object types must always have low bits 0b11, so fixnum can have 31 bits of
  // precision and heap_object can be 4-byte aligned
} object_tag;

object_tag lisp_object_tag(lisp_object);

typedef enum {
  HEAP_OBJECT_CONS
  , HEAP_OBJECT_ARRAY
  , HEAP_OBJECT_STRING
  , HEAP_OBJECT_SYMBOL
  , HEAP_OBJECT_TOMBSTONE = 0xffffffff
} heap_object_tag;

typedef s32 fixnum;

typedef struct {
  lisp_object head;
  lisp_object tail;
} cons;

typedef struct {
  u32 length;
  lisp_object contents[];
} array;

typedef struct {
  u32 length;
  char contents[];
} string;

typedef struct {
  string *name;
} symbol;

typedef struct {
  heap_object_tag type;
  union {
    cons as_cons;
    array as_array;
    string as_string;
    symbol as_symbol;
    lisp_object as_tombstone;
  };
} heap_object;

bool lisp_object_pointer_p(lisp_object);
heap_object *lisp_object_as_pointer(lisp_object);
heap_object *inner_pointer_as_heap_object(void *);
lisp_object tag_heap_object(heap_object *);

fixnum lisp_object_as_fixnum(lisp_object);
bool lisp_object_as_bool(lisp_object);
cons *lisp_object_as_cons(lisp_object);
array *lisp_object_as_array(lisp_object);
string *lisp_object_as_string(lisp_object);
symbol *lisp_object_as_symbol(lisp_object);

bool lisp_object_fixnum_p(lisp_object);
bool lisp_object_bool_p(lisp_object);
bool lisp_object_cons_p(lisp_object);
bool lisp_object_array_p(lisp_object);
bool lisp_object_string_p(lisp_object);
bool lisp_object_symbol_p(lisp_object);

lisp_object tag_fixnum(fixnum);
lisp_object tag_bool(bool);
lisp_object tag_cons(cons *);
lisp_object tag_array(array *);
lisp_object tag_string(string *);
lisp_object tag_symbol(symbol *);

s32 fixnum_to_s32(fixnum);
fixnum s32_to_fixnum(s32);

static const lisp_object bool_false = (lisp_object) OBJECT_BOOLEAN;
static const lisp_object bool_true = (lisp_object) ((TAG_BITS + 1) | OBJECT_BOOLEAN);

lisp_object array_get_unchecked(array *, u32);
void array_set_unchecked(array *, u32, lisp_object);

char string_get_unchecked(string *, u32);
void string_set_unchecked(string *, u32, char);
