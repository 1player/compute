#include <stdarg.h>
#include <stdio.h>

#include "common.h"

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

static void dump(int indent, expr_t *expr) {
  switch (expr->type) {
  case EXPR_LITERAL:
    switch (expr->literal.type) {
    case LITERAL_NUMBER:
      emit("Number(%d)", expr->literal.value_number);
      break;
    default:
      panic("Not implemented expr->literal.type %d", expr->literal.type);
    }
    break;

    
  case EXPR_BINARY_OP:
    emit("BinaryOp {");
    ln(indent + 1); emit("left: "); dump(indent + 1, expr->binary_op.left);
    ln(indent + 1); emit("op: %c", expr->binary_op.op);
    ln(indent + 1); emit("right: "); dump(indent + 1, expr->binary_op.right);
    ln(indent); emit("}");
    break;

  default:
    panic("Not implemented expr->type %d", expr->type);
  }
}

void expr_dump(expr_t *expr) {
  dump(0, expr);
}
