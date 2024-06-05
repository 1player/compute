#include <assert.h>

#include "lang.h"
#include "object.h"
#include "builtins.h"

static bool true_condition(object *test) {
  return test != NULL && test != singleton_false;
}

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
  return send(scope, intern(expr->identifier.name));
}

static object *eval_assignment(expr_t *expr, object *scope) {
  if (expr->assignment.left->type != EXPR_IDENTIFIER) {
    panic("Expected left-hand side of assignment to be a variable name.");
  }

  char *var_name = expr->assignment.left->identifier.name;
  object *var = intern(var_name);

  bool is_definition = expr->assignment.definition;
  object *obj = eval(expr->assignment.right, scope);

  bool success = scope_assign(scope, var, obj, is_definition);
  if (!success) {
    panic("Trying to assign to undefined variable '%s'", var_name);
  }

  return obj;
}

static object *eval_block_with_scope(expr_t *expr, object *inner_scope) {
  object *obj = NULL;
  array_foreach(expr->block.exprs, expr_t *, inner_expr) {
    obj = eval(inner_expr, inner_scope);
  }

  return obj;
}

static object *eval_block(expr_t *expr, object *scope) {
  object *inner_scope = scope_derive(scope);
  return eval_block_with_scope(expr, inner_scope);
}

static object *eval_conditional(expr_t *expr, object *scope_) {
  object *test = eval(expr->conditional.test, scope_);

  // Only nil and `false` are false
  if (true_condition(test)) {
    return eval(expr->conditional.if_block, scope_);
  }

  if (expr->conditional.else_expr) {
    return eval(expr->conditional.else_expr, scope_);
  }

  return NULL;
}

static object *eval_loop(expr_t *expr, object *scope) {
  object *o = NULL;
  object *inner_scope = scope_derive(scope);

  while (true_condition(eval(expr->loop.condition, scope))) {
    o = eval_block_with_scope(expr->loop.block, inner_scope);
  }

  return o;
}

static object *eval_closure(expr_t *expr, object *scope) {
  // Intern the arguments
  array_t *arg_names = array_new_with_capacity(expr->closure.args->size);

  for (int i = 0; i < expr->closure.args->size; i++) {
    expr_t *arg_expr = expr->closure.args->elements[i];

    if (arg_expr->type != EXPR_IDENTIFIER) {
      panic("Expected only identifiers in closure argument list");
    }

    object *name = intern(arg_expr->identifier.name);
    array_append(arg_names, name);
  }

  return closure_new_interpreted(arg_names, expr->closure.block, scope);
}

object *eval_interpreted_closure(interpreted_closure_t *c, void *tdata, object *receiver, ...) {
  va_list va;
  va_start(va, receiver);

  // Create a new child scope and set all the names of arguments to the passed values
  object *inner_scope = scope_derive(c->scope);

  array_foreach(c->arg_names, object *, arg_name) {
    object *arg_value = va_arg(va, object *);
    scope_assign(inner_scope, arg_name, arg_value, true);
  }

  va_end(va);

  return eval_block(c->body, inner_scope);
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

  case EXPR_CLOSURE:
    result = eval_closure(expr, scope);
    break;

  case EXPR_LOOP:
    result = eval_loop(expr, scope);
    break;

  case EXPR_SCOPE:
    result = scope;
    break;

  default:
    info("Eval of %d not implemented", expr->type);
  }

  return result;
}
