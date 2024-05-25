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
  TOKEN_EQUALS, // ==
  TOKEN_IS,     // ===
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
    } assignment;
  };
} expr_t;

void expr_dump(expr_t *expr);

//

typedef struct toplevel_t {
  array_t exprs;
} toplevel_t;

toplevel_t *parse_toplevel(char *file, char *input);
expr_t *parse_expression(char *input);


#endif
