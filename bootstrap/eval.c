#include "lang.h"
#include "object.h"

static Object *eval_literal(expr_t *expr) {
  switch (expr->literal.type) {
  case LITERAL_STRING:
    return world_make_string(expr->literal.value_string);

  case LITERAL_NUMBER:
    return world_make_native_integer(expr->literal.value_number);
  }

  return NULL;
}

Object *eval(expr_t *expr) {
  Object *result = NULL;

  switch (expr->type) {
  case EXPR_LITERAL:
    result = eval_literal(expr);
    break;

  default:
    info("Eval of %d not implemented", expr->type);
  }

  return result;
}
