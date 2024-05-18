#include <stddef.h>
#include <stdarg.h>

#include "common.h"

static void parser_error(parser_t *parser, char *format, ...) {
  char *msg;

  va_list args;
  va_start(args, format);

  vasprintf(&msg, format, args);
  array_append(&parser->errors, msg);

  va_end(args);

}

static enum token_type peek(parser_t *parser) {
  return parser->cur_token.type;
}

static void advance(parser_t *parser) {
  parser->prev_token = parser->cur_token;
  lexer_next(&parser->lexer, &parser->cur_token);
}

static int expect_and_advance(parser_t *parser, enum token_type expected) {
  if (peek(parser) == expected) {
    advance(parser);
    return 0;
  }

  return 1;
}

static expr_t *new_expr(enum expr_type type) {
  expr_t *expr = calloc(1, sizeof(expr_t));
  expr->type = type;
  return expr;
}

expr_t *parse_expression(parser_t *parser) {
  expr_t *left = NULL;

  enum token_type tok = peek(parser);
  switch (tok) {
  case TOKEN_NUMBER:
    left = new_expr(EXPR_LITERAL);
    left->literal.type = LITERAL_NUMBER;
    left->literal.value_number = parser->cur_token.value_number;
    break;

  default:
    parser_error(parser, "Unexpected token %d while parsing expression", tok);
    return NULL;
  }

  advance(parser);

  expr_t *expr = NULL;
  tok = peek(parser);

  switch ((char)tok) {
  case '+':
  case '-':
  case '*':
  case '/':
    advance(parser);

    expr = new_expr(EXPR_BINARY_OP);
    expr->binary_op.op = tok;
    expr->binary_op.left = left;
    expr->binary_op.right = parse_expression(parser);
    break;

  default:
    return left;
  }

  return expr;
}

toplevel_t *parser_parse(parser_t *parser) {
  expr_t *expr;

  toplevel_t *top = calloc(1, sizeof(toplevel_t));
  array_init(&top->exprs);

  while (peek(parser) != TOKEN_EOF) {
    if ((expr = parse_expression(parser))) {
      array_append(&top->exprs, expr);
    } else {
      break;
    }
  }


  return top;
}

int parser_create(parser_t *parser, char *filename, char *input) {
  if (lexer_create(&parser->lexer, input)) {
    return 1;
  }

  parser->cur_token.type = -1;
  parser->prev_token.type = -1;
  array_init(&parser->errors);

  advance(parser);

  return 0;
}
