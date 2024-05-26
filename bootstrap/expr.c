#include <stdarg.h>
#include <stdio.h>

#include "lang.h"

static void dump(int indent, expr_t *expr);

static void emit(char *s, ...) {
  va_list args;

  va_start(args, s);
  vfprintf(stderr, s, args);
  va_end(args);
}

static void ln(int indent) {
  fputc('\n', stderr);

  while (indent--) {
    fputs("  ", stderr);
  }
}

static void dump_array(int indent, array_t *ary) {
  emit("[");

  for (int i = 0; i < ary->size; i++) {
    expr_t *expr = ary->elements[i];
    ln(indent + 1); emit("%d: ", i); dump(indent + 1, expr); emit(", ");
  }

  ln(indent); emit("]");
}

static void dump(int indent, expr_t *expr) {
  switch (expr->type) {
  case EXPR_LITERAL:
    switch (expr->literal.type) {
    case LITERAL_NUMBER:
      emit("Number(%d)", expr->literal.value_number);
      break;
    case LITERAL_STRING:
      emit("String(\"%s\")", expr->literal.value_string);
      break;
    default:
      panic("Not implemented dump of expr->literal.type %d", expr->literal.type);
    }
    break;

  case EXPR_SELF:
    emit("Self");
    break;

  case EXPR_IDENTIFIER:
    emit("Identifier(%s)", expr->identifier.name);
    break;

  case EXPR_BINARY_SEND:
    emit("BinarySend {");
    ln(indent + 1); emit("left: "); dump(indent + 1, expr->binary_send.left);
    ln(indent + 1); emit("selector: "); dump(indent + 1, expr->binary_send.selector);
    ln(indent + 1); emit("right: "); dump(indent + 1, expr->binary_send.right);
    ln(indent); emit("}");
    break;

  case EXPR_SEND:
    emit("Send {");
    ln(indent + 1); emit("receiver: "); dump(indent + 1, expr->send.receiver);
    ln(indent + 1); emit("selector: "); dump(indent + 1, expr->send.selector);
    ln(indent + 1); emit("args: "); dump_array(indent + 1, expr->send.args);
    ln(indent); emit("}");
    break;

  case EXPR_ASSIGNMENT:
    emit("Assignment {");
    ln(indent + 1); emit("left: "); dump(indent + 1, expr->assignment.left);
    ln(indent + 1); emit("right: "); dump(indent + 1, expr->assignment.right);
    ln(indent); emit("}");
    break;

  case EXPR_BLOCK:
    emit("Block {");
    ln(indent + 1); dump_array(indent + 1, expr->block.exprs);
    ln(indent); emit("}");
    break;

  default:
    panic("Not implemented dump of expr->type %d", expr->type);
  }
}

void expr_dump(expr_t *expr) {
  dump(0, expr);
  putchar('\n');
}
