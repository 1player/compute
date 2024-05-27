#include <assert.h>

#include "lang.h"
#include "object.h"
#include "builtins.h"

static object *eval_literal(expr_t *expr, object *scope_) {
  (void)scope_;

  switch (expr->literal.type) {
  case LITERAL_STRING:
    return string_new(expr->literal.value_string);

  case LITERAL_NUMBER:
    return native_integer_new(expr->literal.value_number);
  }

  return NULL;
}

static object *eval_send(expr_t *expr, object *scope_) {
  object *receiver = eval(expr->send.receiver, scope_);

  if (expr->send.selector->type != EXPR_IDENTIFIER) {
    panic("Expected selector of send expression to be an identifier.");
  }

  // Eval each argument
  array_t *args = array_new_with_capacity(expr->send.args->size);

  for (int i = 0; i < expr->send.args->size; i++) {
    object *arg = eval(expr->send.args->elements[i], scope_);
    array_append(args, arg);
  }

  char *selector = expr->send.selector->identifier.name;
  object *ret = send_args(receiver, intern(selector), args->size, (object **)args->elements);
  array_free(args);

  return ret;
}

static object *eval_binary_send(expr_t *expr, object *scope_) {
  if (expr->binary_send.selector->type != EXPR_IDENTIFIER) {
    panic("Expected selector of binary send expression to be an identifier.");
  }

  object *receiver = eval(expr->binary_send.left, scope_);
  object *other = eval(expr->binary_send.right, scope_);
  char *selector = expr->binary_send.selector->identifier.name;

  return send(receiver, intern(selector), other);
}

static object *eval_identifier(expr_t *expr, object *scope) {
  object *name = intern(expr->identifier.name);
  return send(scope, name);
}

static object *eval_assignment(expr_t *expr, object *scope) {
  if (expr->assignment.left->type != EXPR_IDENTIFIER) {
    panic("Expected left-hand side of assignment to be a variable name.");
  }

  char *var_name = expr->assignment.left->identifier.name;
  object *var = intern(var_name);
  bool found = object_lookup(scope, var) != NULL;

  if (!found && !expr->assignment.definition) {
    panic("Trying to assign to undefined variable '%s'", var_name);
  } else if (found && expr->assignment.definition) {
    panic("Trying to redefine variable '%s'", var_name);
  }

  object *obj = eval(expr->assignment.right, scope);
  object_set_variable(scope, var, obj);

  return obj;
}

static object *eval_block(expr_t *expr, object *scope) {
  object *inner_scope = object_derive(scope, sizeof(object));

  object *obj = NULL;
  array_foreach(expr->block.exprs, expr_t *, inner_expr) {
    obj = eval(inner_expr, inner_scope);
  }

  return obj;
}

static object *eval_conditional(expr_t *expr, object *scope_) {
  object *test = eval(expr->conditional.test, scope_);

  // Only nil and `false` are false
  if (test != NULL && test != singleton_false) {
    return eval(expr->conditional.if_block, scope_);
  }

  if (expr->conditional.else_expr) {
    return eval(expr->conditional.else_expr, scope_);
  }

  return NULL;
}

object *eval(expr_t *expr, object *scope) {
  object *result = NULL;

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

  case EXPR_CONDITIONAL:
    result = eval_conditional(expr, scope);
    break;

  default:
    info("Eval of %d not implemented", expr->type);
  }

  return result;
}
