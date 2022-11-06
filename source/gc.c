#include <3ds.h>
#include "lisp_object.h"
#include "gc.h"
#include "sys.h"
#include <string.h>
#include <stdlib.h>

void *active_gc_arena;
void *gc_alloc_pointer;
void *gc_end_pointer;
void *waiting_gc_arena;

void *alloc_arena(u32 arena_size) {
  void *new_arena = malloc(arena_size);
  if (!new_arena) {
    fail("Could not allocate GC arena");
  }
  return new_arena;
}

void gc_init(u32 arena_size) {
  active_gc_arena = alloc_arena(arena_size);
  waiting_gc_arena = alloc_arena(arena_size);
  gc_alloc_pointer = active_gc_arena;
  gc_end_pointer = active_gc_arena + arena_size;
}

u32 increment_to_align_4(u32 addr) {
  return (addr + 3) & (~ 3);
}

heap_object  *allocate_without_gc(u32 length_in_bytes) {
  void * new_object = gc_alloc_pointer;
  // FIXME: ensure gc_alloc_pointer remains aligned
  gc_alloc_pointer += length_in_bytes;
  return new_object;  
}

heap_object *allocate(lisp_object *live_stack, u32 live_stack_size, u32 length_in_bytes) {
  if ((gc_alloc_pointer + length_in_bytes) > gc_end_pointer) {
    garbage_collect(live_stack, live_stack_size);
    if ((gc_alloc_pointer + length_in_bytes) > gc_end_pointer) {
      fail("Insufficient heap size for allocation after garbage collection");
    }
  }
  return allocate_without_gc(length_in_bytes);
}

const u32 heap_object_header_size = sizeof(heap_object_tag);

u32 array_total_size(u32 length) {
  return increment_to_align_4(heap_object_header_size + sizeof(array) + (length * sizeof(lisp_object)));
}

array *allocate_array(lisp_object *live_stack,
                      u32 live_stack_size,
                      u32 length,
                      lisp_object initial_element) {
  heap_object *new_array = allocate(live_stack, live_stack_size, array_total_size(length));
  new_array->type = HEAP_OBJECT_ARRAY;
  new_array->as_array.length = length;
  for (u32 idx = 0; idx < length; ++idx) {
    new_array->as_array.contents[idx] = initial_element;
  }
  return &new_array->as_array;
}

u32 string_total_size(u32 length) {
  return increment_to_align_4(heap_object_header_size + sizeof(string) + length);
}

const u32 cons_size = heap_object_header_size + sizeof(cons);
const u32 symbol_size = heap_object_header_size + sizeof(symbol);

string *allocate_string(lisp_object *live_stack,
                        u32 live_stack_size,
                        u32 length,
                        char initial_char) {
  heap_object *new_string = allocate(live_stack, live_stack_size, string_total_size(length));
  new_string->as_string.length = length;
  for (u32 idx = 0; idx < length; ++idx) {
    new_string->as_string.contents[idx] = initial_char;
  }
  return &new_string->as_string;
}

bool tombstone_p(heap_object *obj) {
  return (obj->type == HEAP_OBJECT_TOMBSTONE);
}

lisp_object tombstone_forward(heap_object *obj) {
  return obj->as_tombstone;
}

void make_tombstone(heap_object *obj, lisp_object new) {
  obj->type = HEAP_OBJECT_TOMBSTONE;
  obj->as_tombstone = new;
}

u32 heap_object_size(heap_object *obj) {
  switch (obj->type) {
  case HEAP_OBJECT_ARRAY:
    return array_total_size(obj->as_array.length);
  case HEAP_OBJECT_STRING:
    return string_total_size(obj->as_string.length);
  default:
    fail("Unexpected heap object tag in heap_object_size during gc");
  }
}

lisp_object move_object(heap_object *obj) {
  u32 size = heap_object_size(obj);
  heap_object *new = allocate_without_gc(size);
  memcpy(new, obj, size);
  lisp_object new_obj = tag_heap_object(new);
  make_tombstone(obj, new_obj);
  return new_obj;
}

lisp_object maybe_move_object(lisp_object obj) {
  if (lisp_object_pointer_p(obj)) {
    heap_object *ptr = lisp_object_as_pointer(obj);
    if (tombstone_p(ptr)) {
      return tombstone_forward(ptr);
    } else {
      return move_object(ptr);
    }
  } else {
    // no need to move fixnums (or other non-heap-allocated objects, once we get them)
    return obj;
  }
}

void scan_live_stack(lisp_object *live_stack, u32 size) {
  for (u32 idx = 0; idx < size; ++idx) {
    live_stack[idx] = maybe_move_object(live_stack[idx]);
  }
}

void move_within_cons(cons *cell) {
  cell->head = maybe_move_object(cell->head);
  cell->tail = maybe_move_object(cell->tail);
}

void move_within_array(array *arr) {
  for (u32 idx = 0; idx < arr->length; ++idx) {
    arr->contents[idx] = maybe_move_object(arr->contents[idx]);
  }
}

void move_within_symbol(symbol *sym) {
  sym->name = lisp_object_as_string(maybe_move_object(tag_string(sym->name)));
}

void move_objects_within(heap_object *obj) {
  switch (obj->type) {
  case HEAP_OBJECT_CONS:
    move_within_cons(&obj->as_cons);
    break;
  case HEAP_OBJECT_ARRAY:
    move_within_array(&obj->as_array);
    break;
  case HEAP_OBJECT_STRING:
    break;
  case HEAP_OBJECT_SYMBOL:
    move_within_symbol(&obj->as_symbol);
  default:
    fail("Unexpected heap object tag in move_objects_within during gc");
  }
}

void scan_new_arena(void *scan_pointer) {
  while (scan_pointer < gc_alloc_pointer) {
    heap_object *obj = scan_pointer;
    u32 size = heap_object_size(obj);
    move_objects_within(obj);
    scan_pointer += size;
  }
}

void garbage_collect(lisp_object *live_stack, u32 live_stack_size) {
  void *scan_pointer = waiting_gc_arena;
  waiting_gc_arena = active_gc_arena;
  active_gc_arena = scan_pointer;
  gc_alloc_pointer = active_gc_arena;
  scan_live_stack(live_stack, live_stack_size);
  scan_new_arena(scan_pointer);
}
