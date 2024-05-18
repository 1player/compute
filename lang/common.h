#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>

typedef struct {
  void **elements;
  int size;
  int capacity;
} array_t;

void array_init(array_t *ary);
void array_append(array_t *ary, void *el);
void array_resize(array_t *ary, int new_capacity);

//

enum token_type {
  TOKEN_EOF = 0,
  TOKEN_ID = 256,
  TOKEN_STRING,
  TOKEN_NUMBER,
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
  unsigned int line;
} lexer_t;

int lexer_create(lexer_t *lexer, char *input);
int lexer_next(lexer_t *lexer, token_t *token);

//

typedef struct {
  lexer_t lexer;
  token_t cur_token;
  token_t prev_token;
  array_t errors;
} parser_t;

typedef struct toplevel_t {
  array_t exprs;
} toplevel_t;

toplevel_t *parser_parse(parser_t *parser);
int parser_create(parser_t *parser, char *filename, char *input);

//

enum literal_type {
  LITERAL_NUMBER
};

enum expr_type {
  EXPR_LITERAL,
  EXPR_IDENTIFIER,
  EXPR_BINARY_OP,
};

typedef struct expr_t {
  enum expr_type type;

  union {
    struct {
      enum literal_type type;
      int value_number;
    } literal;

    struct {
      struct expr_t *callee;
      array_t args;
    } call;

    struct {
      char *name;
    } identifier;

    struct {
      struct expr_t *left;
      enum token_type op;
      struct expr_t *right;
    } binary_op;
  };
} expr_t;

void expr_dump(expr_t *expr);

//

void panic(char *msg, ...);

#endif
