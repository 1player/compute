#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "lang.h"

typedef struct {
  lexer_t lexer;
  token_t cur_token;
  token_t prev_token;
  bool had_errors;
} parser_t;

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

  parser_error(parser, "Unexpected token %s", token_explain(&parser->cur_token));
  return 1;
}

static expr_t *new_expr(enum expr_type type) {
  expr_t *expr = calloc(1, sizeof(expr_t));
  expr->type = type;
  return expr;
}

expr_t *expression(parser_t *parser);
static expr_t *expression_(parser_t *parser, int min_precedence);

expr_t *subexpression(parser_t *parser) {
  if (expect_and_advance(parser, '(')) {
    return NULL;
  }
  expr_t *expr = expression(parser);
  if (!expr) {
    return NULL;
  }

  if (expect_and_advance(parser, ')')) {
    return NULL;
  }

  return expr;
}

static array_t *send_args(parser_t *parser) {
  if (expect_and_advance(parser, '(')) {
    return NULL;
  }

  array_t *args = calloc(1, sizeof(array_t));
  array_init(args);

  while (peek(parser) != ')') {
    expr_t *arg = expression(parser);
    array_append(args, arg);

    enum token_type tok = peek(parser);
    if (tok == ',') {
      advance(parser);
      continue;
    } else if (tok == ')') {
      break;
    }

    parser_error(parser, "Expected ',' or ')' in argument list");
    return NULL;
  }

  // Skip over )
  advance(parser);

  return args;
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
    parser_error(parser, "Expected end of expression, instead got %s", token_explain(&parser->cur_token));
    synchronize(parser);
  }
}

expr_t *atom(parser_t *parser) {
  expr_t *expr = NULL;

  enum token_type tok = peek(parser);
  switch ((int)tok) {
  case '(':
    expr = subexpression(parser);
    break;

  case TOKEN_NUMBER:
    expr = new_expr(EXPR_LITERAL);
    expr->literal.type = LITERAL_NUMBER;
    expr->literal.value_number = parser->cur_token.value_number;

    advance(parser);
    break;

  case TOKEN_STRING:
    expr = new_expr(EXPR_LITERAL);
    expr->literal.type = LITERAL_STRING;
    expr->literal.value_string = parser->cur_token.value_string;

    advance(parser);
    break;

  case TOKEN_ID:
    expr = new_expr(EXPR_IDENTIFIER);
    expr->identifier.name = parser->cur_token.value_id;

    advance(parser);
    break;

  default:
    char *e = token_explain(&parser->cur_token);
    parser_error(parser, "Unexpected %s while parsing expression", e);
    free(e);

    synchronize(parser);
  }

  return expr;
}

// Return precedence when left-associative, -precedence otherwise
static int operator_precedence(enum token_type tok) {
  switch ((char)tok) {
  case '=':
    return -1;

  case '+':
  case '-':
    return 2;

  case '*':
  case '/':
    return 3;

  case '(':
  case '.':
    return 100;
  }

  return 0;
}

static expr_t *expression_(parser_t *parser, int min_precedence) {
  expr_t *result = atom(parser);
  if (!result) {
    return NULL;
  }

  expr_t *expr, *right;
  enum token_type tok;

  while ((tok = peek(parser)) != TOKEN_EOF) {
    int p = operator_precedence(tok);
    int precedence = abs(p);
    bool left_associative = p > 0;

    if (precedence < min_precedence) {
      break;
    }

    int next_min_precedence = left_associative ? precedence + 1 : precedence;

    switch ((char)tok) {
    case '+':
    case '-':
    case '*':
    case '/':
      advance(parser);

      if (!(right = expression_(parser, next_min_precedence))) {
        return NULL;
      }

      expr = new_expr(EXPR_BINARY_SEND);
      expr->binary_send.left = result;
      expr->binary_send.right = right;
      expr->binary_send.selector = new_expr(EXPR_IDENTIFIER);
      asprintf(&expr->binary_send.selector->identifier.name, "%c", (char)tok);

      result = expr;
      break;

    case '=':
      advance(parser);

      if (!(right = expression_(parser, next_min_precedence))) {
        return NULL;
      }

      expr = new_expr(EXPR_ASSIGNMENT);
      expr->assignment.left = result;
      expr->assignment.right = right;

      result = expr;
      break;

    case '(':
      expr = new_expr(EXPR_SEND);
      expr->send.receiver = new_expr(EXPR_SELF);
      expr->send.selector = result;
      if (!(expr->send.args = send_args(parser))) {
        return NULL;
      }
      result = expr;
      break;

    case '.':
      advance(parser);

      tok = peek(parser);
      if (tok != TOKEN_ID) {
        parser_error(parser, "Expected identifier, got %s", token_explain(&parser->cur_token));
        return NULL;
      }

      expr = new_expr(EXPR_SEND);
      expr->send.receiver = result;
      expr->send.selector = new_expr(EXPR_IDENTIFIER);
      expr->send.selector->identifier.name = parser->cur_token.value_id;

      advance(parser);
      if (!(expr->send.args = send_args(parser))) {
        return NULL;
      }

      result = expr;
      break;

    default:
      goto end;
    }
  }

 end:
  return result;
}

expr_t *expression(parser_t *parser) {
  return expression_(parser, 0);
}

static toplevel_t *toplevel(parser_t *parser) {
  expr_t *expr;

  toplevel_t *top = calloc(1, sizeof(toplevel_t));
  array_init(&top->exprs);

  while (peek(parser) != TOKEN_EOF) {
    if ((expr = expression(parser))) {
      end_of_expression(parser);
      array_append(&top->exprs, expr);
    } else {
      break;
    }
  }

  return top;
}

static int parser_init(parser_t *parser, char *file, char *input) {
  if (lexer_create(&parser->lexer, file, input)) {
    return 1;
  }

  parser->cur_token.type = -1;
  parser->prev_token.type = -1;
  parser->had_errors = false;

  advance(parser);

  return 0;
}

toplevel_t *parse_toplevel(char *file, char *input) {
  parser_t parser;
  if (parser_init(&parser, file, input)) {
    return NULL;
  }

  toplevel_t *top = toplevel(&parser);
  if (parser.had_errors) {
    return NULL;
  }
  return top;
}

expr_t *parse_expression(char *input) {
  parser_t parser;
  if (parser_init(&parser, "expr", input)) {
    return NULL;
  }

  expr_t *expr = expression(&parser);
  if (parser.had_errors) {
    return NULL;
  }
  return expr;
}
