#ifndef LANG_H
#define LANG_H

#include <stdbool.h>
#include <stddef.h>

#include "lib.h"

enum token_type {
  TOKEN_EOF = 0,
  TOKEN_NEWLINE = 256,
  TOKEN_ID,
  TOKEN_STRING,
  TOKEN_NUMBER,
  TOKEN_DEFINE, // :=
  TOKEN_EQUALS, // ==
  TOKEN_IS,     // ===
  TOKEN_IF,     // if
  TOKEN_ELSE,   // else
  TOKEN_FUNC,   // func
};

typedef struct token {
  enum token_type type;

  union {
    char *value_id;
    char *value_string;
    int value_number;
  };
} token_t;

typedef struct lexer {
  char *ptr;
  const char *file;
  unsigned int line;
} lexer_t;

int lexer_create(lexer_t *lexer, char *file, char *input);
int lexer_next(lexer_t *lexer, token_t *token);
char *lexer_explain(token_t *tok);

//

enum literal_type {
  LITERAL_NUMBER,
  LITERAL_STRING,
};

enum expr_type {
  EXPR_LITERAL,
  EXPR_IDENTIFIER,
  EXPR_SELF,
  EXPR_BINARY_SEND,
  EXPR_SEND,
  EXPR_ASSIGNMENT,
  EXPR_BLOCK,
  EXPR_CONDITIONAL,
  EXPR_CLOSURE,
};

typedef struct expr_t {
  enum expr_type type;

  union {
    struct {
      enum literal_type type;
      int value_number;
      char *value_string;
    } literal;

    struct {
      struct expr_t *receiver;
      struct expr_t *selector;
      array_t *args;
    } send;

    struct {
      char *name;
    } identifier;

    struct {
      struct expr_t *left;
      struct expr_t *selector;
      struct expr_t *right;
    } binary_send;

    struct {
      struct expr_t *left;
      struct expr_t *right;
      bool definition;
    } assignment;

    struct {
      array_t *exprs;
    } block;

    struct {
      struct expr_t *test;
      struct expr_t *if_block;
      struct expr_t *else_expr;
    } conditional;

    struct {
      array_t *args;
      struct expr_t *block;
    } closure;
  };
} expr_t;

void expr_dump(expr_t *expr);

//

typedef struct {
  lexer_t lexer;
  token_t cur_token;
  token_t prev_token;
  bool had_errors;
} parser_t;

int parser_init(parser_t *parser, char *file, char *input);
expr_t *parser_next(parser_t *parser);



#endif
