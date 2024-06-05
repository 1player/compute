#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "lang.h"

#define TRY(e) do {     \
    if (!(e)) {         \
      return NULL;      \
    }                   \
} while(0)

expr_t *expression(parser_t *parser);
static expr_t *expression_(parser_t *parser, int min_precedence);

// Return precedence when left-associative, -precedence otherwise
static int operator_precedence(enum token_type tok) {
  switch ((int)tok) {
  case TOKEN_DEFINE:
  case '=':
    return -10;

  case TOKEN_EQUALS:
    return 20;

  case '+':
  case '-':
    return 30;

  case '*':
  case '/':
  case '%':
    return 40;

  case '(':
  case '.':
    return 100;
  }

  return 0;
}

static enum token_type peek(parser_t *parser) {
  return parser->cur_token.type;
}

static void advance(parser_t *parser) {
  parser->prev_token = parser->cur_token;
  lexer_next(&parser->lexer, &parser->cur_token);
}

void skip_empty(parser_t *parser) {
  while (peek(parser) == TOKEN_NEWLINE || peek(parser) == ';') {
    advance(parser);
  }
}

static void parser_error(parser_t *parser, char *msg, ...) {
  va_list args;
  va_start(args, msg);
  error(parser->lexer.file, parser->lexer.line, msg, args);
  va_end(args);

  parser->had_errors = true;
}


static int expect_and_advance(parser_t *parser, enum token_type expected) {
  if (peek(parser) == expected) {
    advance(parser);
    return 0;
  }

  parser_error(parser, "Unexpected token %s", lexer_explain(&parser->cur_token));
  return 1;
}

static expr_t *new_expr(enum expr_type type) {
  expr_t *expr = calloc(1, sizeof(expr_t));
  expr->type = type;
  return expr;
}

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

static array_t *arg_list(parser_t *parser) {
  if (expect_and_advance(parser, '(')) {
    return NULL;
  }

  array_t *args = array_new();

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

// Parse an argument list only if there is one, otherwise return an empty array
static array_t *maybe_arg_list(parser_t *parser) {
  if (peek(parser) == '(')
    return arg_list(parser);

  return array_new();
}

void synchronize(parser_t *parser) {
  enum token_type tok;

  do {
    tok = peek(parser);
    advance(parser);
  } while (tok != TOKEN_NEWLINE && tok != TOKEN_EOF);
}

void end_of_expression(parser_t *parser, bool in_block) {
  switch((int)peek(parser)) {
  case '}':
    if (in_block) {
      return;
    } else {
      break;
    }

  case ';':
  case TOKEN_EOF:
  case TOKEN_NEWLINE:
    advance(parser);
    return;

  default:
    break;
  }

  parser_error(parser, "Expected end of expression, instead got %s", lexer_explain(&parser->cur_token));
  synchronize(parser);
}

expr_t *block(parser_t *parser) {
  if (expect_and_advance(parser, '{')) {
    return NULL;
  }

  skip_empty(parser);

  expr_t *block_expr = new_expr(EXPR_BLOCK);
  block_expr->block.exprs = array_new();

  expr_t *expr;
  while (peek(parser) != '}') {
    if ((expr = expression(parser))) {
      end_of_expression(parser, true);
      array_append(block_expr->block.exprs, expr);
    } else {
      return NULL;
    }
  }

  if (expect_and_advance(parser, '}')) {
    return NULL;
  }

  return block_expr;
}

expr_t *conditional(parser_t *parser) {
  if (expect_and_advance(parser, TOKEN_IF)) {
    return NULL;
  }

  expr_t *expr = new_expr(EXPR_CONDITIONAL);
  TRY(expr->conditional.test = expression(parser));
  TRY(expr->conditional.if_block = block(parser));

  if (peek(parser) == TOKEN_ELSE) {
    advance(parser);

    // Expect a block or another if (else if)
    if (peek(parser) == '{') {
      TRY(expr->conditional.else_expr = block(parser));
    } else if (peek(parser) == TOKEN_IF) {
      TRY(expr->conditional.else_expr = conditional(parser));
    } else {
      parser_error(parser, "Expected 'if' or a block to follow the 'else' keyword");
      return NULL;
    }
  }

  return expr;
}

expr_t *closure(parser_t *parser) {
  if (expect_and_advance(parser, TOKEN_FUNC)) {
    return NULL;
  }

  expr_t *expr = new_expr(EXPR_CLOSURE);
  TRY(expr->closure.args = arg_list(parser));
  TRY(expr->closure.block = block(parser));

  return expr;
}

expr_t *loop(parser_t *parser) {
  if (expect_and_advance(parser, TOKEN_LOOP)) {
    return NULL;
  }

  expr_t *expr = new_expr(EXPR_LOOP);
  TRY(expr->loop.condition = expression(parser));
  TRY(expr->loop.block = block(parser));

  return expr;
}


expr_t *atom(parser_t *parser) {
  expr_t *expr = NULL;

  enum token_type tok = peek(parser);
  switch ((int)tok) {
  case '(':
    expr = subexpression(parser);
    break;

  case '{':
    expr = block(parser);
    break;

  case TOKEN_IF:
    expr = conditional(parser);
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

  case TOKEN_FUNC:
    expr = closure(parser);
    break;

  case TOKEN_LOOP:
    expr = loop(parser);
    break;

  case TOKEN_SCOPE:
    expr = new_expr(EXPR_SCOPE);
    advance(parser);
    break;

  case TOKEN_SELF:
    expr = new_expr(EXPR_SELF);
    advance(parser);
    break;

  default:
    char *e = lexer_explain(&parser->cur_token);
    parser_error(parser, "Unexpected %s while parsing expression", e);
    free(e);

    synchronize(parser);
  }

  return expr;
}


static expr_t *expression_(parser_t *parser, int min_precedence) {
  expr_t *left = atom(parser);
  if (!left) {
    return NULL;
  }

  expr_t *result = left;
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

    switch ((int)tok) {
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case TOKEN_IS:
    case '<':
    case '>':
    case TOKEN_EQUALS:
    case TOKEN_LTE:
    case TOKEN_GTE:
      advance(parser);

      if (!(right = expression_(parser, next_min_precedence))) {
        return NULL;
      }

      expr = new_expr(EXPR_BINARY_SEND);
      expr->binary_send.left = result;
      expr->binary_send.right = right;
      expr->binary_send.selector = new_expr(EXPR_IDENTIFIER);

      char **name = &expr->binary_send.selector->identifier.name;
      if (tok < 256) {
        asprintf(name, "%c", (char)tok);
      } else if (tok == TOKEN_EQUALS) {
        asprintf(name, "==");
      } else if (tok == TOKEN_IS) {
        asprintf(name, "===");
      } else if (tok == TOKEN_LTE) {
        asprintf(name, "<=");
      } else if (tok == TOKEN_GTE) {
        asprintf(name, ">=");
      } else {
        panic("Unimplemented");
      }


      result = expr;
      break;

    case TOKEN_DEFINE:
    case '=':
      advance(parser);

      if (!(right = expression_(parser, next_min_precedence))) {
        return NULL;
      }

      expr = new_expr(EXPR_ASSIGNMENT);
      expr->assignment.definition = tok == TOKEN_DEFINE;
      expr->assignment.left = result;
      expr->assignment.right = right;

      result = expr;
      break;

    case '(':
      if (left->type != EXPR_IDENTIFIER) {
        parser_error(parser, "Expected identifier before argument list");
        return NULL;
      }

      expr = new_expr(EXPR_SEND);
      expr->send.receiver = new_expr(EXPR_SCOPE);
      expr->send.selector = left;
      if (!(expr->send.args = arg_list(parser))) {
        return NULL;
      }
      result = expr;

      break;

    case '.':
      advance(parser);

      tok = peek(parser);
      if (tok != TOKEN_ID) {
        parser_error(parser, "Expected identifier, got %s", lexer_explain(&parser->cur_token));
        return NULL;
      }

      expr = new_expr(EXPR_SEND);
      expr->send.receiver = result;
      expr->send.selector = new_expr(EXPR_IDENTIFIER);
      expr->send.selector->identifier.name = parser->cur_token.value_id;
      advance(parser);

      if (!(expr->send.args = maybe_arg_list(parser))) {
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

int parser_init(parser_t *parser, char *file, char *input) {
  if (lexer_create(&parser->lexer, file, input)) {
    return 1;
  }

  parser->cur_token.type = -1;
  parser->prev_token.type = -1;
  parser->had_errors = false;

  advance(parser);

  return 0;
}


expr_t *parser_next(parser_t *parser) {
  skip_empty(parser);

  if (peek(parser) == TOKEN_EOF) {
    return NULL;
  }

  expr_t *expr = expression(parser);
  end_of_expression(parser, false);

  if (parser->had_errors) {
    return NULL;
  }
  return expr;
}
