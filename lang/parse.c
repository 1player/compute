#include <stddef.h>
#include <stdarg.h>

#include "lang.h"

static void parser_error(parser_t *parser, char *msg, ...) {
  va_list args;
  va_start(args, msg);
  error(parser->lexer.file, parser->lexer.line, msg, args);
  va_end(args);

  parser->had_errors = true;
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

static char *token_explain(token_t *tok) {
  char *e = NULL;
  
  if ((int)tok->type > 0 && (int)tok->type < 256) {
    asprintf(&e, "character '%c'", (unsigned char)tok->type);
    return e;
  }

  switch (tok->type) {
  case TOKEN_EOF:
    asprintf(&e, "EOF");
    break;

  case TOKEN_NEWLINE:
    asprintf(&e, "newline");
    break;

  case TOKEN_ID:
    asprintf(&e, "identifier '%s'", tok->value_id);
    break;

  case TOKEN_STRING:
    asprintf(&e, "string '%s'", tok->value_string);
    break;

  case TOKEN_NUMBER:
    asprintf(&e, "number '%d'", tok->value_number);
    break;
  }

  return e;
}

expr_t *parse_expression(parser_t *parser);

expr_t *parse_subexpression(parser_t *parser) {
  if (expect_and_advance(parser, '(') != 0) {
    return NULL;
  }
  expr_t *expr = parse_expression(parser);
  if (!expr) {
    return NULL;
  }

  if (expect_and_advance(parser, ')') != 0) {
    return NULL;
  }

  return expr;
}

void synchronize(parser_t *parser) {
  // TODO
  (void)parser;
}

void end_of_expression(parser_t *parser) {
  switch(peek(parser)) {
  case TOKEN_EOF:
  case TOKEN_NEWLINE:
    advance(parser);
    break;

  default:
    parser_error(parser, "Expected end of expression");
    synchronize(parser);
  }
}

expr_t *parse_expression(parser_t *parser) {
  expr_t *left = NULL;

  enum token_type tok = peek(parser);
  switch ((int)tok) {
  case '(':
    left = parse_subexpression(parser);
    break;

  case TOKEN_NUMBER:
    left = new_expr(EXPR_LITERAL);
    left->literal.type = LITERAL_NUMBER;
    left->literal.value_number = parser->cur_token.value_number;

    advance(parser);
    break;

  case TOKEN_ID:
    left = new_expr(EXPR_IDENTIFIER);
    left->identifier.name = parser->cur_token.value_id;

    advance(parser);
    break;

  default:
    char *e = token_explain(&parser->cur_token);
    parser_error(parser, "Unexpected %s while parsing expression", e);
    free(e);

    synchronize(parser);
    return NULL;
  }

  expr_t *expr = NULL, *right;
  tok = peek(parser);

  switch ((char)tok) {
  case '+':
  case '-':
  case '*':
  case '/':
    advance(parser);

    if (!(right = parse_expression(parser))) {
        return NULL;
    }

    expr = new_expr(EXPR_BINARY_OP);
    expr->binary_op.op = tok;
    expr->binary_op.left = left;
    expr->binary_op.right = right;
    break;

  default:
    expr = left;
  }

  return expr;
}

toplevel_t *parser_parse(parser_t *parser) {
  expr_t *expr;

  toplevel_t *top = calloc(1, sizeof(toplevel_t));
  array_init(&top->exprs);

  while (peek(parser) != TOKEN_EOF) {
    if ((expr = parse_expression(parser))) {
      end_of_expression(parser);
      array_append(&top->exprs, expr);
    } else {
      break;
    }
  }


  return top;
}

int parser_create(parser_t *parser, char *file, char *input) {
  if (lexer_create(&parser->lexer, file, input)) {
    return 1;
  }

  parser->cur_token.type = -1;
  parser->prev_token.type = -1;

  advance(parser);

  return 0;
}
