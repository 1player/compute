#include <assert.h>

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

static Object *eval_send(expr_t *expr) {
  Object *receiver = eval(expr->send.receiver);
  assert(receiver != NULL);

  if (expr->send.selector->type != EXPR_IDENTIFIER) {
    panic("Expected selector of send expression to be an identifier.");
  }

  // Eval each argument
  array_t args;
  array_init(&args);

  for (int i = 0; i < expr->send.args->size; i++) {
    Object *arg = eval(expr->send.args->elements[i]);
    array_append(&args, arg);
  }

  char *selector = expr->send.selector->identifier.name;
  return send_args(receiver, selector, &args);
}

Object *eval(expr_t *expr) {
  Object *result = NULL;

  switch (expr->type) {
  case EXPR_LITERAL:
    result = eval_literal(expr);
    break;

  case EXPR_SEND:
    result = eval_send(expr);
    break;

  default:
    info("Eval of %d not implemented", expr->type);
  }

  return result;
}
