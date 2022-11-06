#pragma once

#include <3ds.h>
#include "lisp_object.h"

void gc_init(u32 arena_size);

array *allocate_array(lisp_object *live_stack,
                      u32 live_stack_size,
                      u32 length,
                      lisp_object initial_element);

string *allocate_string(lisp_object *live_stack,
                        u32 live_stack_size,
                        u32 length,
                        char initial_char);

cons *allocate_cons(lisp_object *live_stack,
                    u32 live_stack_size,
                    lisp_object head,
                    lisp_object tail);

symbol *allocate_symbol(lisp_object *live_stack,
                        u32 live_stack_size,
                        lisp_object name);

void garbage_collect(lisp_object *live_stack, u32 live_stack_size);
