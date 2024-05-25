#include <assert.h>

#include "lang.h"
#include "object.h"
#include "builtins.h"

static Object *eval_literal(expr_t *expr, Object *scope_) {
  (void)scope_;

  switch (expr->literal.type) {
  case LITERAL_STRING:
    return string_new(expr->literal.value_string);

  case LITERAL_NUMBER:
    return native_integer_new(expr->literal.value_number);
  }

  return NULL;
}

static Object *eval_send(expr_t *expr, Object *scope_) {
  Object *receiver = eval(expr->send.receiver, scope_);

  if (expr->send.selector->type != EXPR_IDENTIFIER) {
    panic("Expected selector of send expression to be an identifier.");
  }

  // Eval each argument
  array_t *args = array_new_with_capacity(expr->send.args->size);

  for (int i = 0; i < expr->send.args->size; i++) {
    Object *arg = eval(expr->send.args->elements[i], scope_);
    array_append(args, arg);
  }

  char *selector = expr->send.selector->identifier.name;
  Object *ret = send_args(receiver, selector, args);
  array_free(args);

  return ret;
}

static Object *eval_binary_send(expr_t *expr, Object *scope_) {
  if (expr->binary_send.selector->type != EXPR_IDENTIFIER) {
    panic("Expected selector of binary send expression to be an identifier.");
  }

  Object *receiver = eval(expr->binary_send.left, scope_);
  Object *other = eval(expr->binary_send.right, scope_);
  char *selector = expr->binary_send.selector->identifier.name;

  return send(receiver, selector, other);
}

static Object *eval_identifier(expr_t *expr, Object *scope_) {
  Scope *scope = (Scope *)scope_;
  bool found = false;

  Object *obj = scope_lookup(scope, symbol_intern(expr->identifier.name), &found);
  if (!found) {
    panic("%s not found in current scope", expr->identifier.name);
  }

  return obj;
}

Object *eval(expr_t *expr, Object *scope) {
  Object *result = NULL;

  switch (expr->type) {
  case EXPR_LITERAL:
    result = eval_literal(expr, scope);
    break;

  case EXPR_SEND:
    result = eval_send(expr, scope);
    break;

  case EXPR_BINARY_SEND:
    result = eval_binary_send(expr, scope);
    break;

  case EXPR_IDENTIFIER:
    result = eval_identifier(expr, scope);
    break;

  default:
    info("Eval of %d not implemented", expr->type);
  }

  return result;
}
