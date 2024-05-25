#include <assert.h>
#include <stdio.h>

#include "builtins.h"
#include "lib.h"

static VTable *symbol_vt;
static string_table_t *global_symbol_table;

static Object *symbol_new(char *string) {
  Symbol *sym = (Symbol *)vtable_allocate(symbol_vt);
  sym->string = string;
  return (Object *)sym;
}

Object *symbol_intern(char *string) {
  location_t loc;
  Object *sym;

  if (string_table_lookup(global_symbol_table, string, &loc)) {
    sym = (Object *)string_table_get(global_symbol_table, loc);
  } else {
    sym = symbol_new(string);
    string_table_set(global_symbol_table, loc, string, sym);
  }

  return sym;
}

Object *symbol_intern_(Object *self, String *string) {
  (void)self;

  assert(string->buf[string->len] == '\0');
  return symbol_intern(string->buf);
}

Object *symbol_inspect(Object *self) {
  Symbol *self_ = (Symbol *)self;
  char *buf;
  asprintf(&buf, "<Symbol '%s'>", self_->string);
  return string_new(buf);
}

method_descriptor_t Symbol_methods[] = {
  { .name = "intern",  .fn = symbol_intern_ },
  { .name = "inspect", .fn = symbol_inspect },
  { NULL },
};

VTable *symbol_bootstrap() {
  global_symbol_table = string_table_new();

  symbol_vt = vtable_delegated(object_vt, sizeof(Symbol));
  vtable_add_method_descriptors(symbol_vt, Symbol_methods);

  return symbol_vt;
}
