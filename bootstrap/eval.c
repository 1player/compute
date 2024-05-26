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

static Object *eval_assignment(expr_t *expr, Object *scope_) {
  Scope *scope = (Scope *)scope_;

  if (expr->assignment.left->type != EXPR_IDENTIFIER) {
    panic("Expected left-hand side of assignment to be a variable name.");
  }

  char *var_name = expr->assignment.left->identifier.name;
  Object *var = symbol_intern(var_name);
  bool found;
  scope_lookup(scope, var, &found);

  if (!found && !expr->assignment.definition) {
    panic("Trying to assign to undefined variable '%s'", var_name);
  } else if (found && expr->assignment.definition) {
    panic("Trying to redefine variable '%s'", var_name);
  }

  Object *obj = eval(expr->assignment.right, scope_);
  scope_add(scope, var, obj);

  return obj;
}

static Object *eval_block(expr_t *expr, Object *scope_) {
  Scope *scope = (Scope *)scope_;
  Scope *inner_scope = scope_new(scope);

  Object *obj = NULL;
  array_foreach(expr->block.exprs, expr_t *, inner_expr) {
    obj = eval(inner_expr, (Object *)inner_scope);
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

  case EXPR_ASSIGNMENT:
    result = eval_assignment(expr, scope);
    break;

  case EXPR_BLOCK:
    result = eval_block(expr, scope);
    break;

  default:
    info("Eval of %d not implemented", expr->type);
  }

  return result;
}
